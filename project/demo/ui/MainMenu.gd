extends Control


@export var PlayButton: Button
@export var CCButton: Button
@export var AuthPopup: Popup
@export var LoginPanel: PanelContainer

@export_file("*.tscn", "*.scn") var game_scene : String 


func _ready() -> void:
	PlayButton.grab_focus()
	CrowdControl.disconnected.connect(_on_CrowdControl_disconnected)
	CrowdControl.logged_out.connect(_on_CrowdControl_logged_out)
	CrowdControl.logged_in.connect(_on_CrowdControl_logged_in)
	if CrowdControl.is_connected_to_crowd_control():
		CCButton.text = "Disconnect from Crowd Control"


func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if CrowdControl.is_connected_to_crowd_control():
			CrowdControl.close()
			get_tree().quit(0)


func _on_play_button_pressed() -> void:
	# Start the game
	get_tree().change_scene_to_file(game_scene)


func _on_cc_button_pressed() -> void:
	if CrowdControl.is_connected_to_crowd_control():
		CrowdControl.close()
		CCButton.text = "Connect to Crowd Control"
	else:
		CrowdControl.connect_to_crowd_control()
		get_tree().auto_accept_quit = false
		CCButton.disabled = true


func _on_quit_button_pressed() -> void:
	if CrowdControl.is_session_active() or CrowdControl.is_connected_to_crowd_control():
		CrowdControl.close()
	get_tree().quit(0)


func _on_CrowdControl_logged_out() -> void:
	AuthPopup.popup_centered()
	LoginPanel.initial_focus.grab_focus()


func _on_CrowdControl_logged_in() -> void:
	AuthPopup.hide()
	CCButton.text = "Disconnect from Crowd Control"


func _on_CrowdControl_disconnected() -> void:
	CCButton.text = "Connect to Crowd Control"
	CCButton.disabled = false


func _on_popup_popup_hide() -> void:
	CCButton.disabled = false
	CCButton.grab_focus()
