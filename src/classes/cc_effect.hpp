#ifndef CROWD_CONTROL_CC_EFFECT_HPP
#define CROWD_CONTROL_CC_EFFECT_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "classes/stream_user.hpp"
#include "util/binding_macros.hpp"
#include "util/virtuals.hpp"

using namespace godot;

class CCEffectInstance;
class CCEffectInstanceTimed;
class CCEffectInstanceParameters;
class CrowdControl;

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

	/* VIRTUALS */
	/*
	 * DO NO OVERRIDE IN C++
	 * I cannot stress this enough, Godot virtual functions are currently still extremely broken.
	 * Calling them via -> operator will always execute the behaviour defined in C++, so the
	 * Object::call function has to be used. Additionally if a return value is expected, if
	 * GDScript does not overwrite them they return an empty Variant.
	 * See https://github.com/godotengine/godot-cpp/issues/1199
	 */

	virtual bool _can_run() const { return true; }
	virtual EffectResult _trigger(Ref<CCEffectInstance> effect);

	/* VIRTUAL CALLERS */
	/*
	 * Use these functions
	 */

	bool can_run();
	virtual EffectResult trigger(Ref<CCEffectInstance> effect);

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

	virtual EffectResult _start(Ref<CCEffectInstanceTimed> effect);
	virtual bool _stop(Ref<CCEffectInstanceTimed> effect, bool force = false);
	virtual void _pause() {}
	virtual void _resume() {}
	virtual void _reset() {}
	virtual bool _should_be_running() const { return true; }

	virtual EffectResult trigger(Ref<CCEffectInstance> effect) override;
	EffectResult start(Ref<CCEffectInstanceTimed> effect);
	bool stop(Ref<CCEffectInstanceTimed> effect, bool force = false);
	void pause(Ref<CCEffectInstanceTimed> effect);
	void resume(Ref<CCEffectInstanceTimed> effect);
	void reset(Ref<CCEffectInstanceTimed> effect);
	bool should_be_running();
};

class CCEffectBidWar : public CCEffect {
	GDCLASS(CCEffectBidWar, CCEffect)

public:
	struct BidWarEntry {
		String name;
		Ref<Texture2D> icon;
		Color tint = Color(1, 1, 1);
		String id;
	};

	struct BidWarLibrary {
		HashMap<String, uint32_t> bids;
		String winner_id;
		uint32_t highest_bid;

		bool place_bid(const String &p_id, uint32_t p_amount);
	};

private:
	BidWarLibrary library;
	BidWarEntry winner;
	HashMap<String, BidWarEntry> entries_by_id;
	Vector<BidWarEntry> entries;

	void build_entry_map();

protected:
	static void _bind_methods();

public:
	virtual EffectType get_effect_type() const override { return BID_WAR; }
	virtual EffectResult trigger(Ref<CCEffectInstance> effect) override;

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	BidWarEntry get_winner() const { return winner; };
	void set_winner(const BidWarEntry &p_winner) { winner = p_winner; };

	Dictionary get_winner_dict() const;
	void set_winner_dict(const Dictionary &p_winner);

	uint32_t get_bid_amount(const String &p_id);

	Vector<BidWarEntry> get_entries() const { return entries; }
	void set_entries(const Vector<BidWarEntry> &p_entries) { entries = p_entries; }

	HashMap<String, BidWarEntry> get_entries_by_id() const { return entries_by_id; }
	void set_entries_by_id(const HashMap<String, BidWarEntry> &p_entries_by_id) { entries_by_id = p_entries_by_id; }

	void set_entry_count(int p_count);
	int get_entry_count() const;

	bool place_bid(const String &p_id, uint32_t p_amount);

	void add_entry(const String &p_name);
	void remove_entry(int p_idx);
	void clear();

	void set_entry_name(int p_idx, const String &p_name);
	void set_entry_icon(int p_idx, const Ref<Texture2D> &p_icon);
	void set_entry_tint(int p_idx, const Color &p_tint);
	void set_entry_id(int p_idx, const String &p_id);

	String get_entry_name(int p_idx) const;
	int get_entry_idx(const String &p_id) const;
	Ref<Texture2D> get_entry_icon(int p_idx) const;
	Color get_entry_tint(int p_idx) const;
	String get_entry_id(int p_idx) const;

	String get_name() const override;
};

class CCEffectParameters : public CCEffect {
	GDCLASS(CCEffectParameters, CCEffect)

	friend class CCEffectInstanceParameters;

public:
	enum ParameterEntryKind {
		ITEM,
		RANGE,
	};

	struct ParameterOption {
		String id;
		String name;
		String parent_id;
	};

	struct ParameterEntry : CCEffectBidWar::BidWarEntry {
		ParameterEntryKind kind;
		Vector<ParameterOption> options;
		uint32_t min;
		uint32_t max;

		void create_options(const String &p_id, const PackedStringArray &p_names);
		PackedStringArray get_option_names() const;
	};

private:
	HashMap<String, ParameterEntry> parameter_entries_by_id;
	Vector<ParameterEntry> parameter_entries;

	void build_parameter_map();

protected:
	static void _bind_methods();

public:
	virtual EffectType get_effect_type() const override { return PARAMETERS; }

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	HashMap<String, ParameterEntry> get_parameter_entries_by_id() const { return parameter_entries_by_id; }

	void set_parameter_count(int p_count);
	int get_parameter_count() const;

	void add_item_parameter(const String &p_name, const PackedStringArray &p_options);
	void add_range_parameter(const String &p_name, uint32_t p_min, uint32_t p_max);
	void remove_parameter(int p_idx);
	void clear();

	void set_parameter_name(int p_idx, const String &p_name);
	void set_parameter_icon(int p_idx, const Ref<Texture2D> &p_icon);
	void set_parameter_tint(int p_idx, const Color &p_tint);
	void set_parameter_id(int p_idx, const String &p_id);
	void set_parameter_kind(int p_idx, ParameterEntryKind p_kind);

	String get_parameter_name(int p_idx) const;
	int get_parameter_idx(const String &p_id) const;
	Ref<Texture2D> get_parameter_icon(int p_idx) const;
	Color get_parameter_tint(int p_idx) const;
	String get_parameter_id(int p_idx) const;
	ParameterEntryKind get_parameter_kind(int p_idx) const;

	void set_item_parameter_options(int p_idx, const PackedStringArray &p_options);
	PackedStringArray get_item_parameter_options(int p_idx) const;

	void set_range_parameter_min(int p_idx, uint32_t p_min);
	void set_range_parameter_max(int p_idx, uint32_t p_max);

	uint32_t get_range_parameter_min(int p_idx) const;
	uint32_t get_range_parameter_max(int p_idx) const;
};

VARIANT_ENUM_CAST(CCEffectParameters::ParameterEntryKind);

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