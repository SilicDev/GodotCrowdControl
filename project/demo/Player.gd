extends Node2D


signal health_changed(value: int)
signal coins_changed(value: int)
signal died()
signal show_price(item: String)
signal upgrade_obtained(upgrade: String, value: int)


enum Facing {
	FRONT,
	BACK,
	LEFT,
	RIGHT,
}


const MAX_HEALTH := 5

@export var AnimPlayer: AnimationPlayer
@export var TerrainManager: Node
@export var Sprite: Sprite2D
@export var HurtBox: Area2D

var facing := Facing.FRONT :
	set(value):
		facing = value
		update_idle_anim()
		
var busy := false

var coins := 0:
	set(value):
		coins = value
		coins_changed.emit(coins)
var health := MAX_HEALTH :
	set(value):
		health = value
		health_changed.emit(health)

var alive : bool :
	get:
		return health > 0

var got_upgrade = false
var tween : Tween

var controls_inverted := false


func _ready() -> void:
	update_idle_anim()


func _process(_delta: float) -> void:
	if not busy:
		var input := _get_input()
		if input == Vector2.UP:
			facing = Facing.BACK
			if not TerrainManager.can_move_to(position + Vector2.UP * 16):
				return
			busy = true
			AnimPlayer.play("move_back")
			tween = create_tween()
			tween.tween_property(self, "position:y", position.y - 16, 0.4)
			tween.finished.connect(_on_tween_finished)
		elif input == Vector2.DOWN:
			facing = Facing.FRONT
			if not TerrainManager.can_move_to(position + Vector2.DOWN * 16):
				return
			busy = true
			AnimPlayer.play("move_front")
			tween = create_tween()
			tween.tween_property(self, "position:y", position.y + 16, 0.4)
			tween.finished.connect(_on_tween_finished)
		elif input == Vector2.RIGHT:
			facing = Facing.RIGHT
			if not TerrainManager.can_move_to(position + Vector2.RIGHT * 16):
				return
			busy = true
			AnimPlayer.play("move_right")
			tween = create_tween()
			tween.tween_property(self, "position:x", position.x + 16, 0.4)
			tween.finished.connect(_on_tween_finished)
		elif input == Vector2.LEFT:
			facing = Facing.LEFT
			if not TerrainManager.can_move_to(position + Vector2.LEFT * 16):
				return
			busy = true
			AnimPlayer.play("move_left")
			tween = create_tween()
			tween.tween_property(self, "position:x", position.x - 16, 0.4)
			tween.finished.connect(_on_tween_finished)


func _input(event: InputEvent) -> void:
	if not busy and event.is_action_pressed("ui_select"):
		var item: String = TerrainManager.get_item_on(position)
		if item == "pick_upgrade" and not got_upgrade and coins >= 10:
			coins -= 10
			got_upgrade = true
			TerrainManager.clear_item(position)
			show_price.emit("")
			upgrade_obtained.emit("pickaxe", 1)
			return
		busy = true
		match facing:
			Facing.BACK:
				TerrainManager.try_mine(position + Vector2.UP * 16, got_upgrade)
				if got_upgrade:
					AnimPlayer.play("mine2_back")
				else:
					AnimPlayer.play("mine_back")
			Facing.FRONT:
				TerrainManager.try_mine(position + Vector2.DOWN * 16, got_upgrade)
				if got_upgrade:
					AnimPlayer.play("mine2_front")
				else:
					AnimPlayer.play("mine_front")
			Facing.RIGHT:
				TerrainManager.try_mine(position + Vector2.RIGHT * 16, got_upgrade)
				if got_upgrade:
					AnimPlayer.play("mine2_right")
				else:
					AnimPlayer.play("mine_right")
			Facing.LEFT:
				TerrainManager.try_mine(position + Vector2.LEFT * 16, got_upgrade)
				if got_upgrade:
					AnimPlayer.play("mine2_left")
				else:
					AnimPlayer.play("mine_left")


func damage(source: Node2D, amount: int) -> void:
	if source != self:
		health = max(0, health - amount)
		if amount > 0:
			var damage_tween := create_tween()
			damage_tween.tween_property(self, "visible", false, 0.1)
			damage_tween.tween_property(self, "visible", true, 0.1)
			damage_tween.set_loops(5)
			damage_tween.play()
		if not alive:
			on_death()


func on_death() -> void:
	Sprite.hide()
	HurtBox.process_mode = Node.PROCESS_MODE_DISABLED
	died.emit()


func update_idle_anim() -> void:
	match facing:
		Facing.FRONT:
			AnimPlayer.play("idle_front")
		Facing.BACK:
			AnimPlayer.play("idle_back")
		Facing.RIGHT:
			AnimPlayer.play("idle_right")
		Facing.LEFT:
			AnimPlayer.play("idle_left")


func _get_input() -> Vector2:
	var input := Vector2(
		Input.get_action_strength("ui_right") - Input.get_action_strength("ui_left"),
		Input.get_action_strength("ui_down") - Input.get_action_strength("ui_up")
	)
	if abs(input.x) > abs(input.y):
		input.x = sign(input.x)
		input.y = 0
	else:
		input.x = 0
		input.y = sign(input.y)
	if controls_inverted:
		input *= -1
	return input


func _on_tween_finished() -> void:
	update_idle_anim()
	var item: String = TerrainManager.get_item_on(position)
	if item == "gold":
		coins += 1
		TerrainManager.clear_item(position)
	if item == "pick_upgrade" and not got_upgrade:
		show_price.emit(item)
	else:
		show_price.emit("")
	# allow the player to act again
	busy = false


func _on_animation_player_animation_finished(anim_name: StringName) -> void:
	if anim_name.begins_with("mine"):
		update_idle_anim()
		busy = false
