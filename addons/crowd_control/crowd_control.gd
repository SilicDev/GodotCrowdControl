@tool
extends EditorPlugin

const AUTOLOAD_NAME = "CrowdControl"

func _enter_tree() -> void:
	# Initialization of the plugin goes here.
	add_autoload_singleton(AUTOLOAD_NAME, "res://addons/crowd_control/src/autoload/CrowdControl.gd")
	pass


func _exit_tree() -> void:
	# Clean-up of the plugin goes here.
	remove_autoload_singleton(AUTOLOAD_NAME)
	pass
