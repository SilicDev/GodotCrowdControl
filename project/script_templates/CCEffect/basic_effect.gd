# meta-name: Basic Effect
# meta-default: true
# meta-use_space_indent: 4

#class_name 
extends _BASE_


func _trigger(instance: CCEffectInstance) -> EffectResult:
    return SUCCESS


func _can_run() -> bool:
    return true