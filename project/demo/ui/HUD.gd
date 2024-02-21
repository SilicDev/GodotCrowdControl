extends Control


@export var HealthBar: ProgressBar
@export var CoinCount: Label
@export var PickUpgrade: TextureRect
@export var GameOverScreen: PanelContainer
@export var RetryButton: Button
@export var PricePopup: Panel
@export var PriceLabel: Label

@export_file("*.tscn", "*.scn") var main_scene : String 


func change_coins(value: int) -> void:
	CoinCount.text = str(value).lpad(3, "0")


func change_health(value: int) -> void:
	HealthBar.value = value


func show_game_over() -> void:
	GameOverScreen.show()


func _on_game_over_screen_visibility_changed() -> void:
	if GameOverScreen.visible:
		RetryButton.grab_focus()
	else:
		grab_focus() # release focus from ui


func _on_retry_pressed() -> void:
	get_tree().reload_current_scene()


func _on_back_to_title_pressed() -> void:
	get_tree().change_scene_to_file(main_scene)


func show_price(value: int) -> void:
	if value <= 0:
		if PricePopup.visible:
			PricePopup.hide()
		return
	PriceLabel.text = str(value)
	PricePopup.show()


func update_pickaxe(value: int) -> void:
	match value:
		0:
			PickUpgrade.texture = preload("res://demo/assets/ui/pick1.png")
		1:
			PickUpgrade.texture = preload("res://demo/assets/ui/pick2.png")
		_:
			push_warning("Unknown upgrade tier for pickaxe!")
