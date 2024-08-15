#ifndef CROWD_CONTROL_CC_EFFECT_TIMED_HPP
#define CROWD_CONTROL_CC_EFFECT_TIMED_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/cc_effect_instance.hpp"
#include "classes/stream_user.hpp"
#include "util/binding_macros.hpp"

class CCEffect;

class CCEffectTimed : public CCEffect {
	GDCLASS(CCEffectTimed, CCEffect)

protected:
	static void _bind_methods();

private:
	float duration = 60.0f;
	bool paused;

public:
	virtual EffectType get_effect_type() const override { return TIMED; }

	float get_duration() const { return duration; }
	void set_duration(float p_duration) { duration = p_duration; }

	bool is_paused() const { return paused; }
	void set_paused(bool p_paused) { paused = p_paused; }

	EffectResult trigger(Ref<CCEffectInstance> effect) override;
	virtual EffectResult start(Ref<CCEffectInstanceTimed> effect);
	virtual bool stop(Ref<CCEffectInstanceTimed> effect, bool force = false);
	virtual void pause(Ref<CCEffectInstanceTimed> effect);
	virtual void resume(Ref<CCEffectInstanceTimed> effect);
	virtual void reset(Ref<CCEffectInstanceTimed> effect);
	bool should_be_running() const;

	GDVIRTUAL1R(EffectResult, _start, Ref<CCEffectInstanceTimed>)
	GDVIRTUAL2R(bool, _stop, Ref<CCEffectInstanceTimed>, bool)
	GDVIRTUAL0(_pause)
	GDVIRTUAL0(_resume)
	GDVIRTUAL0(_reset)
	GDVIRTUAL0RC(bool, _should_be_running)
};

#endif // CROWD_CONTROL_CC_EFFECT_TIMED_HPP