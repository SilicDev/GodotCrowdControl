#include "cc_effect_bid_war.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/cc_effect_instance.hpp"

void CCEffectBidWar::build_entry_map() {
	entries_by_id.clear();
	for (int i = 0; i < entries.size(); i++) {
		entries_by_id.insert(entries[i].id, entries[i]);
	}
}

void CCEffectBidWar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_winner"), &CCEffectBidWar::get_winner_dict);
	ClassDB::bind_method(D_METHOD("place_bid", "id", "amount"), &CCEffectBidWar::place_bid);

	ClassDB::bind_method(D_METHOD("add_entry", "name"), &CCEffectBidWar::add_entry);
	ClassDB::bind_method(D_METHOD("remove_entry", "idx"), &CCEffectBidWar::remove_entry);
	ClassDB::bind_method(D_METHOD("clear"), &CCEffectBidWar::clear);

	ClassDB::bind_method(D_METHOD("set_entry_name", "idx", "name"), &CCEffectBidWar::set_entry_name);
	ClassDB::bind_method(D_METHOD("get_entry_name", "idx"), &CCEffectBidWar::get_entry_name);

	ClassDB::bind_method(D_METHOD("get_entry_idx", "id"), &CCEffectBidWar::get_entry_idx);

	ClassDB::bind_method(D_METHOD("set_entry_icon", "idx", "icon"), &CCEffectBidWar::set_entry_icon);
	ClassDB::bind_method(D_METHOD("get_entry_icon", "idx"), &CCEffectBidWar::get_entry_icon);

	ClassDB::bind_method(D_METHOD("set_entry_tint", "idx", "tint"), &CCEffectBidWar::set_entry_tint);
	ClassDB::bind_method(D_METHOD("get_entry_tint", "idx"), &CCEffectBidWar::get_entry_tint);

	ClassDB::bind_method(D_METHOD("set_entry_id", "idx", "id"), &CCEffectBidWar::set_entry_id);
	ClassDB::bind_method(D_METHOD("get_entry_id", "idx"), &CCEffectBidWar::get_entry_id);

	ClassDB::bind_method(D_METHOD("set_entry_count", "count"), &CCEffectBidWar::set_entry_count);
	ClassDB::bind_method(D_METHOD("get_entry_count"), &CCEffectBidWar::get_entry_count);

	ADD_ARRAY_COUNT("Entries", "entry_count", "set_entry_count", "get_entry_count", "entry_");
}

CCEffect::EffectResult CCEffectBidWar::trigger(Ref<CCEffectInstance> effect) {
	Ref<CCEffectInstanceBidWar> bid_war = Object::cast_to<CCEffectInstanceBidWar>(effect.ptr());
	if (place_bid(bid_war->get_bid_id(), get_price())) {
		Variant res = CCEffect::trigger(bid_war);
		if ((int)res != 0) {
			return (EffectResult)(int)res;
		}
	}
	return SUCCESS;
}

bool CCEffectBidWar::_set(const StringName &p_name, const Variant &p_value) {
	PackedStringArray components = String(p_name).split("/", true, 2);
	if (components.size() >= 2 && components[0].begins_with("entry_") && components[0].trim_prefix("entry_").is_valid_int()) {
		uint32_t index = components[0].trim_prefix("entry_").to_int();
		String property = components[1];
		bool edited = false;
		if (property == "name") {
			set_entry_name(index, p_value);
			return true;
		} else if (property == "icon") {
			set_entry_icon(index, p_value);
			return true;
		} else if (property == "tint") {
			set_entry_tint(index, p_value);
			return true;
		} else if (property == "id") {
			set_entry_id(index, p_value);
			return true;
		}
	}
	return false;
}

bool CCEffectBidWar::_get(const StringName &p_name, Variant &r_ret) const {
	PackedStringArray components = String(p_name).split("/", true, 2);
	if (components.size() >= 2 && components[0].begins_with("entry_") && components[0].trim_prefix("entry_").is_valid_int()) {
		uint32_t index = components[0].trim_prefix("entry_").to_int();
		String property = components[1];
		if (property == "name") {
			r_ret = get_entry_name(index);
			return true;
		} else if (property == "icon") {
			r_ret = get_entry_icon(index);
			return true;
		} else if (property == "tint") {
			r_ret = get_entry_tint(index);
			return true;
		} else if (property == "id") {
			r_ret = get_entry_id(index);
			return true;
		}
	}
	return false;
}

void CCEffectBidWar::_get_property_list(List<PropertyInfo> *p_list) const {
	// Godot allows us to create "fake" properties that are not bound to a Godot Object instance by
	// providing their property info here and handling them in _get and _set
	for (int i = 0; i < entries.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, vformat("entry_%d/name", i)));
		p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("entry_%d/icon", i), PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"));
		p_list->push_back(PropertyInfo(Variant::COLOR, vformat("entry_%d/tint", i)));
		p_list->push_back(PropertyInfo(Variant::STRING, vformat("entry_%d/id", i)));
	}
}

Dictionary CCEffectBidWar::get_winner_dict() const {
	Dictionary out;
	out["id"] = winner.id;
	out["name"] = winner.name;
	out["icon"] = winner.icon;
	out["tint"] = winner.tint;
	return out;
}

void CCEffectBidWar::set_winner_dict(const Dictionary &p_winner) {
	winner.id = p_winner.get("id", "");
	winner.name = p_winner.get("name", "");
	winner.icon = p_winner.get("icon", nullptr);
	winner.tint = p_winner.get("tint", Color(1, 1, 1));
}

uint32_t CCEffectBidWar::get_bid_amount(const String &p_id) {
	if (library.bids.has(p_id)) {
		return library.bids[p_id];
	}
	return 0;
}

void CCEffectBidWar::set_entry_count(int p_count) {
	ERR_FAIL_COND(p_count < 0);
	int prev_size = get_entry_count();

	if (prev_size == p_count) {
		return;
	}

	entries.resize(p_count);
	build_entry_map();
	notify_property_list_changed();
}

int CCEffectBidWar::get_entry_count() const {
	return entries.size();
}

bool CCEffectBidWar::place_bid(const String &p_id, uint32_t p_amount) {
	bool flag = library.place_bid(p_id, p_amount);
	if (flag) {
		winner = entries_by_id[library.winner_id];
	}
	return flag;
}

void CCEffectBidWar::add_entry(const String &p_name) {
	BidWarEntry entry;
	entry.name = p_name;
	entry.id = vformat("%s_%s", get_id(), p_name.to_lower());
	entries.append(entry);

	build_entry_map();
	notify_property_list_changed();
}

void CCEffectBidWar::remove_entry(int p_idx) {
	ERR_FAIL_INDEX(p_idx, entries.size());
	entries.remove_at(p_idx);
	build_entry_map();
}

void CCEffectBidWar::clear() {
	entries.clear();
	build_entry_map();
	notify_property_list_changed();
}

void CCEffectBidWar::set_entry_name(int p_idx, const String &p_name) {
	if (p_idx < 0) {
		p_idx += get_entry_count();
	}
	ERR_FAIL_INDEX(p_idx, entries.size());
	if (entries[p_idx].name == p_name) {
		return;
	}
	entries.write[p_idx].name = p_name;
	build_entry_map();
}

void CCEffectBidWar::set_entry_icon(int p_idx, const Ref<Texture2D> &p_icon) {
	if (p_idx < 0) {
		p_idx += get_entry_count();
	}
	ERR_FAIL_INDEX(p_idx, entries.size());
	if (entries[p_idx].icon == p_icon) {
		return;
	}
	entries.write[p_idx].icon = p_icon;
	build_entry_map();
}

void CCEffectBidWar::set_entry_tint(int p_idx, const Color &p_tint) {
	if (p_idx < 0) {
		p_idx += get_entry_count();
	}
	ERR_FAIL_INDEX(p_idx, entries.size());
	if (entries[p_idx].tint == p_tint) {
		return;
	}
	entries.write[p_idx].tint = p_tint;
	build_entry_map();
}

void CCEffectBidWar::set_entry_id(int p_idx, const String &p_id) {
	if (p_idx < 0) {
		p_idx += get_entry_count();
	}
	ERR_FAIL_INDEX(p_idx, entries.size());
	if (entries[p_idx].id == p_id) {
		return;
	}
	entries.write[p_idx].id = p_id;
	build_entry_map();
}

String CCEffectBidWar::get_entry_name(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, entries.size(), "");
	return entries[p_idx].name;
}

int CCEffectBidWar::get_entry_idx(const String &p_id) const {
	for (int i = 0; i < entries.size(); i++) {
		if (entries[i].id == p_id) {
			return i;
		}
	}
	return -1;
}

Ref<Texture2D> CCEffectBidWar::get_entry_icon(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, entries.size(), nullptr);
	return entries[p_idx].icon;
}

Color CCEffectBidWar::get_entry_tint(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, entries.size(), Color(1, 1, 1));
	return entries[p_idx].tint;
}

String CCEffectBidWar::get_entry_id(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, entries.size(), "");
	return entries[p_idx].id;
}

String CCEffectBidWar::get_name() const {
	if (entries_by_id.has(winner.id)) {
		return vformat("%s: %s", get_display_name(), winner.name);
	}
	return get_display_name();
}

bool CCEffectBidWar::BidWarLibrary::place_bid(const String &p_id, uint32_t p_amount) {
	if (p_amount == 0) {
		return false;
	}

	if (!bids.has(p_id)) {
		bids.insert(p_id, p_amount);
	} else {
		bids[p_id] = p_amount;
	}

	if (p_amount <= highest_bid) {
		return false;
	}

	highest_bid = p_amount;

	if (p_id == winner_id) {
		return false;
	}

	winner_id = p_id;
	return true;
}