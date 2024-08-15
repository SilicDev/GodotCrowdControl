#include "cc_network_manager.hpp"

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "classes/cc_effect_instance.hpp"
#include "classes/crowd_control.hpp"
#include "json/effect_response.hpp"
#include "json/coin_exchange.hpp"
#include "json/effect_report.hpp"
#include "json/user_info.hpp"

bool CCNetworkManager::is_connected_to_crowd_control() const {
	if (socket.is_null()) {
		return false;
	}
	return socket->get_ready_state() == WebSocketPeer::STATE_OPEN;
}

Error CCNetworkManager::connect(TargetServer target) {
	ERR_FAIL_COND_V_EDMSG(socket->get_ready_state() != socket->WebSocketPeer::STATE_CLOSED, ERR_ALREADY_IN_USE, "Already connected to a server!");
	target_server = target;
	if (connect_thread.is_null()) {
		connect_thread.instantiate();
	}
	if (connect_thread->is_started()) {
		connect_thread->wait_to_finish();
	}
	connect_thread->start(Callable(this, "__socket_connect"));
	CrowdControl::singleton->call_deferred("emit_signal", "connecting");
	time_to_next_ping = (int)Time::get_singleton()->get_unix_time_from_system() + 300;
	time_to_timeout = time_to_next_ping + 300;
	return OK;
}

void CCNetworkManager::close() {
	// might use this in the future to close all connections
}

void CCNetworkManager::login(Platform p_platform) {
	ERR_FAIL_COND_EDMSG(connection_id.is_empty(), "Socket connection hasn't been established yet!");
	String platform_str = CrowdControl::platform_names[p_platform];
	String auth_url = get_url_for_target(target_server, "auth.crowdcontrol.live");
	String url = vformat("%s?platform=%s&connectionID=%s", auth_url, platform_str.to_lower(), connection_id);
	OS::get_singleton()->shell_open(url);
}

void CCNetworkManager::subscribe() {
	Array subscriptions = Array::make("session/self", "prv/self", "pub/self");
	Dictionary request_data;
	request_data["token"] = token;
	request_data["topics"] = subscriptions;
	socket_send_action("subscribe", JSON::stringify(request_data));
}

Error CCNetworkManager::socket_send(const String &msg) const {
	if (is_connected_to_crowd_control()) {
		String msg_out = msg + String("\r\n");
		Error err = socket->send(msg_out.to_utf8_buffer(), WebSocketPeer::WRITE_MODE_TEXT);
		if (err != OK) {
			CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", err);
		}
		return err;
	}
	return ERR_UNAVAILABLE;
}

Error CCNetworkManager::socket_send_action(const String &action, const String &data) const {
	Dictionary dict;
	dict["action"] = action;
	dict["data"] = data;
	return socket_send(JSON::stringify(dict));
}

void CCNetworkManager::client_post(const String &path, const Array &headers, const String &body) {
	Array temp = headers.duplicate();
	temp.append("Content-Type: application/json");
	client_request(HTTPClient::METHOD_POST, path, temp, body);
}

void CCNetworkManager::client_post_authorized(const String &path, const Array &headers, const String &body) {
	Array temp = headers.duplicate();
	temp.append(vformat("Authorization: cc-auth-token %s", token));
	client_post(path, temp, body);
}

void CCNetworkManager::client_request(HTTPClient::Method method, const String &path, const Array &headers, const String &body) {
	// We shouldn't send requests unless we are connected
	if (is_connected_to_crowd_control()) {
		// Restart http client
		if (!client_alive) {
			client_alive = true;
			CrowdControl::singleton->log_info("Client is dead, restarting...");
			if (client_poll_thread.is_null()) {
				client_poll_thread.instantiate();
			}
			if (!client_poll_thread->is_alive()) {
				if (client_poll_thread->is_started()) {
					client_poll_thread->wait_to_finish(); // might be an issue if we somehow enter this before the thread is alive again but after it has been restarted.
				}
				client_poll_thread->start(Callable(this, "__client_loop"));
			}
		}
		Array temp = headers.duplicate();
		// Godot user-agent and default accept header
		temp.append_array(Array::make("User-Agent: Pirulo/1.0 (Godot)", "Accept: application/json"));
		mutex->lock();
		requests.append(Array::make(method, path, temp, body));
		mutex->unlock();
	}
}

void CCNetworkManager::client_request_authorized(HTTPClient::Method method, const String &path, const Array &headers, const String &body) {
	Array temp = headers.duplicate();
	temp.append(vformat("Authorization: cc-auth-token %s", token));
	client_request(method, path, temp, body);
}

void CCNetworkManager::handle_socket_message(const Dictionary &dict) {
	String domain = "";
	String type = "";
	int timestamp = -1;
	Dictionary payload;
	if (dict.has("domain"))
		domain = dict["domain"];
	if (dict.has("type"))
		type = dict["type"];
	if (dict.has("timestamp"))
		timestamp = dict["timestamp"];
	if (dict.has("payload"))
		payload = dict["payload"];

	if (domain == "direct") {
		if (type == "whoami") {
			CrowdControl::singleton->log_info(dict);
			connection_id = payload["connectionID"];
			if (token.is_empty()) {
				CrowdControl::singleton->call_deferred("emit_signal", "logged_out");
			} else {
				subscribe();
				CrowdControl::singleton->call_deferred("emit_signal", "logged_in");
			}
		} else if (type == "login-success") {
			CrowdControl::singleton->log_info(dict);
			token = payload["token"];

			subscribe();
			CrowdControl::singleton->load_effects();
			CrowdControl::singleton->call_deferred("emit_signal", "logged_in");
		} else if (type == "subscription-result") {
			CrowdControl::singleton->log_info(dict);
			Array success = payload["success"];
			Array failure = payload["failure"];

			if (success.size() == 3) {
				session_id = success[0];
				prv_id = success[1];
				pub_id = success[2];
			} else {
				CrowdControl::singleton->log_error("Failed to retrieve CrowdControl PubSub IDs!");
			}

			if (failure.size() != 0) {
				CrowdControl::singleton->log_error(failure);
			}
			client_request_authorized(HTTPClient::METHOD_GET, "/user/profile", Array());
			if (CrowdControl::singleton->auto_starting_session) {
				CrowdControl::singleton->start_session();
			}
		}
	} else if (domain == "pub") {
		if (type == "game-session-stop") {
			CrowdControl::singleton->log_info(dict);
			// String==(Variant) seems broken so we enforce String type
			if (game_session_id == String(payload["gameSessionID"])) {
				// server ended session?
				CrowdControl::singleton->call_deferred("emit_signal", "session_stopped");
				CrowdControl::singleton->display_message("Stopped the Crowd Control session!");
				if (CrowdControl::singleton->quit_on_session_close) {
					CrowdControl::singleton->quitting = true;
				}
				CrowdControl::singleton->session_active = false;
			} else {
				CrowdControl::singleton->log_warn(vformat("Invalid game session stop received! Was the last session not closed properly?"));
			}

		} else if (type == "game-session-start") {
			CrowdControl::singleton->log_info(dict);
			game_session_id = payload["gameSessionID"];
			CrowdControl::singleton->_on_session_started();

		} else if (type == "effect-request") {
			CrowdControl::singleton->log_info(dict);
			String effectID = ((Dictionary)payload.get("effect", {})).get("effectID", "");
			if (CrowdControl::singleton->effects.has(effectID)) {
				EffectRequest request = JSONSerializer::deserialize<EffectRequest>(payload);
				if (!request.parameters.has("quantity") && request.quantity != 0) {
					request.parameters.insert("quantity", { "", "", "quantity", UtilityFunctions::str(request.quantity) });
				}
				Ref<CCEffect> effect = CrowdControl::singleton->effects[effectID];
				CrowdControl::singleton->_on_effect_requested(effect);
				CrowdControl::singleton->queue_effect(effect, request.requester, request.request_id, request.is_test || request.is_anonymous, request.parameters);
			} else {
				// unknown effect? Return fail-permanent
				Ref<CCEffectInstance> temp;
				temp.instantiate();
				temp->set_id(payload["requestID"]);
				socket_send_action("rpc", build_effect_response(temp, FAIL_PERMANENTLY));
				CrowdControl::singleton->log_error(vformat("Unknown effect ID '%s' received!", effectID));
			}
		} else if (type == "coins-exchange") {
			CrowdControl::singleton->log_info(dict);
			CoinExchange exchange = JSONSerializer::deserialize<CoinExchange>(payload);
			Ref<StreamUser> exchanger = CrowdControl::singleton->get_stream_user(exchange.exchanger);
			CrowdControl::singleton->call_deferred("emit_signal", "coins_exchanged", exchange.amount, exchanger);
		}
	}

	if (timestamp != -1) {
		last_message = timestamp;
	}
}

void CCNetworkManager::handle_client_message(const int code, const PackedStringArray &headers, const String &message) {
	if (!message.is_empty()) {
		Error err = json_parser->parse(message, true);
		ERR_FAIL_COND_EDMSG(unlikely(err != OK), vformat("Error while parsing message (%s): %d: %s", UtilityFunctions::error_string(err), json_parser->get_error_line(), json_parser->get_error_message()));
		Dictionary data = json_parser->get_data();
		if (data.has("profile")) {
			UserInfo user = JSONSerializer::deserialize<UserInfo>(data);
			CrowdControl::singleton->streamer_user = StreamUser::make(
					user.profile.type,
					user.profile.origin_data.user.login,
					user.profile.origin_data.user.display_name,
					user.profile.image_url,
					user.profile.roles,
					user.profile.subscriptions,
					user.profile.origin_id);
		} else {
			// we don't print profile data to avoid leaking sensitive data
			CrowdControl::singleton->log_info(vformat("%d: %s", code, message));
		}
	}
}

Ref<Texture2D> CCNetworkManager::download_image(const String &url) {
	if (!url.is_empty()) {
		String host = url.substr(0, url.find("/", url.find(":") + 3));
		String path = url.substr(url.find("/", url.find(":") + 3));
		bool done = false;
		bool request_done = false;
		PackedByteArray rb;
		PackedByteArray chunk;
		Ref<HTTPClient> http;
		http.instantiate();
		http->connect_to_host(host);
		http->poll();
		while (!done) {
			switch (http->get_status()) {
				case HTTPClient::STATUS_DISCONNECTED:
					done = true;
					break;
				case HTTPClient::STATUS_RESOLVING:
					http->poll();
					break;
				case HTTPClient::STATUS_CANT_RESOLVE:
					done = true;
					break;
				case HTTPClient::STATUS_CONNECTING:
					http->poll();
					break;
				case HTTPClient::STATUS_CANT_CONNECT:
					done = true;
					break;
				case HTTPClient::STATUS_CONNECTED:
					if (!request_done) {
						http->request(HTTPClient::METHOD_GET, path, PackedStringArray());
						request_done = true;
					}
					break;
				case HTTPClient::STATUS_REQUESTING:
					http->poll();
					break;
				case HTTPClient::STATUS_BODY:
					if (http->has_response()) {
						chunk = http->read_response_body_chunk();
						if (chunk.size() == 0) {
							// No data, but received body, let's keep waiting for the body
							OS::get_singleton()->delay_msec(20);
						} else {
							rb.append_array(chunk);
						}
					}
					http->poll();
					if (http->get_status() != HTTPClient::STATUS_BODY) {
						done = true;
					}
					break;
				case HTTPClient::STATUS_CONNECTION_ERROR:
					done = true;
					break;
				case HTTPClient::STATUS_TLS_HANDSHAKE_ERROR:
					done = true;
					break;

				default:
					break;
			}
			OS::get_singleton()->delay_msec(20);
		}
		if (rb.size() != 0) {
			Ref<Image> image;
			image.instantiate();
			image->load_png_from_buffer(rb);
			Ref<ImageTexture> tex;
			tex.instantiate();
			tex->set_image(image);
			return tex;
		}
	}
	return nullptr;
}

void CCNetworkManager::attempt_reconnect() {
	current_reconnect_retry_count++;
	bool should_retry = (CrowdControl::singleton->reconnect_retry_count == -1 || current_reconnect_retry_count < CrowdControl::singleton->reconnect_retry_count);
	if (CrowdControl::singleton->reconnect_retry_count != 0 && should_retry) {
		time_to_next_ping = (int)Time::get_singleton()->get_unix_time_from_system() + CrowdControl::singleton->reconnect_retry_delay;
	} else {
		close();
		CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_CONNECT);
	}
}

String CCNetworkManager::get_url_for_target(TargetServer target, const String &base_url) const {
	switch (target) {
		case TargetServer::PRODUCTION:
			return "https://" + base_url;
		case TargetServer::STAGING:
			return "https://beta-" + base_url;
		case TargetServer::DEV:
			return "https://dev-" + base_url;

		default:
			ERR_FAIL_V_EDMSG("", "Invalid enum value supplied for target server");
	}
}

void CCNetworkManager::socket_connect() {
	OS::get_singleton()->set_thread_name("Crowd Control Connect Thread");

	while (unlikely(socket->get_ready_state() == WebSocketPeer::STATE_CLOSING)) {
		OS::get_singleton()->delay_msec(100);
	}

	Error err = socket->connect_to_url("wss://pubsub.crowdcontrol.live");
	if (unlikely(err != OK)) {
		// Malformed URL?
		CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_RESOLVE);
		ERR_FAIL_EDMSG("Failed to connect to Crowd Control!");
	}

	socket->poll();
	while (socket->get_ready_state() == WebSocketPeer::STATE_CONNECTING) {
		OS::get_singleton()->delay_msec(20);
		socket->poll();
	}

	if (socket->get_ready_state() != WebSocketPeer::STATE_OPEN) {
		// Failed to connect
		CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_CONNECT);
		ERR_FAIL_EDMSG("Failed to connect to Crowd Control!");
	}
	CrowdControl::singleton->_on_connected();
	if (socket_poll_thread.is_null()) {
		socket_poll_thread.instantiate();
	}
	socket_poll_thread->start(Callable(this, "__socket_poll_loop"));
}

void CCNetworkManager::socket_poll_loop() {
	OS::get_singleton()->set_thread_name("Crowd Control Poll Loop");
	PackedByteArray msg;
	while (!CrowdControl::singleton->quitting) {
		OS::get_singleton()->delay_msec(20);
		socket->poll();

		if (socket->get_ready_state() != WebSocketPeer::STATE_OPEN) {
			if (socket->get_ready_state() == WebSocketPeer::STATE_CLOSED) {
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CONNECTION_ERROR);
				CrowdControl::singleton->_on_disconnected();
				attempt_reconnect();
				call_deferred("__cleanup_thread", socket_poll_thread);
				ERR_FAIL_EDMSG("Crowd Control socket closed unexpectedly!");
			}
			continue;
		}

		int packets = socket->get_available_packet_count();
		while (packets != 0) {
			msg = socket->get_packet();
			Error err = socket->get_packet_error();
			if (unlikely(err != OK)) {
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", err);
				packets--;
				continue;
			}
			String msg_str = msg.get_string_from_utf8();
			Variant data = UtilityFunctions::str_to_var(msg_str);
			if (data.get_type() == Variant::DICTIONARY) {
				handle_socket_message((Dictionary)data);
			} else {
				CrowdControl::singleton->log_info(vformat("Received: %s", msg_str));
			}
			packets--;
		}
		if (time_to_next_ping < Time::get_singleton()->get_unix_time_from_system()) {
			socket_send_action("ping");
			time_to_next_ping = (int)Time::get_singleton()->get_unix_time_from_system() + 300;
			time_to_timeout = time_to_next_ping + 300;
		}
		if (!CrowdControl::singleton->quitting) {
			OS::get_singleton()->delay_msec(1000);
		}
	}
	socket->close();
	while (socket->get_ready_state() == WebSocketPeer::STATE_CLOSING) {
		OS::get_singleton()->delay_msec(1000);
		socket->poll();
	}
	CrowdControl::singleton->_on_disconnected();
	call_deferred("__cleanup_thread", socket_poll_thread);
}

void CCNetworkManager::client_loop() {
	CrowdControl::singleton->log_info("Starting client...");
	if (unlikely(client->get_status() == HTTPClient::STATUS_CONNECTED)) {
		client->close();
	}
	OS::get_singleton()->set_thread_name("Crowd Control HTTP Poll Loop");
	String host = get_url_for_target(target_server, "openapi.crowdcontrol.live");
	Error err = client->connect_to_host(host);
	if (err != OK) {
		// Malformed URL?
		CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_RESOLVE);
		ERR_FAIL_EDMSG("Failed to connect to Crowd Control! Was the gdextension edited?");
	}
	bool has_connected = false;
	int ticks = 0;
	client->poll();
	while (!CrowdControl::singleton->quitting && client_alive) {
		switch (client->get_status()) {
			case HTTPClient::STATUS_DISCONNECTED:
				client_alive = false;
				break;
			case HTTPClient::STATUS_RESOLVING:
				client->poll();
				break;
			case HTTPClient::STATUS_CANT_RESOLVE:
				client_alive = false;
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_RESOLVE);
				ERR_FAIL_EDMSG("Failed to resolve address to Crowd Control! Was the gdextension modified?");
				break;
			case HTTPClient::STATUS_CONNECTING:
				client->poll();
				break;
			case HTTPClient::STATUS_CANT_CONNECT:
				client_alive = false;
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CANT_CONNECT);
				ERR_FAIL_EDMSG("Failed to connect to Crowd Control!");
				break;
			case HTTPClient::STATUS_CONNECTED:
				if (!has_connected) {
					has_connected = true;
				}
				if (requests.size() != 0) {
					mutex->lock();
					Array request = requests.pop_front();
					mutex->unlock();
					ticks = Time::get_singleton()->get_ticks_msec();
					//CrowdControl::singleton->log_info(request);
					err = client->request((HTTPClient::Method)((int)request[0]), request[1], request[2], request[3]);
					if (err != OK) {
						CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", err);
						break;
					}
				}
				client->poll();
				break;
			case HTTPClient::STATUS_REQUESTING:
				client->poll();
				break;
			case HTTPClient::STATUS_BODY:
				if (client->has_response()) {
					PackedByteArray rb;
					const int response_code = client->get_response_code();
					const PackedStringArray headers = client->get_response_headers();
					PackedByteArray chunk = client->read_response_body_chunk();
					if (chunk.size() == 0) {
						// No data, but received body, let's keep waiting for the body
						OS::get_singleton()->delay_msec(20);
					} else {
						rb.append_array(chunk);
					}
					if (rb.size() != 0) {
						String message = rb.get_string_from_utf8();
						handle_client_message(response_code, headers, message);
					}
				}
				client->poll();
				if (client->get_status() != HTTPClient::STATUS_BODY) {
					// response end
					CrowdControl::singleton->log_info(vformat("Request completed in %s msec(s)", Time::get_singleton()->get_ticks_msec() - ticks));
				}
				break;
			case HTTPClient::STATUS_CONNECTION_ERROR:
				client_alive = false;
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CONNECTION_ERROR);
				client->close();
				break;
			case HTTPClient::STATUS_TLS_HANDSHAKE_ERROR:
				client_alive = false;
				CrowdControl::singleton->call_deferred("emit_signal", "connection_errored", ERR_CONNECTION_ERROR);
				ERR_FAIL_EDMSG("Connection handshake with Crowd Control failed!");
				break;

			default:
				break;
		}
		OS::get_singleton()->delay_msec(20);
	}
	client->close();
	call_deferred("__cleanup_thread", client_poll_thread);
}

String CCNetworkManager::build_effect_response(const Ref<CCEffectInstance> &instance, EffectInstanceStatus status) {
	return JSONSerializer::serialize_string<EffectResponse>(EffectResponse(token, instance, CrowdControl::effect_instance_status_names[status]));
}

String CCNetworkManager::build_effect_report(const Ref<CCEffect> &p_effect, const String &command) {
	return JSONSerializer::serialize_string<EffectReport>(EffectReport(token, p_effect, command));
}

void CCNetworkManager::download_image_for_user(const Ref<Thread> &thread, const Ref<StreamUser> &user) {
	String url = user->get_profile_icon_url();
	Ref<Texture2D> tex = download_image(url);
	user->set_profile_icon(tex);
	user->call_deferred("emit_signal", "icon_downloaded");
	call_deferred("__cleanup_thread", thread);
}

void CCNetworkManager::cleanup_thread(const Ref<Thread> &thread) {
	if (thread.is_valid() && thread->is_started()) {
		thread->wait_to_finish();
	}
}

void CCNetworkManager::_bind_methods() {
	/// Due to the limitations of the exposed @see Thread class only being able to
	/// be started with @see Callable 's these functions need to be exposed to
	/// the engine. Thankfully members prefixed with `__` are considered internal
	/// and are thus not shown to the user.
	/// However this also means this class needed to be exposed to the engine as well,
	/// though marking it as abstract disables the user from ever instantiating it.
	ClassDB::bind_method(D_METHOD("__socket_connect"), &CCNetworkManager::socket_connect);
	ClassDB::bind_method(D_METHOD("__socket_poll_loop"), &CCNetworkManager::socket_poll_loop);
	ClassDB::bind_method(D_METHOD("__client_loop"), &CCNetworkManager::client_loop);
	ClassDB::bind_method(D_METHOD("__download_profile_image"), &CCNetworkManager::download_image_for_user);
	ClassDB::bind_method(D_METHOD("__cleanup_thread"), &CCNetworkManager::cleanup_thread);
}

String CCNetworkManager::generate_random_string(uint32_t length) {
	const String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	String out = "";
	for (int i = 0; i < length; i++) {
		out += chars[CrowdControl::singleton->rng->randi_range(0, chars.length())];
	}
	return out;
}

CCNetworkManager::CCNetworkManager() {
	socket.instantiate();
	client.instantiate();
	json_parser.instantiate();
	mutex.instantiate();
}

CCNetworkManager::~CCNetworkManager() {
	CrowdControl::singleton->quitting = true;
	if (connect_thread.is_valid() && connect_thread->is_started()) {
		connect_thread->wait_to_finish();
	}
	if (socket_poll_thread.is_valid() && socket_poll_thread->is_started()) {
		socket_poll_thread->wait_to_finish();
	}
	if (client_poll_thread.is_valid() && client_poll_thread->is_started()) {
		client_poll_thread->wait_to_finish();
	}
}