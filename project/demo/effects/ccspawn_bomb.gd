class_name CCSpawnBomb
extends CCEffect


@export_file("*.tscn", "*.scn") var bomb_scene : String 

var BombScene : PackedScene


func _trigger(instance: CCEffectInstance) -> EffectResult:
	if BombScene == null:
		BombScene = load(bomb_scene)
	var bomb = BombScene.instantiate()
	bomb.position = GameManager.instance.Player.position
	GameManager.instance.add_child(bomb)
	GameManager.instance.move_child(bomb, 1)
	return SUCCESS


func _can_run():
	return GameManager.instance.Player != null and GameManager.instance.Player.alive
