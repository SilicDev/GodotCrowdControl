class_name CCColorChangeBidWar
extends CCEffectBidWar


func _trigger(instance: CCEffectInstance) -> EffectResult:
	var bidwar_instance := (instance as CCEffectInstanceBidWar)
	var bidname := bidwar_instance.get_bid_name()
	match bidname:
		"Red":
			GameManager.instance.Player.modulate = Color.RED
		"Blue":
			GameManager.instance.Player.modulate = Color.BLUE
		"Green":
			GameManager.instance.Player.modulate = Color.GREEN
	return SUCCESS


func _can_run() -> bool:
	return GameManager.instance.Player != null and GameManager.instance.Player.alive
