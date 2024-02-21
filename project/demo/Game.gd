# class_name is needed so we can use static variables
class_name GameManager
extends Node


static var instance

@export var Player : Node2D
@export var Camera : Camera2D
@export var Terrain : TileMap
@export var HUD: Control

@export_file("*.tscn", "*.scn") var bomb_scene : String 

@onready var BombScene : PackedScene = load(bomb_scene)


func _ready() -> void:
	get_tree().root.canvas_item_default_texture_filter = Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS
	if CrowdControl.is_connected_to_crowd_control() and not CrowdControl.is_session_active():
		CrowdControl.start_session()
	instance = self


func _process(_delta: float) -> void:
	# Could probably tween this with the player
	Camera.position = Player.position


func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if CrowdControl.is_connected_to_crowd_control():
			CrowdControl.disconnected.connect(_on_CrowdControl_disconnected)
			CrowdControl.close()


func can_move_to(pos: Vector2) -> bool:
	var tile_pos := Terrain.local_to_map(Terrain.to_local(pos))
	return (Terrain.get_cell_source_id(0, tile_pos) == 0 
			and Terrain.get_cell_source_id(2, tile_pos) == -1)


func try_mine(pos: Vector2, has_upgrade := false) -> bool:
	var tile_pos := Terrain.local_to_map(Terrain.to_local(pos))
	var can_mine := Terrain.get_cell_source_id(2, tile_pos) == 2
	if has_upgrade:
		can_mine = can_mine or Terrain.get_cell_source_id(2, tile_pos) == 3
	if can_mine:
		var item := Terrain.get_cell_source_id(1, tile_pos)
		if item == 6:
			Terrain.set_cell(1, tile_pos)
			var bomb := BombScene.instantiate()
			bomb.position = pos
			add_child(bomb)
			# move infront of player
			move_child(bomb, 1)
		Terrain.set_cell(2, tile_pos)
		
	return can_mine


func get_item_on(pos: Vector2) -> String:
	var tile_pos := Terrain.local_to_map(Terrain.to_local(pos))
	var item := Terrain.get_cell_source_id(1, tile_pos)
	var item_coord := Terrain.get_cell_atlas_coords(1, tile_pos)
	if item == 4:
		if item_coord == Vector2i(0, 0):
			return "pickaxe"
		if item_coord == Vector2i(1, 0):
			return "pick_upgrade"
		if item_coord == Vector2i(2, 0):
			return "gold"
	return ""


func clear_item(pos: Vector2) -> void:
	var tile_pos := Terrain.local_to_map(Terrain.to_local(pos))
	Terrain.set_cell(1, tile_pos)


func _on_player_coins_changed(value: int) -> void:
	HUD.change_coins(value)


func _on_player_health_changed(value: int) -> void:
	HUD.change_health(value)


func _on_player_died() -> void:
	HUD.show_game_over()


func _on_player_show_price(item: String) -> void:
	var value = -1
	if item == "pick_upgrade":
		value = 10
	HUD.show_price(value)


func _on_player_upgrade_obtained(upgrade: String, value: int) -> void:
	if upgrade == "pickaxe":
		HUD.update_pickaxe(value)


## Handles quitting the game after player closed the window, ensures session is closed
func _on_CrowdControl_disconnected() -> void:
	# quit(0) fails with an encode error. Godot bug?
	get_tree().quit()
