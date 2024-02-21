extends PanelContainer


@export var CountLabel: Label
@export var EffectIcon: TextureRect

var effect_id: StringName
var count := 1


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	CrowdControl.effect_queued.connect(_on_effect_queued)
	CrowdControl.effect_dequeued.connect(_on_effect_dequeued)
	update_label()


func update_label() -> void:
	CountLabel.text = str(count)


func _on_effect_queued(effect: CCEffectInstance) -> void:
	if effect.get_effect_id() == effect_id:
		count += 1
		update_label()


func _on_effect_dequeued(id: String, _result: CCEffect.EffectResult) -> void:
	if id == effect_id:
		count -= 1
		if count == 0:
			queue_free()
		else:
			update_label()
