class_name CCInvertControls
extends CCEffectTimed


func _start(_instance: CCEffectInstanceTimed) -> EffectResult:
	GameManager.instance.Player.controls_inverted = true
	return RUNNING


func _stop(_instance: CCEffectInstanceTimed, _force := false) -> bool:
	GameManager.instance.Player.controls_inverted = false
	return true


func _pause() -> void:
	GameManager.instance.Player.controls_inverted = false


func _resume() -> void:
	GameManager.instance.Player.controls_inverted = true


func _reset() -> void:
	GameManager.instance.Player.controls_inverted = true


func _should_be_running() -> bool:
	return GameManager.instance.Player != null and GameManager.instance.Player.alive
