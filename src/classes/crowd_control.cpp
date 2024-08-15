#include "crowd_control.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "classes/cc_network_manager.hpp"
#include "classes/stream_user.hpp"
#include "json/game_session.hpp"
#include "json/request_effect.hpp"
#include "util/binding_macros.hpp"
#include "util/json_serializer.hpp"
#include "util/project_settings.hpp"

CrowdControl *CrowdControl::singleton = nullptr;

const char *CrowdControl::platform_names[] = {
	"Twitch",
	"Youtube",
	"Discord"
};

const char *CrowdControl::effect_instance_status_names[] = {
	"success",
	"failTemporary",
	"failPermanent",
	"timedBegin",
	"timedPause",
	"timedResume",
	"timedEnd",
};

void CrowdControl::_bind_methods() {
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/common/game_id"), "GodotDemo");
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/common/game_name"), "Godot Demo");
	GLOBAL_DEF(PropertyInfo(Variant::INT, "crowd_control/common/retry_count"), -1);
	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "crowd_control/common/retry_delay"), 5.0f);
	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "crowd_control/common/delay_between_effects", PROPERTY_HINT_RANGE, "0,10,0.001,suffix:s"), 0.5f);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/common/effects", PROPERTY_HINT_FILE, "*.tres"), "res://effects.tres");

	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "crowd_control/common/process/process_mode", PROPERTY_HINT_ENUM, "Internal,Process,Physics Process"), 1);
	GLOBAL_DEF_RST(PropertyInfo(Variant::BOOL, "crowd_control/common/process/use_scene_tree_pause"), false);

	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/visuals/temp_user_icon", PROPERTY_HINT_FILE, "*.png,*.svg,*.jpg"), "res://addons/godotcrowdcontrol/assets/icons/user.png");
	GLOBAL_DEF(PropertyInfo(Variant::COLOR, "crowd_control/visuals/temp_user_color"), Color(0.0f, 0.0f, 0.0f, 0.6f));
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/visuals/crowd_user_icon", PROPERTY_HINT_FILE, "*.png,*.svg,*.jpg"), "res://addons/godotcrowdcontrol/assets/icons/user_crowd.png");
	GLOBAL_DEF(PropertyInfo(Variant::COLOR, "crowd_control/visuals/crowd_user_color"), Color(0.094117f, 0.466666f, 0.937254f));
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "crowd_control/visuals/error_user_icon", PROPERTY_HINT_FILE, "*.png,*.svg,*.jpg"), "res://addons/godotcrowdcontrol/assets/icons/user_error.png");
	GLOBAL_DEF(PropertyInfo(Variant::COLOR, "crowd_control/visuals/error_user_color"), Color(0.77647f, 0.0f, 0.0f));

	GLOBAL_DEF(PropertyInfo(Variant::BOOL, "crowd_control/debug/log_info"), true);
	GLOBAL_DEF(PropertyInfo(Variant::BOOL, "crowd_control/debug/log_warn"), true);
	GLOBAL_DEF(PropertyInfo(Variant::BOOL, "crowd_control/debug/log_error"), true);
	GLOBAL_DEF(PropertyInfo(Variant::BOOL, "crowd_control/debug/log_exception"), true);

	ClassDB::bind_method(D_METHOD("connect_to_crowd_control"), &CrowdControl::connect_to_crowd_control);
	ClassDB::bind_method(D_METHOD("login", "platform"), &CrowdControl::login);
	ClassDB::bind_method(D_METHOD("close"), &CrowdControl::close);
	ClassDB::bind_method(D_METHOD("is_session_active"), &CrowdControl::is_session_active);
	ClassDB::bind_method(D_METHOD("is_connected_to_crowd_control"), &CrowdControl::is_connected_to_crowd_control);
	ClassDB::bind_method(D_METHOD("start_session"), &CrowdControl::start_session);
	ClassDB::bind_method(D_METHOD("stop_session"), &CrowdControl::close);
	ClassDB::bind_method(D_METHOD("get_streamer_user"), &CrowdControl::get_streamer_user);
	ClassDB::bind_method(D_METHOD("__thread_process"), &CrowdControl::thread_process);
	ClassDB::bind_method(D_METHOD("__tree_process"), &CrowdControl::tree_process);
	ClassDB::bind_method(D_METHOD("__tree_physics_process"), &CrowdControl::tree_physics_process);
	ClassDB::bind_method(D_METHOD("__process", "delta"), &CrowdControl::_process);
	ClassDB::bind_method(D_METHOD("stop_effect", "effect"), &CrowdControl::stop_effect);
	ClassDB::bind_method(D_METHOD("resume_effect", "effect"), &CrowdControl::resume_effect);
	ClassDB::bind_method(D_METHOD("pause_effect", "effect"), &CrowdControl::pause_effect);
	ClassDB::bind_method(D_METHOD("reset_effect", "effect"), &CrowdControl::reset_effect);
	ClassDB::bind_method(D_METHOD("test_effect", "effect"), &CrowdControl::test_effect);
	ClassDB::bind_method(D_METHOD("test_effect_remotely", "effect"), &CrowdControl::test_effect_remotely);
	ClassDB::bind_method(D_METHOD("get_effect", "id"), &CrowdControl::get_effect);

	ClassDB::bind_method(D_METHOD("set_staging", "enable"), &CrowdControl::set_staging);
	ClassDB::bind_method(D_METHOD("is_staging"), &CrowdControl::is_staging);

	ClassDB::bind_method(D_METHOD("set_paused", "enable"), &CrowdControl::set_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &CrowdControl::is_paused);

	ClassDB::bind_method(D_METHOD("set_auto_starting_session", "enable"), &CrowdControl::set_auto_starting_session);
	ClassDB::bind_method(D_METHOD("is_auto_starting_session"), &CrowdControl::is_auto_starting_session);

	//ClassDB::bind_method(D_METHOD("set_broadcaster_type", "type"), &CrowdControl::set_broadcaster_type);
	//ClassDB::bind_method(D_METHOD("get_broadcaster_type"), &CrowdControl::get_broadcaster_type);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "staging"), "set_staging", "is_staging");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "paused"), "set_paused", "is_paused");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_starting_session"), "set_auto_starting_session", "is_auto_starting_session");
	//ADD_PROPERTY(PropertyInfo(Variant::INT, "broadcaster_type", PROPERTY_HINT_ENUM, "Community,Affiliate,Partner"), "set_broadcaster_type", "get_broadcaster_type");

	//BIND_ENUM_CONSTANT(COMMUNITY);
	//BIND_ENUM_CONSTANT(AFFILIATE);
	//BIND_ENUM_CONSTANT(PARTNER);

	BIND_ENUM_CONSTANT(TWITCH);
	BIND_ENUM_CONSTANT(YOUTUBE);
	BIND_ENUM_CONSTANT(DISCORD);

	ADD_SIGNAL(MethodInfo("connecting"));
	// TODO: Should we add an error source (HTTPClient vs. WebSocket) ?
	ADD_SIGNAL(MethodInfo("connection_errored", PropertyInfo(Variant::INT, "err", PROPERTY_HINT_RESOURCE_TYPE, "Error")));
	ADD_SIGNAL(MethodInfo("connected"));
	ADD_SIGNAL(MethodInfo("disconnected"));

	ADD_SIGNAL(MethodInfo("session_started"));
	ADD_SIGNAL(MethodInfo("session_stopped"));

	ADD_SIGNAL(MethodInfo("logged_in"));
	ADD_SIGNAL(MethodInfo("logged_out"));

	ADD_SIGNAL(MethodInfo("coins_exchanged", PropertyInfo(Variant::INT, "amount"), PropertyInfo(Variant::OBJECT, "exchanger", PROPERTY_HINT_RESOURCE_TYPE, "StreamUser")));
	ADD_SIGNAL(MethodInfo("message_display_requested", PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::FLOAT, "duration"), PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D")));

	ADD_SIGNAL(MethodInfo("effect_queued", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstance")));
	ADD_SIGNAL(MethodInfo("effect_dequeued", PropertyInfo(Variant::STRING, "id"), PropertyInfo(Variant::INT, "result", PROPERTY_HINT_RESOURCE_TYPE, "EffectResult")));

	ADD_SIGNAL(MethodInfo("effect_triggered", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstance")));
	ADD_SIGNAL(MethodInfo("effect_started", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstanceTimed")));
	ADD_SIGNAL(MethodInfo("effect_stopped", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstanceTimed")));
	ADD_SIGNAL(MethodInfo("effect_paused", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstanceTimed")));
	ADD_SIGNAL(MethodInfo("effect_resumed", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstanceTimed")));
	ADD_SIGNAL(MethodInfo("effect_reset", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffectInstanceTimed")));

	ADD_SIGNAL(MethodInfo("effect_requested", PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "CCEffect")));
}

CrowdControl *CrowdControl::get_singleton() {
	if (singleton == nullptr) {
		memnew(CrowdControl);
	}
	return singleton;
}

Error CrowdControl::connect_to_crowd_control() {
	quitting = false;
	process_mode = GLOBAL_GET("crowd_control/common/process/process_mode");
	debug_info = GLOBAL_GET("crowd_control/debug/log_info");
	debug_warn = GLOBAL_GET("crowd_control/debug/log_warn");
	debug_error = GLOBAL_GET("crowd_control/debug/log_error");
	debug_exception = GLOBAL_GET("crowd_control/debug/log_exception");
	if (process_mode == 0) {
		if (process_thread.is_null()) {
			process_thread.instantiate();
		}
		process_thread->start(Callable(this, "__thread_process"));
	} else {
		MainLoop *main = Engine::get_singleton()->get_main_loop();
		scene_tree = Object::cast_to<SceneTree>(main);
		ERR_FAIL_COND_V_EDMSG(scene_tree == nullptr, FAILED, "MainLoop does not extend SceneTree! Cannot connect to process signals!");
		if (process_mode == 1) {
			if (!scene_tree->is_connected("process_frame", Callable(this, "__tree_process"))) {
				scene_tree->connect("process_frame", Callable(this, "__tree_process"));
			}
		} else if (process_mode == 2) {
			if (!scene_tree->is_connected("physics_frame", Callable(this, "__tree_physics_process"))) {
				scene_tree->connect("physics_frame", Callable(this, "__tree_physics_process"));
			}
		}
	}
#ifdef DEV_MODE
	return connection->connect(DEV);
#else
	if (staging) {
		return connection->connect(STAGING);
	}
	return connection->connect(PRODUCTION);
#endif
}

void CrowdControl::close() {
	quit_on_session_close = true;
	stop_session();
	connection->close();
}

void CrowdControl::login(Platform p_platform) {
	connection->login(p_platform);
}

bool CrowdControl::is_connected_to_crowd_control() const {
	if (connection.is_null()) {
		return false;
	}
	return connection->is_connected_to_crowd_control();
}

bool CrowdControl::is_session_active() const {
	return session_active;
}

void CrowdControl::start_session() {
	if (!session_active) {
		quit_on_session_close = false;
		connection->client_post_authorized("/game-session/start", Array(), JSONSerializer::serialize_string(StartSession(GLOBAL_GET("crowd_control/common/game_id"), Array())));
	} else {
		log_warn("A session is already active!");
	}
}

void CrowdControl::stop_session() {
	if (session_active) {
		connection->client_post_authorized("/game-session/stop", Array(), JSONSerializer::serialize_string<StopSession>(StopSession(connection->get_game_session_id())));
	}
}

void CrowdControl::test_effect(const Ref<CCEffect> &effect) {
	HashMap<String, ParameterEntry> parameters = HashMap<String, ParameterEntry>();
	if (effect->get_effect_type() == CCEffect::PARAMETERS) {
		Ref<CCEffectParameters> effect_params = Object::cast_to<CCEffectParameters>(effect.ptr());
		for (KeyValue<String, CCEffectParameters::ParameterEntry> kvp : effect_params->get_parameter_entries_by_id()) {
			ParameterEntry entry = ParameterEntry();
			entry.name = kvp.value.name;
			entry.value = kvp.value.name;
			parameters.insert(kvp.value.name, entry);
		}
	}
	queue_effect(effect, User(), UtilityFunctions::str((int)(Time::get_singleton()->get_unix_time_from_system() * 1000)), true, parameters);
}

void CrowdControl::test_effect_remotely(const Ref<CCEffect> &effect) {
	connection->client_post_authorized("/game-session/effect-request", Array(), JSONSerializer::serialize_string<RequestEffect>(RequestEffect(connection->get_game_session_id(), effect->get_id())));
}

void CrowdControl::display_message(String message, float duration) {
	call_deferred("emit_signal", "message_display_requested", message, duration, streamer_user.is_valid() ? streamer_user->get_profile_icon() : nullptr);
}

void CrowdControl::register_effect(const Ref<CCEffect> &effect) {
	effects.insert(effect->get_id(), effect);
	log_info(vformat("Registered Effect %s!", effect->get_id()));
}

Ref<CCEffect> CrowdControl::get_effect(const StringName &name) const {
	if (effects.has(name)) {
		return effects[name];
	}
	return Ref<CCEffect>();
}

void CrowdControl::thread_process() {
	double last_run = Time::get_singleton()->get_unix_time_from_system();
	while (!quitting) {
		float delta_usec = 10000000.0 / Engine::get_singleton()->get_frames_per_second();
		OS::get_singleton()->delay_usec(delta_usec);
		float delta = Time::get_singleton()->get_unix_time_from_system() - last_run;
		_process(delta);
		last_run = Time::get_singleton()->get_unix_time_from_system();
	}
	if (process_thread.is_valid() && process_thread->is_started()) {
		connection->call_deferred("__cleanup_thread", process_thread);
	}
	_on_disconnected();
}

void CrowdControl::tree_process() {
	ERR_FAIL_NULL_EDMSG(scene_tree, "MainLoop does not extend SceneTree!");
	if (GLOBAL_GET("crowd_control/common/process/use_scene_tree_pause")) {
		bool tree_pause = scene_tree->is_paused();
		if (tree_pause && !paused) {
			set_paused(tree_pause);
		} else if (!tree_pause && paused) {
			set_paused(tree_pause);
		}
	}
	if (!(quitting || paused)) {
		double delta = scene_tree->get_root()->get_process_delta_time();
		_process(delta);
	}
}

void CrowdControl::tree_physics_process() {
	if (GLOBAL_GET("crowd_control/common/process/use_scene_tree_pause")) {
		bool tree_pause = scene_tree->is_paused();
		if (tree_pause && !paused) {
			set_paused(tree_pause);
		} else if (!tree_pause && paused) {
			set_paused(tree_pause);
		}
	}
	ERR_FAIL_NULL_EDMSG(scene_tree, "MainLoop does not extend SceneTree!");
	if (!(quitting || paused)) {
		double delta = scene_tree->get_root()->get_physics_process_delta_time();
		_process(delta);
	}
}

void CrowdControl::handle_pending() {
	if (effect_queue.size() == 0) {
		return;
	}
	queue_mutex->lock();
	Ref<CCEffectInstance> effect = effect_queue.pop_front();
	if (effect->get_effect()->get_effect_type() == CCEffect::EffectType::TIMED && running_effects.has(effect->get_effect_id())) {
		String effectID = effect->get_effect_id();
		if (!halted_timers.has(effectID)) {
			halted_timers.insert(effectID, TypedArray<CCEffectInstanceTimed>());
		}
		halted_timers[effectID].append(effect);
	} else if (!try_start(effect)) {
		effect_queue.push_back(effect);
	}
	queue_mutex->unlock();
}

bool CrowdControl::try_start(const Ref<CCEffectInstance> &instance) {
	if (time_until_next_effect > 0) {
		return false;
	}
	float time = Time::get_singleton()->get_unix_time_from_system();
	if (instance->get_effect()->get_delay_until() > time) {
		return false;
	}
	return instance->get_start_time() <= time && start_effect(instance);
}

bool CrowdControl::stop_effect(const Ref<CCEffectInstanceTimed> &instance) {
	String effectID = instance->get_effect_id();
	instance->get_effect_timed()->stop(instance, true);
	call_deferred("emit_signal", "effect_stopped", instance);
	connection->socket_send_action("rpc", connection->build_effect_response(instance, TIMED_END));
	running_effects.erase(effectID);
	if (halted_timers.has(effectID) && halted_timers[effectID].size() != 0) {
		start_effect(halted_timers[effectID].pop_front());
	}
	return true;
}

bool CrowdControl::start_effect(const Ref<CCEffectInstance> &instance) {
	bool dequeue = true;
	Ref<CCEffect> effect = instance->get_effect();
	CCEffect::EffectResult result;
	Ref<CCEffectInstanceTimed> timed = Object::cast_to<CCEffectInstanceTimed>(instance.ptr());
	if (effect->get_effect_type() == CCEffect::EffectType::TIMED) {
		// This might even need a crash, not sure how we savely recover from this
		ERR_FAIL_NULL_V_EDMSG(timed, true, "Timed effect found on non-timed effect instance!");
		if (!timed->is_active()) {
			result = CCEffect::EffectResult::RETRY;
			//Retry
			retry_start_effect(effect, instance, dequeue, result);
			finish_start_effect(effect, instance, dequeue);
			return dequeue;
		}
	}

	result = effect->can_run() ? effect->trigger(instance) : CCEffect::EffectResult::RETRY;
	switch (result) {
		default:
			log_error(vformat("Unhandled EffectResult.%d", (int64_t)result));
			break;
		case CCEffect::EffectResult::FAILURE:
			dequeue_effect(instance, result);
			connection->socket_send_action("rpc", connection->build_effect_response(instance, FAIL_TEMPORARILY));
			break;
		case CCEffect::EffectResult::UNAVAILABLE:
			dequeue_effect(instance, result);
			connection->socket_send_action("rpc", connection->build_effect_response(instance, FAIL_PERMANENTLY));
			break;
		case CCEffect::EffectResult::SUCCESS:
			time_until_next_effect = delay_between_effects;
			call_deferred("emit_signal", "effect_triggered", instance);
			dequeue_effect(instance, CCEffect::EffectResult::SUCCESS);
			connection->socket_send_action("rpc", connection->build_effect_response(instance, SUCCESS));
			break;
		case CCEffect::EffectResult::RUNNING:
			time_until_next_effect = delay_between_effects;
			running_effects.insert(instance->get_effect_id(), timed);
			call_deferred("emit_signal", "effect_started", timed);
			dequeue_effect(instance, CCEffect::EffectResult::SUCCESS);
			connection->socket_send_action("rpc", connection->build_effect_response(instance, SUCCESS));
			connection->socket_send_action("rpc", connection->build_effect_response(instance, TIMED_BEGIN));
			break;
		case CCEffect::EffectResult::RETRY:
			// Retry
			retry_start_effect(effect, instance, dequeue, result);
			finish_start_effect(effect, instance, dequeue);
			break;
	}

	return dequeue;
}

void CrowdControl::retry_start_effect(const Ref<CCEffect> &effect, const Ref<CCEffectInstance> &instance, bool &dequeue, CCEffect::EffectResult &result) {
	instance->set_retry_count(instance->get_retry_count() + 1);
	if (instance->get_retry_count() > effect->get_max_retries()) {
		result = CCEffect::EffectResult::FAILURE;
		cancel_effect(instance);
	} else {
		instance->set_start_time(effect->get_retry_delay() + Time::get_singleton()->get_unix_time_from_system());
		dequeue = false;
	}
}

void CrowdControl::finish_start_effect(const Ref<CCEffect> &effect, const Ref<CCEffectInstance> &instance, bool &dequeue) {
	if (dequeue) {
		effect->set_delay_until(effect->get_pending_delay() + Time::get_singleton()->get_unix_time_from_system());
	} else {
		effect->set_delay_until(instance->get_start_time());
	}
}

void CrowdControl::resume_effect(const Ref<CCEffectTimed> &effect) {
	if (running_effects.has(effect->get_id())) {
		Ref<CCEffectInstanceTimed> timed = running_effects[effect->get_id()];
		effect->resume(timed);
		call_deferred("emit_signal", "effect_resumed", timed);
		connection->socket_send_action("rpc", connection->build_effect_response(timed, TIMED_RESUME));
	}
}

void CrowdControl::pause_effect(const Ref<CCEffectTimed> &effect) {
	if (running_effects.has(effect->get_id())) {
		Ref<CCEffectInstanceTimed> timed = running_effects[effect->get_id()];
		effect->pause(timed);
		call_deferred("emit_signal", "effect_paused", timed);
		connection->socket_send_action("rpc", connection->build_effect_response(timed, TIMED_PAUSE));
	}
}

void CrowdControl::reset_effect(const Ref<CCEffectTimed> &effect) {
	if (running_effects.has(effect->get_id())) {
		Ref<CCEffectInstanceTimed> timed = running_effects[effect->get_id()];
		effect->reset(timed);
		call_deferred("emit_signal", "effect_reset", timed);
		// No rpc?
	}
}

void CrowdControl::load_effects() {
	effect_entries = ResourceLoader::get_singleton()->load(GLOBAL_GET("crowd_control/common/effects"));
	for (int i = 0; i < effect_entries->get_effects().size(); i++) {
		Ref<CCEffect> effect = effect_entries->get_effects()[i];
		register_effect(effect);
	}
}

void CrowdControl::_on_connected() {
	if (connection->get_game_session_id().is_empty()) {
		connection->socket_send_action("whoami");
	} else {
		connection->subscribe();
	}
	call_deferred("emit_signal", "connected");
}

void CrowdControl::_on_disconnected() {
	call_deferred("emit_signal", "disconnected");
}

void CrowdControl::_on_effect_requested(const Ref<CCEffect> &effect) {
	call_deferred("emit_signal", "effect_requested", effect);
}

void CrowdControl::_on_session_started() {
	CrowdControl::singleton->display_message("Started the Crowd Control session!");
	call_deferred("emit_signal", "session_started");
	CrowdControl::singleton->session_active = true;
}

void CrowdControl::queue_effect(const Ref<CCEffect> &effect, User requester, String requestID, bool is_anonymous, HashMap<String, ParameterEntry> parameters) {
	Ref<StreamUser> display_user;
	if (!is_anonymous) {
		display_user = get_stream_user(requester);
	} else {
		display_user = anonymous_user;
	}
	if (effect->get_effect_type() == CCEffect::EffectType::TIMED) {
		create_effect_instance<CCEffectInstanceTimed>(display_user, effect, requester, requestID, parameters);
	} else if (effect->get_effect_type() == CCEffect::EffectType::BID_WAR) {
		create_effect_instance<CCEffectInstanceBidWar>(display_user, effect, requester, requestID, parameters);
	} else if (effect->get_effect_type() == CCEffect::EffectType::PARAMETERS) {
		create_effect_instance<CCEffectInstanceParameters>(display_user, effect, requester, requestID, parameters);
	} else if (effect->get_effect_type() == CCEffect::EffectType::NORMAL) {
		create_effect_instance<CCEffectInstance>(display_user, effect, requester, requestID, parameters);
	}
}

void CrowdControl::dequeue_effect(const Ref<CCEffectInstance> &instance, CCEffect::EffectResult result) {
	call_deferred("emit_signal", "effect_dequeued", instance->get_effect_id(), result);
}

void CrowdControl::cancel_effect(const Ref<CCEffectInstance> &instance) {
	connection->socket_send_action("rpc", connection->build_effect_response(instance, FAIL_TEMPORARILY));
	dequeue_effect(instance, CCEffect::EffectResult::FAILURE);
}

Ref<StreamUser> CrowdControl::get_stream_user(const User &user) {
	Ref<StreamUser> out;
	String user_name = user.name;
	if (!stream_users.has(user_name)) {
		out = StreamUser::make(user_name, user.image_url, user.roles);
		stream_users[user_name] = out;
		Ref<Thread> thread;
		thread.instantiate();
		thread->start(Callable(connection.ptr(), "__download_profile_image").bindv(Array::make(thread, out)));
	} else {
		out = stream_users[user_name];
	}
	return out;
}

template <class T>
void CrowdControl::create_effect_instance(const Ref<StreamUser> &user, const Ref<CCEffect> &effect, User requester, String requestID, HashMap<String, ParameterEntry> parameters) {
	Ref<T> out;
	out.instantiate();
	out->set_id(requestID);
	out->set_user(user);
	out->set_effect(effect);
	out->set_retry_count(0);
	out->set_start_time(Time::get_singleton()->get_unix_time_from_system());

	String effectID = effect->get_id();
	if (effect->get_effect_type() == CCEffect::EffectType::PARAMETERS) {
		if (parameters.size() == 0) {
			cancel_effect(out);
			return;
		}
		Array arr = Array();
		for (KeyValue<String, ParameterEntry> kv : parameters) {
			arr.append(kv.key);
		}
		log_info(arr);
		((Ref<CCEffectInstanceParameters>)out)->assign_parameters(parameters);
	} else if (effect->get_effect_type() == CCEffect::EffectType::BID_WAR) {
		Ref<CCEffectBidWar> bid_war = Object::cast_to<CCEffectBidWar>(effect.ptr());
		Ref<CCEffectInstanceBidWar> instance = Object::cast_to<CCEffectInstanceBidWar>(out.ptr());
		instance->set_bid_id(parameters[vformat("%s_options", bid_war->get_id())].value);
	}
	queue_mutex->lock();
	effect_queue.push_back(out);
	queue_mutex->unlock();
	call_deferred("emit_signal", "effect_queued", out);
}

/*template <class... Args>
void CrowdControl::log_info(const Variant &arg1, const Args &...args) {
	if (debug_info) {
		UtilityFunctions::print_rich(LOG_PREFIX, arg1, args...);
	}
}

template <class... Args>
void CrowdControl::log_warn(const Variant &arg1, const Args &...args) {
	if (debug_warn) {
		UtilityFunctions::push_warning(LOG_PREFIX, arg1, args...);
	}
}

template <class... Args>
void CrowdControl::log_error(const Variant &arg1, const Args &...args) {
	if (debug_error) {
		UtilityFunctions::push_error(LOG_PREFIX, arg1, args...);
	}
}

template <class... Args>
void CrowdControl::log_exception(const Variant &arg1, const Args &...args) {
	if (debug_exception) {
		UtilityFunctions::push_error(LOG_PREFIX, arg1, args...);
	}
}*/

void CrowdControl::log_info(const Variant &arg1) {
	if (debug_info) {
		UtilityFunctions::print_rich(LOG_PREFIX, arg1);
	}
}

void CrowdControl::log_warn(const Variant &arg1) {
	if (debug_warn) {
		UtilityFunctions::push_warning(LOG_PREFIX, arg1);
	}
}

void CrowdControl::log_error(const Variant &arg1) {
	if (debug_error) {
		UtilityFunctions::push_error(LOG_PREFIX, arg1);
	}
}

void CrowdControl::log_exception(const Variant &arg1) {
	if (debug_exception) {
		UtilityFunctions::push_error(LOG_PREFIX, arg1);
	}
}

void CrowdControl::_process(float delta) {
	time_until_next_effect -= delta;
	if (!connection->is_connected_to_crowd_control() && connection->get_time_to_next_ping() <= Time::get_singleton()->get_unix_time_from_system()) {
		connection->set_time_to_next_ping(std::numeric_limits<uint64_t>::max());
#ifdef DEV_MODE
		connection->connect(DEV);
#else
		if (staging) {
			connection->connect(STAGING);
		}
		connection->connect(PRODUCTION);
#endif
	}
	handle_pending();
	for (KeyValue<StringName, Ref<CCEffectInstanceTimed>> kv : running_effects) {
		if (!kv.value->is_paused()) {
			kv.value->set_time_left(kv.value->get_time_left() - delta);
			if (kv.value->get_time_left() < 0) {
				stop_effect(kv.value);
				break; // We may be editing the map here, so it's no longer safe to continue.
			}
		}
	}
	if (connection->is_connected_to_crowd_control()) {
		if (!streamer_user.is_null() && !stream_users.has(streamer_user->get_display_name())) {
			stream_users[streamer_user->get_display_name()] = streamer_user;
			Ref<Thread> thread;
			thread.instantiate();
			thread->start(Callable(connection.ptr(), "__download_profile_image").bindv(Array::make(thread, streamer_user)));
		}
		if (connection->get_time_to_next_ping() <= Time::get_singleton()->get_unix_time_from_system()) {
			connection->socket_send_action("ping");
			log_info("PING");
			connection->set_time_to_next_ping((int)Time::get_singleton()->get_unix_time_from_system() + 300);
			connection->set_time_to_timeout(connection->get_time_to_next_ping() + 300);
		}
	}
}

void CrowdControl::update_effect_status(const Ref<CCEffect> &p_effect, const String &command) {
	connection->socket_send_action("rpc", connection->build_effect_report(p_effect, command));
}

void CrowdControl::post_effect_update(const String &p_data) {
	if (!streamer_user.is_null()) {
		connection->client_post_authorized("/menu/effects", Array(), p_data);
	}
}

CrowdControl::CrowdControl() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	connection.instantiate();

	crowd_user = StreamUser::make("The Crowd", crowd_user_icon);
	anonymous_user = StreamUser::make("Anonymous", temp_user_icon);

	stream_users[crowd_user->get_name()] = crowd_user;
	stream_users[anonymous_user->get_name()] = anonymous_user;
	rng.instantiate();
	rng->randomize();
	queue_mutex.instantiate();
}

CrowdControl::~CrowdControl() {
	if (connection->is_connected_to_crowd_control()) {
		close();
	}
	ERR_FAIL_COND(singleton != this);
}