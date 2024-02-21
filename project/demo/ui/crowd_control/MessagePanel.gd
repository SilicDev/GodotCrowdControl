extends PanelContainer


@export var Message: Label
@export var Icon: TextureRect


func setup(msg: String, duration: float = 5.0, icon: Texture2D = null) -> void:
	Message.text = msg
	if icon != null:
		Icon.texture = icon
	get_tree().create_timer(duration).timeout.connect(_on_timeout)


func _on_timeout() -> void:
	var tween = get_tree().create_tween()
	tween.tween_property(self, "modulate:a", 0, 1)
	tween.tween_callback(queue_free)
	tween.play()
