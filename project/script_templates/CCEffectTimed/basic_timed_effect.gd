# meta-name: Basic Timed Effect
# meta-default: true
# meta-use_space_indent: 4

#class_name
extends _BASE_


func _start(instance: CCEffectInstanceTimed) -> EffectResult:
	return RUNNING


func _stop(instance: CCEffectInstanceTimed, force := false) -> bool:
	return true


func _pause() -> void:
	pass


func _resume() -> void:
	pass


func _reset() -> void:
	pass


func _should_be_running() -> bool:
	return true