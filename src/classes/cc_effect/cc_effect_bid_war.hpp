#ifndef CROWD_CONTROL_CC_EFFECT_BID_WAR_HPP
#define CROWD_CONTROL_CC_EFFECT_BID_WAR_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/stream_user.hpp"
#include "util/binding_macros.hpp"

class CCEffectInstance;

using namespace godot;

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

#endif // CROWD_CONTROL_CC_EFFECT_BID_WAR_HPP