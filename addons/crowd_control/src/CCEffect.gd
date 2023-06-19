@icon("res://addons/crowd_control/icon.png")
class_name CCEffect
extends RefCounted

enum EffectType {
	INSTANT = 0,
	TIMED = 1,
	BID_WAR = 2,
}

## !DO NO SET MANUALLY!
## [br]The id for this effect. Set by [method CrowdControl.register_effect]
var id: int

## The string code of this effect. CrowdControl uses this to identify the different effects.
var code: String

## Returns if this effect is currently active. If set to [code]false[/code] resets the effect for the next use.
var active := false:
	set(value):
		if value == active:
			return
		active = value
		if value:
			elapsed = 0
			start()
		else:
			end()

## A mutex to secure the active state of the effect.
var lock := Mutex.new()

## Time elapsed since this effect was activated.
var elapsed : float

## The type of the effect. [member EffectType.TIMED] will call their [method update] function every physics frame.
var type: EffectType = EffectType.INSTANT

## How long this effect will be active for when activated, in seconds.
var duration := 0

## The expected types for the parameters of this effect. See [enum Variant.Type]
var parameter_types: Array[Variant.Type]

## The parameter values submitted when this effect was started
var parameters: Array

## The group of this effect
var group: String

## The load step. Load any resources required for this effect in this method.
func load() -> void:
	print("%s was loaded [%d]" % [code, id])

## The unload step. Free any resources used for this effect in this method.
func unload() -> void:
	print("%s was unloaded [%d]" % [code, id])

## The start step. Perform any actions that need to happen when this effect is triggered in this method.
func start() -> void:
	print("%s was started [%d]" % [code, id])

## The start step. Perform any cleanup that need to happen when this effect ends in this method.
func end() -> void:
	print("%s was stopped [%d]" % [code, id])

## Called every physics frame when this effect is active. For instant effect this will be called once.
func update(delta: float) -> void:
	elapsed += delta

## Called every frame when this effect is active.
func draw(canvas: Control, delta: float) -> void:
	pass

## Returns whether this effect is ready to be used.
## Custom subclasses may override this to defer this effect until all resources are loaded.
func is_ready() -> bool:
	return true

## Tries to start this effect with the entered [param params]. Returns [code]true[/code] if successful, [code]false[/code] otherwise.
func try_start(params := []) -> bool:
	lock.lock()
	if active || not is_ready():
		lock.unlock()
		return false
	parameters = params
	active = true
	lock.unlock()
	return true

## Tries to stop this effect. Returns [code]true[/code] if successful, [code]false[/code] otherwise.
func try_stop() -> bool:
	lock.lock()
	if not active:
		lock.unlock()
		return false
	active = false
	lock.unlock()
	return true
