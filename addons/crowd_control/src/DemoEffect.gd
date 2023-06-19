class_name DemoEffect
extends CCEffect


func _init() -> void:
	code = "demo"
	type = EffectType.TIMED
	duration = 30

func update(delta: float) -> void:
	super.update(delta)
	print(elapsed)
	pass

func draw(canvas: Control, delta: float) -> void:
	canvas.draw_string(canvas.get_theme_default_font(), canvas.size / 2, "Demo activated", HORIZONTAL_ALIGNMENT_CENTER, -1, 28)
	pass
