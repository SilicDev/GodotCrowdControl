extends Node2D


func _on_area_2d_area_entered(area: Area2D) -> void:
	if area.get_parent().get_groups().has("player"):
		area.get_parent().damage(self, 1)
