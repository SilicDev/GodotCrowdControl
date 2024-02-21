extends Control


@export var MessageQueue: VBoxContainer
@export var EffectQueue: HBoxContainer
@export var EventQueue: VBoxContainer
@export var DebuffQueue: VBoxContainer

@export_file("*.tscn", "*.scn") var event_panel_scene : String 
@export_file("*.tscn", "*.scn") var message_panel_scene : String 
@export_file("*.tscn", "*.scn") var queue_panel_scene : String 

# loading these onready instead of exporting scenes is safer in case of cyclic references
@onready var EventPanelScene: PackedScene = load(event_panel_scene)
@onready var MessagePanelScene: PackedScene = load(message_panel_scene)
@onready var QueuePanelScene: PackedScene = load(queue_panel_scene)


## Connects the signals of the CrowdControl singleton to handle the effect queue
## on the overlay
func _ready() -> void:
	CrowdControl.session_started.connect(_on_session_started)
	CrowdControl.effect_queued.connect(_on_effect_queued)
	CrowdControl.effect_dequeued.connect(_on_effect_dequeued)
	CrowdControl.effect_triggered.connect(_on_effect_triggered)
	CrowdControl.effect_started.connect(_on_effect_started)
	CrowdControl.message_display_requested.connect(_on_message_display_requested)


func _on_session_started() -> void:
	pass


## Adds a new queue icon to the effect queue and the effect is not already in
## the queue 
## [b]Note:[/b] increasing and decreasing of the queue count is handled by the
## queue panel
func _on_effect_queued(effect: CCEffectInstance) -> void:
	var in_queue := false
	for c in EffectQueue.get_children():
		if c.effect_id == effect.get_effect_id():
			in_queue = true
			break
	if not in_queue:
		var panel := QueuePanelScene.instantiate()
		panel.effect_id = effect.get_effect_id()
		EffectQueue.add_child(panel)


func _on_effect_dequeued(_id: String, _result: CCEffect.EffectResult) -> void:
	pass


## Adds information about an effect that was just triggered to the overlay
func _on_effect_triggered(effect: CCEffectInstance) -> void:
	var event_panel := EventPanelScene.instantiate()
	EventQueue.add_child(event_panel)
	event_panel.setup(effect)


## Adds information about a timed effect that was just started to the overlay
func _on_effect_started(effect: CCEffectInstanceTimed) -> void:
	var event_panel := EventPanelScene.instantiate()
	EventQueue.add_child(event_panel)
	event_panel.setup(effect)
	event_panel = EventPanelScene.instantiate()
	DebuffQueue.add_child(event_panel)
	event_panel.setup_timed(effect)


## Displays an information message requested by Crowd Control
func _on_message_display_requested(msg: String, duration: float, icon: Texture2D) -> void:
	var message_panel := MessagePanelScene.instantiate()
	MessageQueue.add_child(message_panel)
	message_panel.setup(msg, duration, icon)
