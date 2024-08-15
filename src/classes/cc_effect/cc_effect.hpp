#ifndef CROWD_CONTROL_CC_EFFECT_HPP
#define CROWD_CONTROL_CC_EFFECT_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "classes/cc_effect_instance.hpp"
#include "classes/stream_user.hpp"
#include "util/binding_macros.hpp"

using namespace godot;

class CrowdControl;
class CCEffectInstance;

class CCEffect : public Resource {
	GDCLASS(CCEffect, Resource)

public:
	enum EffectType {
		NORMAL,
		TIMED,
		BID_WAR,
		PARAMETERS,
	};

	enum Morality {
		NEUTRAL,
		GOOD,
		EVIL,
	};

	enum EffectResult {
		SUCCESS,
		FAILURE,
		UNAVAILABLE,
		RETRY,
		QUEUE,
		RUNNING,
	};

	enum EffectMenuStatus {
		MENU_UNAVAILABLE,
		MENU_AVAILABLE,
		MENU_VISIBLE,
		MENU_HIDDEN,
	};
	static const char *effect_menu_status_names[];

protected:
	static void _bind_methods();

private:
	bool pooling_disabled = false;
	bool sellable = true;
	bool visible = true;
	Morality morality = NEUTRAL;
	Ref<Texture2D> icon;
	Color icon_color = Color(1, 1, 1);
	StringName id;
	String display_name;
	String description;
	int price = 10;
	int max_retries = 3;
	float retry_delay = 5.0f;
	float pending_delay = 0.5f;
	int session_max = 0;
	float delay_until = 0;
	PackedStringArray categories = PackedStringArray();

public:
	virtual EffectType get_effect_type() const { return NORMAL; }

	/* VIRTUAL CALLERS */
	/*
	 * Use these functions
	 */

	virtual bool can_run();
	virtual EffectResult trigger(Ref<CCEffectInstance> effect);

	GDVIRTUAL0RC(bool, _can_run)
	GDVIRTUAL1R(int, _trigger, Ref<CCEffectInstance>)

	bool is_pooling_disabled() const { return pooling_disabled; }
	void set_pooling_disabled(bool p_pooling_disabled);

	bool is_sellable() const { return sellable; }
	void set_sellable(bool p_sellable);

	bool is_visible() const { return visible; }
	void set_visible(bool p_visible);

	Morality get_morality() const { return morality; }
	void set_morality(Morality p_morality) { morality = p_morality; }

	Ref<Texture2D> get_icon() const { return icon; }
	void set_icon(const Ref<Texture2D> &p_icon) { icon = p_icon; }

	Color get_icon_color() const { return icon_color; }
	void set_icon_color(Color p_icon_color) { icon_color = p_icon_color; }

	StringName get_id() const { return id; }
	void set_id(const StringName &p_id) { id = p_id; }

	String get_display_name() const { return display_name; }
	void set_display_name(const String &p_display_name) { display_name = p_display_name; }

	String get_description() const { return description; }
	void set_description(const String &p_description) { description = p_description; }

	int get_price() const { return price; }
	void set_price(int p_price);

	int get_max_retries() const { return max_retries; }
	void set_max_retries(int p_max_retries) { max_retries = p_max_retries; }

	float get_retry_delay() const { return retry_delay; }
	void set_retry_delay(float p_retry_delay) { retry_delay = p_retry_delay; }

	float get_pending_delay() const { return pending_delay; }
	void set_pending_delay(float p_pending_delay) { pending_delay = p_pending_delay; }

	int get_session_max() const { return session_max; }
	void set_session_max(int p_session_max);

	float get_delay_until() const { return delay_until; }
	void set_delay_until(float p_delay_until) { delay_until = p_delay_until; }

	PackedStringArray get_categories() const { return categories; }
	void set_categories(const PackedStringArray &p_categories) { categories = p_categories; }

	virtual String get_name() const { return display_name; }

	CCEffect();
};

VARIANT_ENUM_CAST(CCEffect::EffectResult);
VARIANT_ENUM_CAST(CCEffect::Morality);

class CCEffectEntries : public Resource {
	GDCLASS(CCEffectEntries, Resource)

protected:
	static void _bind_methods();

private:
	Array effects = Array();

public:
	Array get_effects() const { return effects; }
	void set_effects(const Array &p_effects) { effects = p_effects; };

	String get_manifest() const;
};

#endif // CROWD_CONTROL_CC_EFFECT_HPP