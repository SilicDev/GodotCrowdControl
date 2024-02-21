#ifndef CROWD_CONTROL_CC_NETWORK_MANAGER_HPP
#define CROWD_CONTROL_CC_NETWORK_MANAGER_HPP

#include <godot_cpp/classes/http_client.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/web_socket_peer.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util/binding_macros.hpp"
#include "util/cc_enums.hpp"

class CCEffect;
class CCEffectInstance;
class CrowdControl;
class StreamUser;

using namespace godot;

/// @brief Manages the communication between the engine and Crowd Control, among other internet utilities.
/// @details
/// This is mainly a helper class to seperate the network implementation from the main class.
class CCNetworkManager : public RefCounted {
	GDCLASS(CCNetworkManager, RefCounted)

	//public:
	//
private:
	Ref<WebSocketPeer> socket;
	Ref<HTTPClient> client;

	bool client_alive = false;

	/// @brief Mutex to ensure we don't perform unsafe methods on the requests array (resizing is not thread-safe)
	Ref<Mutex> mutex;

	Ref<Thread> connect_thread;
	Ref<Thread> socket_poll_thread;
	Ref<Thread> client_poll_thread;

	TargetServer target_server;

	String token;
	Array requests;

	int64_t last_message = -1;
	String game_session_id = "";

	String connection_id;

	String session_id;
	String prv_id;
	String pub_id;

	Ref<JSON> json_parser;

	uint64_t time_to_next_ping;
	uint64_t time_to_timeout;
	int current_reconnect_retry_count = 0;

private:
	String get_url_for_target(TargetServer target, const String &base_url) const;

	void socket_connect();
	void socket_poll_loop();
	void client_loop();

	void handle_socket_message(const Dictionary &dict);
	void handle_client_message(const int code, const PackedStringArray &headers, const String &message);

	/// @brief Downloads an image from the specified url
	/// @param url the url to download the image from
	/// @return The requested image if the request is successful, else a nullptr reference
	Ref<Texture2D> download_image(const String &url);

	void attempt_reconnect();

protected:
	static void _bind_methods();

public:
	uint64_t get_time_to_next_ping() const { return time_to_next_ping; }
	void set_time_to_next_ping(uint64_t p_time_to_next_ping) { time_to_next_ping = p_time_to_next_ping; }

	uint64_t get_time_to_timeout() const { return time_to_timeout; }
	void set_time_to_timeout(uint64_t p_time_to_timeout) { time_to_timeout = p_time_to_timeout; }

	static String generate_random_string(uint32_t length);

	const String get_game_session_id() const { return game_session_id; }

	bool is_connected_to_crowd_control() const;

	Error connect(TargetServer target);
	void close();

	void login(Platform p_platform);
	void subscribe();

	Error socket_send(const String &msg) const;
	/// @brief Sends an action as JSON data to the socket server
	/// @param action the action to be performed
	/// @param data data to be send as parameter to the action
	/// @return returns a non OK value when the request failed to be send
	Error socket_send_action(const String &action, const String &data = String()) const;
	void client_post(const String &path, const Array &headers, const String &body = "");
	void client_post_authorized(const String &path, const Array &headers, const String &body = "");
	void client_request(HTTPClient::Method method, const String &path, const Array &headers, const String &body = "");
	void client_request_authorized(HTTPClient::Method method, const String &path, const Array &headers, const String &body = "");

	String build_effect_response(const Ref<CCEffectInstance> &instance, EffectInstanceStatus status);
	String build_effect_report(const Ref<CCEffect> &p_effect, const String &command);

	void download_image_for_user(const Ref<Thread> &thread, const Ref<StreamUser> &user);
	/// @brief waits for a thread to finish to perform cleanup
	/// @param thread the thread to wait on.
	/// @details
	/// Godot requires `wait_to_finish` to be called on every thread at some point.
	/// However as a thread can not wait for itself we use this method via deferred call
	/// to have the main thread perform this.
	/// @note Only call this on threads that are finished, otherwise the main thread will stall.
	void cleanup_thread(const Ref<Thread> &thread);

	CCNetworkManager();
	~CCNetworkManager();
};

#endif // CROWD_CONTROL_CC_NETWORK_MANAGER_HPP