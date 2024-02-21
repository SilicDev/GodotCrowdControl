extends PanelContainer


@export var initial_focus: Control


func _on_twitch_pressed() -> void:
	CrowdControl.login(CrowdControl.TWITCH)


func _on_youtube_pressed() -> void:
	CrowdControl.login(CrowdControl.YOUTUBE)


func _on_discord_pressed() -> void:
	CrowdControl.login(CrowdControl.DISCORD)


func _on_back_pressed() -> void:
	get_parent().hide()
	CrowdControl.close()
