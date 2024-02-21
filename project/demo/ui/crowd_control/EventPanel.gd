extends PanelContainer


@export var EventIcon: TextureRect
@export var RoleLabel: Label
@export var UserIcon: TextureRect
@export var EventTextLabel: RichTextLabel
@export var QuantityLabel: Label
@export var EventValue: Label
@export var TimerBar: ProgressBar

var user: StreamUser
var icon_loaded := false

var effect_instance : CCEffectInstance
# Effect stacking
var quantity := 0
var tween: Tween


func _ready() -> void:
	modulate.a = 0


func _process(_delta: float) -> void:
	if TimerBar.visible:
		var time_left := (effect_instance as CCEffectInstanceTimed).get_time_left()
		TimerBar.value = time_left
		var time_data := Time.get_time_dict_from_unix_time(int(time_left))
		if time_data["hour"] != 0:
			EventValue.text = ("%d:%02d:%02d" % 
					[time_data["hour"], time_data["minute"], time_data["second"]])
		else:
			EventValue.text = "%d:%02d" % [time_data["minute"], time_data["second"]]


func setup(effect: CCEffectInstance) -> void:
	effect_instance = effect
	EventIcon.texture = effect.get_effect().icon
	user = effect.get_user()
	if user.roles.size() != 0:
		RoleLabel.text = user.roles[0]
		RoleLabel.visible = true
	if user.profile_icon != null:
		UserIcon.texture = user.profile_icon
	else:
		user.icon_downloaded.connect(_on_icon_downloaded)
	EventTextLabel.text = ("[center][b]%s[/b]\n%s[/center]" % 
			[user.name.to_upper(), effect.get_effect().display_name])
	if effect is CCEffectInstanceParameters:
		var param_quantity: int = effect.get_parameter("quantity")
		QuantityLabel.text = "x%d" % param_quantity
		quantity = param_quantity
		for param in effect_instance.get_parameter_names():
			if param == "quantity":
				continue
			else:
				EventTextLabel.text += "\n[center]%s[/center]" % effect.get_option_name(param)
	else:
		quantity += 1
	if quantity > 1:
		QuantityLabel.visible = true
	EventValue.text = str(effect.get_effect().price)
	tween = get_tree().create_tween()
	tween.tween_property(self, "modulate:a", 1, 1)
	tween.tween_interval(5)
	tween.tween_property(self, "modulate:a", 0, 1)
	tween.tween_callback(queue_free)
	tween.play()


func setup_timed(effect: CCEffectInstanceTimed) -> void:
	effect_instance = effect
	EventIcon.texture = effect.get_effect().icon
	user = effect.get_user()
	if user.roles.size() != 0:
		RoleLabel.text = user.roles[0]
		RoleLabel.visible = true
	if user.profile_icon != null:
		UserIcon.texture = user.profile_icon
	else:
		user.icon_downloaded.connect(_on_icon_downloaded)
	EventTextLabel.text = ("[center][b]%s[/b]\n%s[/center]" % 
			[user.name.to_upper(), effect.get_effect().display_name])
	EventValue.text = str(effect.get_effect_timed().duration)
	EventValue.remove_theme_color_override("font_color")
	TimerBar.visible = true
	TimerBar.max_value = effect.get_effect_timed().duration
	CrowdControl.effect_stopped.connect(_on_effect_stopped)
	tween = get_tree().create_tween()
	tween.tween_property(self, "modulate:a", 1, 1)
	tween.play()


func _on_icon_downloaded() -> void:
	UserIcon.texture = user.profile_icon


func _on_effect_stopped(effect: CCEffectInstanceTimed) -> void:
	if effect_instance.get_id() == effect.get_id():
		tween.stop()
		tween = get_tree().create_tween()
		tween.tween_property(self, "modulate:a", 0, 1)
		tween.tween_callback(queue_free)
		tween.play()
