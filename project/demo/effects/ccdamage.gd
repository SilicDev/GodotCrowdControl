class_name CCDamage
extends CCEffect


func _trigger(_instance: CCEffectInstance) -> EffectResult:
	GameManager.instance.Player.damage(null, 1)
	return SUCCESS


func _can_run() -> bool:
	return GameManager.instance.Player != null and GameManager.instance.Player.alive
