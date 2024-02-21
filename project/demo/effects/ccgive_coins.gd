class_name CCGiveCoins
extends CCEffectParameters


func _trigger(instance: CCEffectInstance) -> EffectResult:
	var param_instance := (instance as CCEffectInstanceParameters)
	match param_instance.get_option_name("givecoins_totalcoins"):
		"One":
			GameManager.instance.Player.coins += 1
		"Two":
			GameManager.instance.Player.coins += 2
		"Three":
			GameManager.instance.Player.coins += 3
		"Four":
			GameManager.instance.Player.coins += 4
		"Five":
			GameManager.instance.Player.coins += 5
	return SUCCESS


func _can_run() -> bool:
	return GameManager.instance.Player != null and GameManager.instance.Player.alive
