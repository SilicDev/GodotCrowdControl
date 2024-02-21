class_name CCTakeCoins
extends CCEffectParameters


func _trigger(instance: CCEffectInstance) -> EffectResult:
	var param_instance := (instance as CCEffectInstanceParameters)
	var quantity: int = param_instance.get_parameter("quantity")
	GameManager.instance.Player.coins = max(0, GameManager.instance.Player.coins - quantity)
	return SUCCESS


func _can_run() -> bool:
	return (GameManager.instance.Player != null and GameManager.instance.Player.alive 
			and GameManager.instance.Player.coins > 0)
