#ifndef CROWD_CONTROL_CROWD_CONTROL_HPP
#define CROWD_CONTROL_CROWD_CONTROL_HPP

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/cc_effect/cc_effect_bid_war.hpp"
#include "classes/cc_effect/cc_effect_parameters.hpp"
#include "classes/cc_effect/cc_effect_timed.hpp"
#include "classes/cc_effect_instance.hpp"
#include "json/effect_request.hpp"
#include "util/cc_enums.hpp"

using namespace godot;

class StreamUser;
class CCNetworkManager;

/// @brief The main management class for Crowd Control.
/// @details
/// In an attempt to make this as close as possible to a native Godot module
/// this is a singleton registered to the @see Engine singleton.
/// The user in GDScript can use the exposed signals to react to the various
/// events during a session.
///
/// A limitation of the @see ProjectSettings system is that resources cannot be
/// directly loaded in the built-in menu.
/// Instead the user specifies the path to a @see EffectEntries resource that is
/// is loaded when the session starts, which comes with its own set of issues.
///
/// As a lot of the processing happens on the threads monitoring the socket/http client
/// most signals are deferred to the next process frame to allow users to write
/// their code as without having to account for this.
///
/// Because Godot allows to use custom @see MainLoop s that do not rely on
/// @see Node s this is implemented as a standalone module meaning we can not use
/// the `_process` or `_physics_process` functions to handle events or use
/// @SceneTree.paused, choosing to  use threads instead.
/// The `crowd_control/common/process_mode` and `crowd_control/common/use_scene_tree_pause`
/// project settings will allow the user to override this to use the @see SceneTree instead.
class CrowdControl : public Object {
	GDCLASS(CrowdControl, Object)

	friend class CCNetworkManager;

public:
	static const char *platform_names[];

	static const char *effect_instance_status_names[];

private:
	static CrowdControl *singleton;
	const String LOG_PREFIX = "[CC] ";

	Ref<StreamUser> crowd_user;
	Ref<StreamUser> anonymous_user;
	Ref<StreamUser> streamer_user;
	Ref<StreamUser> test_user;
	HashMap<String, Ref<StreamUser>> stream_users = HashMap<String, Ref<StreamUser>>();
	HashMap<StringName, Ref<CCEffect>> effects = HashMap<StringName, Ref<CCEffect>>();
	// For some reason references get lost when using List<>, need to investigate
	TypedArray<CCEffectInstance> effect_queue = TypedArray<CCEffectInstance>();
	HashMap<StringName, TypedArray<CCEffectInstanceTimed>> halted_timers = HashMap<StringName, TypedArray<CCEffectInstanceTimed>>();
	HashMap<StringName, Ref<CCEffectInstanceTimed>> running_effects = HashMap<StringName, Ref<CCEffectInstanceTimed>>();

	// instance properties accessible via bindings
	bool staging = false;
	bool paused = false;
	BroadcasterType broadcaster_type = COMMUNITY;
	bool auto_starting_session = false;

	// ProjectSettings/common
	int game_key = 92;
	String game_name = "Godot Demo";
	int reconnect_retry_count = -1;
	float reconnect_retry_delay = 5.0f;
	float delay_between_effects = 0.5f;
	Ref<CCEffectEntries> effect_entries;
	// ProjectSettings/common/process
	int process_mode = 0;
	// Reference to scenetree if not using internal processing
	SceneTree *scene_tree = nullptr;

	// ProjectSettings/visuals
	Ref<Texture2D> temp_user_icon;
	Color temp_user_color = Color(0.0f, 0.0f, 0.0f, 0.6f);
	Ref<Texture2D> crowd_user_icon;
	Color crowd_user_color = Color(0.094117f, 0.466666f, 0.937254f);
	Ref<Texture2D> error_user_icon;
	Color error_user_color = Color(0.77647f, 0.0f, 0.0f);

	// ProjectSettings/debug
	bool debug_info = true;
	bool debug_warn = true;
	bool debug_error = true;
	bool debug_exception = true;

	bool quitting = false;
	bool quit_on_session_close = false;
	bool session_active = false;

	Ref<CCNetworkManager> connection;
	Ref<RandomNumberGenerator> rng;
	Ref<Thread> process_thread;
	/// @brief Mutex to ensure we don't perform unsafe methods on the effect queue (resizing is not thread-safe)
	Ref<Mutex> queue_mutex;

	float time_until_next_effect = 0;

private:
	void thread_process();
	void tree_process();
	void tree_physics_process();

	void handle_pending();

	bool try_start(const Ref<CCEffectInstance> &instance);
	bool stop_effect(const Ref<CCEffectInstanceTimed> &instance);
	bool start_effect(const Ref<CCEffectInstance> &instance);
	void retry_start_effect(const Ref<CCEffect> &effect, const Ref<CCEffectInstance> &instance, bool &dequeue, CCEffect::EffectResult &result);
	void finish_start_effect(const Ref<CCEffect> &effect, const Ref<CCEffectInstance> &instance, bool &dequeue);
	void resume_effect(const Ref<CCEffectTimed> &effect);
	void pause_effect(const Ref<CCEffectTimed> &effect);
	void reset_effect(const Ref<CCEffectTimed> &effect);

	void load_effects();

	void _on_connected();
	void _on_disconnected();
	void _on_effect_requested(const Ref<CCEffect> &effect);
	void _on_session_started();

	void queue_effect(const Ref<CCEffect> &effect, User requester, String requestID, bool is_anonymous, HashMap<String, ParameterEntry> parameters = HashMap<String, ParameterEntry>());
	void dequeue_effect(const Ref<CCEffectInstance> &instance, CCEffect::EffectResult result);

	template <class T>
	void create_effect_instance(const Ref<StreamUser> &user, const Ref<CCEffect> &effect, User requester, String requestID, HashMap<String, ParameterEntry> parameters = HashMap<String, ParameterEntry>());
	void cancel_effect(const Ref<CCEffectInstance> &instance);

	Ref<StreamUser> get_stream_user(const User &user);

protected:
	static void _bind_methods();

public:
	static CrowdControl *get_singleton();

	int get_game_key() const { return game_key; }
	void set_game_key(int p_game_key) { game_key = p_game_key; }

	String get_game_name() const { return game_name; }
	void set_game_name(const String &p_game_name) { game_name = p_game_name; }

	bool is_staging() const { return staging; }
	void set_staging(bool p_staging) { staging = p_staging; }

	BroadcasterType get_broadcaster_type() const { return broadcaster_type; }
	void set_broadcaster_type(BroadcasterType p_broadcaster_type) { broadcaster_type = p_broadcaster_type; }

	int get_reconnect_retry_count() const { return reconnect_retry_count; }
	void set_reconnect_retry_count(int p_reconnect_retry_count) { reconnect_retry_count = p_reconnect_retry_count; }

	float get_reconnect_retry_delay() const { return reconnect_retry_delay; }
	void set_reconnect_retry_delay(float p_reconnect_retry_delay) { reconnect_retry_delay = p_reconnect_retry_delay; }

	Ref<Texture2D> get_temp_user_icon() const { return temp_user_icon; }
	void set_temp_user_icon(const Ref<Texture2D> &p_temp_user_icon) { temp_user_icon = p_temp_user_icon; }

	Color get_temp_user_color() const { return temp_user_color; }
	void set_temp_user_color(Color p_temp_user_color) { temp_user_color = p_temp_user_color; }

	Ref<Texture2D> get_crowd_user_icon() const { return crowd_user_icon; }
	void set_crowd_user_icon(const Ref<Texture2D> &p_crowd_user_icon) { crowd_user_icon = p_crowd_user_icon; }

	Color get_crowd_user_color() const { return crowd_user_color; }
	void set_crowd_user_color(Color p_crowd_user_color) { crowd_user_color = p_crowd_user_color; }

	Ref<Texture2D> get_error_user_icon() const { return error_user_icon; }
	void set_error_user_icon(const Ref<Texture2D> &p_error_user_icon) { error_user_icon = p_error_user_icon; }

	Color get_error_user_color() const { return error_user_color; }
	void set_error_user_color(Color p_error_user_color) { error_user_color = p_error_user_color; }

	float get_delay_between_effects() const { return delay_between_effects; }
	void set_delay_between_effects(float p_delay_between_effects) { delay_between_effects = p_delay_between_effects; }

	bool is_debug_info() const { return debug_info; }
	void set_debug_info(bool p_debug_info) { debug_info = p_debug_info; }

	bool is_debug_warn() const { return debug_warn; }
	void set_debug_warn(bool p_debug_warn) { debug_warn = p_debug_warn; }

	bool is_debug_error() const { return debug_error; }
	void set_debug_error(bool p_debug_error) { debug_error = p_debug_error; }

	bool is_debug_exception() const { return debug_exception; }
	void set_debug_exception(bool p_debug_exception) { debug_exception = p_debug_exception; }

	bool is_paused() const { return paused; }
	void set_paused(bool p_paused) { paused = p_paused; }

	bool is_auto_starting_session() const { return auto_starting_session; }
	void set_auto_starting_session(bool p_auto_starting_session) { auto_starting_session = p_auto_starting_session; }

	Ref<StreamUser> get_streamer_user() const { return streamer_user; }

	// for some reason more than one argument results in "unresolved externals" errors, need to investigate
	// use vformat(...) instead to format the string into a single object
	/*template <class... Args>
	void log_info(const Variant &arg1, const Args &...args);
	template <class... Args>
	void log_warn(const Variant &arg1, const Args &...args);
	template <class... Args>
	void log_error(const Variant &arg1, const Args &...args);
	template <class... Args>
	void log_exception(const Variant &arg1, const Args &...args);*/

	void log_info(const Variant &arg1);
	void log_warn(const Variant &arg1);
	void log_error(const Variant &arg1);
	void log_exception(const Variant &arg1);

	Error connect_to_crowd_control();
	void close();
	void login(Platform p_platform);
	bool is_connected_to_crowd_control() const;
	bool is_session_active() const;

	void start_session();
	void stop_session();

	void test_effect(const Ref<CCEffect> &effect);
	void test_effect_remotely(const Ref<CCEffect> &effect);

	void display_message(String message, float duration = 5.0);

	void register_effect(const Ref<CCEffect> &effect);
	Ref<CCEffect> get_effect(const StringName &name) const;

	void _process(float delta);

	void update_effect_status(const Ref<CCEffect> &p_effect, const String &command);
	void post_effect_update(const String &p_data = "");

	CrowdControl();
	~CrowdControl();
};

#endif //CROWD_CONTROL_CROWD_CONTROL_HPP