@icon("res://addons/crowd_control/icon.png")
class_name _CrowdControl # dirty hack for documentation, also ensures that enums work even if the autoload is disabled
extends Control

enum EffectResult {
	SUCCESS = 0x00, ## The effect executed successfully.
	FAILURE = 0x01, ## The effect failed to trigger, but is still available for use. Viewer(s) will be refunded.
	UNAVAILABLE = 0x02, ## Same as [enum _CrowdControl.EffectResult].FAILURE but the effect is no longer available for use.
	RETRY = 0x03, ## The effect cannot be triggered right now, try again in a few seconds.
	QUEUE = 0x04, ## INTERNAL USE ONLY. The effect has been queued for execution after the current one ends.
	RUNNING = 0x05, ## INTERNAL USE ONLY. The effect triggered successfully and is now active until it ends.
	PAUSED = 0x06, ## The timed effect has been paused and is now waiting.
	RESUMED = 0x07, ## The timed effect has been resumed and is counting down again.
	FINISHED = 0x08, ## The timed effect has finished.
	VISIBLE = 0x80, ## The effect should be shown in the menu.
}

enum EffectClass {
	NOT_VISIBLE = 0x81, ## The effect should be hidden in the menu.
	SELECTABLE = 0x82, ## The effect should be selectable in the menu.
	NOT_SELECTABLE = 0x83, ## The effect should be unselectable in the menu.
}

enum SystemStatus {
	NOT_READY = 0xFF, ## The processor isn't ready.
}

enum RequestType {
	TEST = 0x00, ## Respond as with a normal request but don’t actually activate the effect.
	START = 0x01, ## A standard effect request. A response (see above) is always required.
	STOP = 0x02, ## A request to stop all instances of the requested effect code.
	
	PLAYER_INFO = 0xE0, ## RESERVED
	LOGIN = 0xF0, ## Indicates that this is a login request or response.
	KEEP_ALIVE = 0xFF, ## Can be used in either direction to keep connections open or test connection status. Responses are neither expected nor given.
}

enum ResponseType {
	EFFECT_REQUEST = 0x00, 	## A response to an effect request.[br]The response ID should match the request ID. Multiple responses with the same ID are allowed in the case of timed effects.
	EFFECT_STATUS = 0x01, ## A status update on a type of effect.[br]These messages concern the specified effect in general, not a specific instance.
	
	LOGIN = 0xF0, ## Indicates that this is a login request or response.
	LOGIN_SUCCESS = 0xF1, ## Indicates the login was successful.
	DISCONNECT = 0xFE,
	KEEP_ALIVE = 0xFF, ## Can be used in either direction to keep connections open or test connection status. Responses are neither expected nor given.[br]Request ID will be 0.
}

## RESERVED
enum ServiceType {
	
}

## Information text that is displayed when not connected yet.
const INITIAL_CONNECT_WARNING := "This plugin requires the Crowd Control client software.\nPlease see https://crowdcontrol.live/ for more information."

## Reference to this object, intended to make sure that only the autoload is active at anytime
var instance
## The TCP client managing the connection to the CrowdControl client.
var client: CCSimpleTCPClient

## [code]true[/code] if the client has connected at least once. Hides [member INITIAL_CONNECT_WARNING]
var _connected_once := false

## Indicates if the game is ready to process CrowdControl events.
var game_ready := false

## Stores when the CrowdControl processer last run.
var last_time := 0.0

## Stores all registered effects
var effects := {}

## Host to connect to, should be localhost in most cases
var host : String = "127.0.0.1"
## Port to connect to, as defined by the CrowdControl Effect Pack
var port : int = 58430

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	if CrowdControl.instance != null:
		push_warning("CrowdControl autoload should never be instantiated!")
		queue_free()
		return
	
	CrowdControl.instance = self
	
	z_index = RenderingServer.CANVAS_ITEM_Z_MAX
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	focus_mode = Control.FOCUS_NONE
	custom_minimum_size = get_tree().root.size
	
	call_deferred("setup_effects")
	client = CCSimpleTCPClient.new()
	client.socket_connected.connect(_on_socket_connected)
	client.request_received.connect(_on_request_received)
	client.connect_to_host(host, port)
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if not game_ready:
		return
	queue_redraw()
	pass

func _physics_process(delta: float) -> void:
	last_time = Time.get_ticks_msec()
	if not game_ready:
		return
	
	for effect in get_active_effects():
		match (effect.type):
			CCEffect.EffectType.TIMED:
				if effect.elapsed <= effect.duration:
					effect.update(delta)
					if get_tree().paused:
						effect.elapsed -= delta
				else:
					effect.try_stop()
			CCEffect.EffectType.BID_WAR:
				effect.update(delta)
			CCEffect.EffectType.INSTANT:
				effect.update(delta)
				effect.try_stop()
	pass

func _draw() -> void:
	var str := ""
	if not client.connected:
		str += "Crowd Control - Not Connected!"
	if not _connected_once:
		str += INITIAL_CONNECT_WARNING
	draw_string(get_theme_default_font(), Vector2.DOWN * 16, str, HORIZONTAL_ALIGNMENT_LEFT)
	
	for effect in get_active_effects():
		effect.draw(self, get_process_delta_time())
	pass

func _exit_tree() -> void:
	client.disconnect_from_host()
	for effect in effects.values():
		effect.try_stop()
		effect.unload()
	pass

## Loads every class extending [class CCEffect] and registers them as effects
func setup_effects() -> void:
	var effect_classes := []
	var classes := ProjectSettings.get_global_class_list()
	__get_child_classes(classes, effect_classes, "CCEffect", true)
	for c in effect_classes:
		var effect = load(c["path"]).new()
		register_effect(effect)
	pass

## Registers an effect, setting its id and performing the load step
func register_effect(effect: CCEffect) -> void:
	effect.id = effects.keys().size()
	effect.load()
	effects[effect.code] = effect
	pass

## Constructs a response for [param request] and sends it to the CrowdControl client
func respond(request: CCSimpleTCPClient.CCRequest, result: EffectResult, time_remaining := 0, message := "") -> bool:
	var response := CCSimpleTCPClient.CCResponse.new()
	response.id = request.id
	response.status = result
	response.time_remaining = time_remaining * 1000
	response.message = message
	response.type = ResponseType.EFFECT_REQUEST
	return await client.respond(response)

## Returns all currently active effects
func get_active_effects() -> Array:
	return effects.values().filter(func(e): return e.active)

## Returns all currently active effects belonging to [param group]
func get_active_effects_in_group(group: String) -> Array:
	return effects.values().filter(func(e): return e.active and e.group == group)

func _on_socket_connected() -> void:
	_connected_once = true
	client.socket_connected.disconnect(_on_socket_connected)
	pass

## process the received [param request] and sends a response if needed
func _on_request_received(request: CCSimpleTCPClient.CCRequest) -> void:
	match request.type:
		RequestType.TEST, RequestType.START:
			print("Got an effect request [%d:%s]." % [request.id, request.code])
			if not request.code in effects.keys():
				push_warning("Effect %s not found." % request.code)
				respond(request, EffectResult.UNAVAILABLE)
				return
			
			var effect: CCEffect = effects[request.code]
			var len: int = effect.parameter_types.size()
			if request.parameters.size() < len:
				respond(request, EffectResult.FAILURE)
				return
			
			var params := []
			params.resize(len)
			for i in range(len):
				params[i] = request.parameters[i]
				if typeof(params[i]) != effect.parameter_types[i]:
					respond(request, EffectResult.FAILURE)
					return
			
			if effect.type == CCEffect.EffectType.BID_WAR:
				for e in get_active_effects_in_group(effect.group):
					e.try_stop()
			
			if not effect.try_start(params):
				push_warning("Effect %s could not start." % request.code)
				respond(request, EffectResult.RETRY)
				return
		
			print("Effect %s started." % request.code)
			respond(request, EffectResult.SUCCESS, effect.duration)
		RequestType.STOP:
			print("Got an effect stop request [%d:%s]." % [request.id, request.code])
			if not request.code in effects.keys():
				push_warning("Effect %s not found." % request.code)
				respond(request, EffectResult.UNAVAILABLE)
				return
			
			var effect: CCEffect = effects[request.code]
			
			if effect.type == CCEffect.EffectType.BID_WAR:
				for e in get_active_effects_in_group(effect.group):
					e.try_stop()
			
			if not effect.try_stop():
				push_warning("Effect %s could not stop." % request.code)
				respond(request, EffectResult.RETRY)
				return
			
			print("Effect %s stopped." % request.code)
			respond(request, EffectResult.SUCCESS)
		RequestType.KEEP_ALIVE:
			print("Keep alive received.")
	pass

## Finds all custom child classes of [param base_class].
## [param classes] is expected to be a collection of custom classes. See also [method ProjectSettings.get_global_class_list]
## [br]If [param recursive] is [code]true[/code] also finds the children of the child classes.
func __get_child_classes(classes: Array, out: Array, base_class: StringName, recursive := false) -> void:
	for c in classes:
		if c.base == base_class:
			out.append(c)
	if recursive:
		for c in out:
			var arr := []
			__get_child_classes(classes, arr, c["class"], true)
			c["children"] = arr
