#ifndef CROWD_CONTROL_CC_EFFECT_PARAMETERS_HPP
#define CROWD_CONTROL_CC_EFFECT_PARAMETERS_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/cc_effect/cc_effect_bid_war.hpp"
#include "classes/cc_effect_instance.hpp"
#include "classes/stream_user.hpp"
#include "util/binding_macros.hpp"

using namespace godot;

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

#endif // CROWD_CONTROL_CC_EFFECT_PARAMETERS_HPP