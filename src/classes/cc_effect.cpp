#include "cc_effect.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "classes/cc_effect_instance.hpp"
#include "classes/crowd_control.hpp"
#include "json/effect_override.hpp"
#include "json/effect_request.hpp"
#include "json/manifest.hpp"
#include "util/binding_macros.hpp"

using namespace godot;

const char *CCEffect::effect_menu_status_names[] = {
	"menuUnavailable",
	"menuAvailable",
	"menuVisible",
	"menuHidden",
};

void CCEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pooling_disabled", "disabled"), &CCEffect::set_pooling_disabled);
	ClassDB::bind_method(D_METHOD("is_pooling_disabled"), &CCEffect::is_pooling_disabled);

	ClassDB::bind_method(D_METHOD("set_sellable", "enable"), &CCEffect::set_sellable);
	ClassDB::bind_method(D_METHOD("is_sellable"), &CCEffect::is_sellable);

	ClassDB::bind_method(D_METHOD("set_visible", "enable"), &CCEffect::set_visible);
	ClassDB::bind_method(D_METHOD("is_visible"), &CCEffect::is_visible);

	ClassDB::bind_method(D_METHOD("set_morality", "morality"), &CCEffect::set_morality);
	ClassDB::bind_method(D_METHOD("get_morality"), &CCEffect::get_morality);

	ClassDB::bind_method(D_METHOD("set_icon", "texture"), &CCEffect::set_icon);
	ClassDB::bind_method(D_METHOD("get_icon"), &CCEffect::get_icon);

	ClassDB::bind_method(D_METHOD("set_icon_color", "color"), &CCEffect::set_icon_color);
	ClassDB::bind_method(D_METHOD("get_icon_color"), &CCEffect::get_icon_color);

	ClassDB::bind_method(D_METHOD("set_id", "id"), &CCEffect::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &CCEffect::get_id);

	ClassDB::bind_method(D_METHOD("set_display_name", "name"), &CCEffect::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &CCEffect::get_display_name);

	ClassDB::bind_method(D_METHOD("set_description", "text"), &CCEffect::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &CCEffect::get_description);

	ClassDB::bind_method(D_METHOD("set_price", "value"), &CCEffect::set_price);
	ClassDB::bind_method(D_METHOD("get_price"), &CCEffect::get_price);

	ClassDB::bind_method(D_METHOD("set_max_retries", "value"), &CCEffect::set_max_retries);
	ClassDB::bind_method(D_METHOD("get_max_retries"), &CCEffect::get_max_retries);

	ClassDB::bind_method(D_METHOD("set_retry_delay", "delay"), &CCEffect::set_retry_delay);
	ClassDB::bind_method(D_METHOD("get_retry_delay"), &CCEffect::get_retry_delay);

	ClassDB::bind_method(D_METHOD("set_pending_delay", "delay"), &CCEffect::set_pending_delay);
	ClassDB::bind_method(D_METHOD("get_pending_delay"), &CCEffect::get_pending_delay);

	ClassDB::bind_method(D_METHOD("set_session_max", "max"), &CCEffect::set_session_max);
	ClassDB::bind_method(D_METHOD("get_session_max"), &CCEffect::get_session_max);

	ClassDB::bind_method(D_METHOD("set_categories", "categories"), &CCEffect::set_categories);
	ClassDB::bind_method(D_METHOD("get_categories"), &CCEffect::get_categories);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pooling_disabled"), "set_pooling_disabled", "is_pooling_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sellable"), "set_sellable", "is_sellable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "morality", PROPERTY_HINT_ENUM, "Neutral,Good,Evil"), "set_morality", "get_morality");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "icon_color"), "set_icon_color", "get_icon_color");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "price", PROPERTY_HINT_RANGE, "1,100000,1"), "set_price", "get_price");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_retries", PROPERTY_HINT_RANGE, "0,60,1"), "set_max_retries", "get_max_retries");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "retry_delay", PROPERTY_HINT_RANGE, "0,10,0.001f"), "set_retry_delay", "get_retry_delay");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pending_delay", PROPERTY_HINT_RANGE, "0,10,0.001f"), "set_pending_delay", "get_pending_delay");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "session_max"), "set_session_max", "get_session_max");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "categories"), "set_categories", "get_categories");

	BIND_VIRTUAL_METHOD(CCEffect, _can_run);
	ClassDB::bind_method(D_METHOD("can_run"), &CCEffect::can_run);
	BIND_VIRTUAL_METHOD(CCEffect, _trigger);
	ClassDB::bind_method(D_METHOD("trigger", "effect"), &CCEffect::trigger);

	BIND_ENUM_CONSTANT(SUCCESS);
	BIND_ENUM_CONSTANT(FAILURE);
	BIND_ENUM_CONSTANT(UNAVAILABLE);
	BIND_ENUM_CONSTANT(RETRY);
	BIND_ENUM_CONSTANT(QUEUE);
	BIND_ENUM_CONSTANT(RUNNING);

	BIND_ENUM_CONSTANT(NEUTRAL);
	BIND_ENUM_CONSTANT(GOOD);
	BIND_ENUM_CONSTANT(EVIL);
}

CCEffect::EffectResult CCEffect::_trigger(Ref<CCEffectInstance> effect) {
	return SUCCESS;
}

bool CCEffect::can_run() {
	Variant res = VIRTUAL_CALL(_can_run);
	if (res == Variant()) {
		return _can_run();
	}
	return res;
}

CCEffect::EffectResult CCEffect::trigger(Ref<CCEffectInstance> effect) {
	Variant res = VIRTUAL_CALL_ARGS(_trigger, effect);
	if (res == Variant()) {
		CrowdControl::get_singleton()->log_error(vformat("Effect '%s' does not override `_trigger()`!", id));
		return _trigger(effect);
	}
	return (EffectResult)(int)res;
};

void CCEffect::set_pooling_disabled(bool p_pooling_disabled) {
	pooling_disabled = p_pooling_disabled;
	CrowdControl::get_singleton()->post_effect_update(JSONSerializer::serialize_string<EffectChangeNonPoolable>(EffectChangeNonPoolable(id, pooling_disabled)));
}

void CCEffect::set_sellable(bool p_sellable) {
	sellable = p_sellable;
	if (sellable) {
		CrowdControl::get_singleton()->update_effect_status(this, effect_menu_status_names[MENU_AVAILABLE]);
	} else {
		CrowdControl::get_singleton()->update_effect_status(this, effect_menu_status_names[MENU_UNAVAILABLE]);
	}
}

void CCEffect::set_visible(bool p_visible) {
	visible = p_visible;
	if (visible) {
		CrowdControl::get_singleton()->update_effect_status(this, effect_menu_status_names[MENU_VISIBLE]);
	} else {
		CrowdControl::get_singleton()->update_effect_status(this, effect_menu_status_names[MENU_HIDDEN]);
	}
}

void CCEffect::set_price(int p_price) {
	if (p_price < 1) {
		CrowdControl::get_singleton()->log_error(vformat("Price for effect '%s' must be greater or equal 1!", get_id()));
		return;
	}
	price = p_price;
	CrowdControl::get_singleton()->post_effect_update(JSONSerializer::serialize_string<EffectChangePrice>(EffectChangePrice(id, price)));
}

void CCEffect::set_session_max(int p_session_max) {
	session_max = p_session_max;
	CrowdControl::get_singleton()->post_effect_update(JSONSerializer::serialize_string<EffectChangeSessionMax>(EffectChangeSessionMax(id, p_session_max)));
}

CCEffect::CCEffect() {
	//CrowdControl::get_singleton()->register_effect(Ref<CCEffect>(this);
}

void CCEffectTimed::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_duration", "secs"), &CCEffectTimed::set_duration);
	ClassDB::bind_method(D_METHOD("get_duration"), &CCEffectTimed::get_duration);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");

	BIND_VIRTUAL_METHOD(CCEffectTimed, _start);
	ClassDB::bind_method(D_METHOD("start", "effect"), &CCEffectTimed::start);
	BIND_VIRTUAL_METHOD(CCEffectTimed, _stop);
	ClassDB::bind_method(D_METHOD("stop", "effect", "force"), &CCEffectTimed::stop, DEFVAL(false));
	BIND_VIRTUAL_METHOD(CCEffectTimed, _pause);
	ClassDB::bind_method(D_METHOD("pause", "effect"), &CCEffectTimed::pause);
	BIND_VIRTUAL_METHOD(CCEffectTimed, _resume);
	ClassDB::bind_method(D_METHOD("resume", "effect"), &CCEffectTimed::resume);
	BIND_VIRTUAL_METHOD(CCEffectTimed, _reset);
	ClassDB::bind_method(D_METHOD("reset", "effect"), &CCEffectTimed::reset);
	BIND_VIRTUAL_METHOD(CCEffectTimed, _should_be_running);
	ClassDB::bind_method(D_METHOD("should_be_running"), &CCEffectTimed::should_be_running);
}

CCEffect::EffectResult CCEffectTimed::_start(Ref<CCEffectInstanceTimed> effect) {
	return SUCCESS;
}

bool CCEffectTimed::_stop(Ref<CCEffectInstanceTimed> effect, bool force) {
	return true;
}

CCEffect::EffectResult CCEffectTimed::trigger(Ref<CCEffectInstance> effect) {
	paused = false;
	Ref<CCEffectInstanceTimed> timed = Object::cast_to<CCEffectInstanceTimed>(effect.ptr());
	timed->set_end_time(effect->get_start_time() + duration);
	timed->set_time_left(duration);
	Variant res = start(timed);
	if ((int)res != 0) {
		return (EffectResult)(int)res;
	}
	return RUNNING;
}

CCEffect::EffectResult CCEffectTimed::start(Ref<CCEffectInstanceTimed> effect) {
	Variant res = VIRTUAL_CALL_ARGS(_start, effect);
	if (res == Variant()) {
		return _start(effect);
	}
	return (EffectResult)(int)res;
}

bool CCEffectTimed::stop(Ref<CCEffectInstanceTimed> effect, bool force) {
	Variant res = VIRTUAL_CALL_ARGS(_stop, effect, force);
	if (res == Variant()) {
		return _stop(effect);
	}
	if (!res && force) {
		CrowdControl::get_singleton()->log_error(vformat("Effect '%s' wasn't stopped, but 'force==true'!", get_id()));
	}
	return res;
}

void CCEffectTimed::pause(Ref<CCEffectInstanceTimed> effect) {
	paused = true;
	VIRTUAL_CALL(_pause);
}

void CCEffectTimed::resume(Ref<CCEffectInstanceTimed> effect) {
	paused = false;
	effect->set_time_left(effect->get_time_left() + (1.0 / Engine::get_singleton()->get_frames_per_second()));
	VIRTUAL_CALL(_resume);
}

void CCEffectTimed::reset(Ref<CCEffectInstanceTimed> effect) {
	paused = false;
	effect->set_time_left(duration);
	VIRTUAL_CALL(_reset);
}

bool CCEffectTimed::should_be_running() {
	if (paused) {
		return false;
	}
	Variant res = VIRTUAL_CALL(_should_be_running);
	if (res == Variant()) {
		return _should_be_running();
	}
	return res;
}

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

bool CCEffectParameters::_set(const StringName &p_name, const Variant &p_value) {
	PackedStringArray components = String(p_name).split("/", true, 2);
	if (components.size() >= 2 && components[0].begins_with("parameter_") && components[0].trim_prefix("parameter_").is_valid_int()) {
		uint32_t index = components[0].trim_prefix("parameter_").to_int();
		String property = components[1];
		bool edited = false;
		if (property == "name") {
			parameter_entries.write[index].name = p_value;
			edited = true;
		} else if (property == "icon") {
			parameter_entries.write[index].icon = p_value;
			edited = true;
		} else if (property == "tint") {
			parameter_entries.write[index].tint = p_value;
			edited = true;
		} else if (property == "id") {
			parameter_entries.write[index].id = p_value;
			edited = true;
		} else if (property == "kind") {
			parameter_entries.write[index].kind = (ParameterEntryKind)(int)p_value;
			edited = true;
			notify_property_list_changed();
		} else if (parameter_entries[index].kind == ParameterEntryKind::ITEM) {
			if (property == "options") {
				parameter_entries.write[index].create_options(parameter_entries[index].id, p_value);
				edited = true;
			}
		} else if (parameter_entries[index].kind == ParameterEntryKind::RANGE) {
			if (property == "min") {
				parameter_entries.write[index].min = p_value;
				edited = true;
			} else if (property == "max") {
				parameter_entries.write[index].max = p_value;
				edited = true;
			}
		}
		build_parameter_map();
		return edited;
	}
	return false;
}

bool CCEffectParameters::_get(const StringName &p_name, Variant &r_ret) const {
	PackedStringArray components = String(p_name).split("/", true, 2);
	if (components.size() >= 2 && components[0].begins_with("parameter_") && components[0].trim_prefix("parameter_").is_valid_int()) {
		uint32_t index = components[0].trim_prefix("parameter_").to_int();
		String property = components[1];
		if (property == "name") {
			r_ret = parameter_entries[index].name;
			return true;
		} else if (property == "icon") {
			r_ret = parameter_entries[index].icon;
			return true;
		} else if (property == "tint") {
			r_ret = parameter_entries[index].tint;
			return true;
		} else if (property == "id") {
			r_ret = parameter_entries[index].id;
			return true;
		} else if (property == "kind") {
			r_ret = parameter_entries[index].kind;
			return true;
		} else if (parameter_entries[index].kind == ParameterEntryKind::ITEM) {
			if (property == "options") {
				r_ret = parameter_entries[index].get_option_names();
				return true;
			}
		} else if (parameter_entries[index].kind == ParameterEntryKind::RANGE) {
			if (property == "min") {
				r_ret = parameter_entries[index].min;
				return true;
			} else if (property == "max") {
				r_ret = parameter_entries[index].max;
				return true;
			}
		}
	}
	return false;
}

void CCEffectParameters::_get_property_list(List<PropertyInfo> *p_list) const {
	// Godot allows us to create "fake" properties that are not bound to a Godot Object instance by
	// providing their property info here and handling them in _get and _set
	for (int i = 0; i < parameter_entries.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, vformat("parameter_%d/name", i)));
		p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("parameter_%d/icon", i), PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"));
		p_list->push_back(PropertyInfo(Variant::COLOR, vformat("parameter_%d/tint", i)));
		p_list->push_back(PropertyInfo(Variant::STRING, vformat("parameter_%d/id", i)));
		p_list->push_back(PropertyInfo(Variant::INT, vformat("parameter_%d/kind", i), PROPERTY_HINT_ENUM, "Item,Range"));
		if (parameter_entries[i].kind == ParameterEntryKind::ITEM) {
			p_list->push_back(PropertyInfo(Variant::PACKED_STRING_ARRAY, vformat("parameter_%d/options", i)));
		} else if (parameter_entries[i].kind == ParameterEntryKind::RANGE) {
			p_list->push_back(PropertyInfo(Variant::INT, vformat("parameter_%d/min", i)));
			p_list->push_back(PropertyInfo(Variant::INT, vformat("parameter_%d/max", i)));
		}
	}
}

void CCEffectParameters::set_parameter_count(int p_count) {
	ERR_FAIL_COND(p_count < 0);
	int prev_size = get_parameter_count();

	if (prev_size == p_count) {
		return;
	}

	parameter_entries.resize(p_count);

	if (p_count > prev_size) {
		for (int i = prev_size; i < p_count; i++) {
			parameter_entries.write[i] = ParameterEntry();
		}
	}
	build_parameter_map();
	notify_property_list_changed();
}

int CCEffectParameters::get_parameter_count() const {
	return parameter_entries.size();
}

void CCEffectParameters::add_item_parameter(const String &p_name, const PackedStringArray &p_options) {
	String id = vformat("%s_%s", get_id(), p_name.to_lower());
	Vector<ParameterOption> options;
	for (int i = 0; i < p_options.size(); i++) {
		ParameterOption option;
		option.id = id + p_options[i].to_lower();
		option.name = p_options[i];
		option.parent_id = id;
		options.append(option);
	}
	ParameterEntry entry = ParameterEntry();
	entry.id = id;
	entry.name = p_name;
	entry.options = options;
	parameter_entries.append(entry);
	build_parameter_map();
	notify_property_list_changed();
}

void CCEffectParameters::add_range_parameter(const String &p_name, uint32_t p_min, uint32_t p_max) {
	String id = p_name.to_lower();
	// quantity is a special parameter without prefix
	if (id != "quantity") {
		id = vformat("%s_%s", get_id(), p_name.to_lower());
	}
	ERR_FAIL_COND_EDMSG(p_min >= p_max, vformat("Upper bound of '%s' must be greater than lower bound!", p_name));
	ERR_FAIL_COND_EDMSG(p_min < 1, vformat("Lower bound of '%s' cannot be less than 1!", p_name));
	ParameterEntry entry = ParameterEntry();
	entry.id = id;
	entry.name = p_name;
	entry.min = p_min;
	entry.max = p_max;
	parameter_entries.append(entry);
	build_parameter_map();
	notify_property_list_changed();
}

void CCEffectParameters::remove_parameter(int p_idx) {
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	parameter_entries.remove_at(p_idx);
	build_parameter_map();
}

void CCEffectParameters::build_parameter_map() {
	parameter_entries_by_id.clear();
	for (int i = 0; i < parameter_entries.size(); i++) {
		parameter_entries_by_id.insert(parameter_entries[i].id, parameter_entries[i]);
	}
}

void CCEffectParameters::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_item_parameter", "name", "options"), &CCEffectParameters::add_item_parameter);
	ClassDB::bind_method(D_METHOD("add_range_parameter", "name", "min", "max"), &CCEffectParameters::add_range_parameter);
	ClassDB::bind_method(D_METHOD("remove_parameter", "idx"), &CCEffectParameters::remove_parameter);
	ClassDB::bind_method(D_METHOD("clear"), &CCEffectParameters::clear);

	ClassDB::bind_method(D_METHOD("set_parameter_name", "idx", "name"), &CCEffectParameters::set_parameter_name);
	ClassDB::bind_method(D_METHOD("get_parameter_name", "idx"), &CCEffectParameters::get_parameter_name);

	ClassDB::bind_method(D_METHOD("get_parameter_idx", "id"), &CCEffectParameters::get_parameter_idx);

	ClassDB::bind_method(D_METHOD("set_parameter_icon", "idx", "icon"), &CCEffectParameters::set_parameter_icon);
	ClassDB::bind_method(D_METHOD("get_parameter_icon", "idx"), &CCEffectParameters::get_parameter_icon);

	ClassDB::bind_method(D_METHOD("set_parameter_tint", "idx", "tint"), &CCEffectParameters::set_parameter_tint);
	ClassDB::bind_method(D_METHOD("get_parameter_tint", "idx"), &CCEffectParameters::get_parameter_tint);

	ClassDB::bind_method(D_METHOD("set_parameter_id", "idx", "id"), &CCEffectParameters::set_parameter_id);
	ClassDB::bind_method(D_METHOD("get_parameter_id", "idx"), &CCEffectParameters::get_parameter_id);

	ClassDB::bind_method(D_METHOD("set_parameter_kind", "idx", "kind"), &CCEffectParameters::set_parameter_kind);
	ClassDB::bind_method(D_METHOD("get_parameter_kind", "idx"), &CCEffectParameters::get_parameter_kind);

	ClassDB::bind_method(D_METHOD("set_item_parameter_options", "idx", "options"), &CCEffectParameters::set_item_parameter_options);
	ClassDB::bind_method(D_METHOD("get_item_parameter_options", "idx"), &CCEffectParameters::get_item_parameter_options);

	ClassDB::bind_method(D_METHOD("set_range_parameter_min", "idx", "min"), &CCEffectParameters::set_range_parameter_min);
	ClassDB::bind_method(D_METHOD("get_range_parameter_min", "idx"), &CCEffectParameters::get_range_parameter_min);

	ClassDB::bind_method(D_METHOD("set_range_parameter_max", "idx", "max"), &CCEffectParameters::set_range_parameter_max);
	ClassDB::bind_method(D_METHOD("get_range_parameter_max", "idx"), &CCEffectParameters::get_range_parameter_max);

	ClassDB::bind_method(D_METHOD("set_parameter_count", "count"), &CCEffectParameters::set_parameter_count);
	ClassDB::bind_method(D_METHOD("get_parameter_count"), &CCEffectParameters::get_parameter_count);

	BIND_ENUM_CONSTANT(ITEM);
	BIND_ENUM_CONSTANT(RANGE);

	ADD_ARRAY_COUNT("Parameters", "parameter_count", "set_parameter_count", "get_parameter_count", "parameter_");
}

void CCEffectParameters::set_parameter_name(int p_idx, const String &p_name) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	if (parameter_entries[p_idx].name == p_name) {
		return;
	}
	parameter_entries.write[p_idx].name = p_name;
	build_parameter_map();
}

void CCEffectParameters::clear() {
	parameter_entries.clear();
	build_parameter_map();
	notify_property_list_changed();
}

void CCEffectParameters::set_parameter_icon(int p_idx, const Ref<Texture2D> &p_icon) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	if (parameter_entries[p_idx].icon == p_icon) {
		return;
	}
	parameter_entries.write[p_idx].icon = p_icon;
	build_parameter_map();
}

void CCEffectParameters::set_parameter_tint(int p_idx, const Color &p_tint) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	if (parameter_entries[p_idx].tint == p_tint) {
		return;
	}
	parameter_entries.write[p_idx].tint = p_tint;
	build_parameter_map();
}

void CCEffectParameters::set_parameter_kind(int p_idx, ParameterEntryKind p_kind) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	if (parameter_entries[p_idx].kind == p_kind) {
		return;
	}
	parameter_entries.write[p_idx].kind = p_kind;
	build_parameter_map();
}

void CCEffectParameters::set_parameter_id(int p_idx, const String &p_id) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	if (parameter_entries[p_idx].id == p_id) {
		return;
	}
	parameter_entries.write[p_idx].id = p_id;
	build_parameter_map();
}

String CCEffectParameters::get_parameter_name(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), "");
	return parameter_entries[p_idx].name;
}

int CCEffectParameters::get_parameter_idx(const String &p_id) const {
	for (int i = 0; i < parameter_entries.size(); i++) {
		if (parameter_entries[i].id == p_id) {
			return i;
		}
	}
	return -1;
}

Ref<Texture2D> CCEffectParameters::get_parameter_icon(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), nullptr);
	return parameter_entries[p_idx].icon;
}

Color CCEffectParameters::get_parameter_tint(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), Color(1, 1, 1));
	return parameter_entries[p_idx].tint;
}

String CCEffectParameters::get_parameter_id(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), "");
	return parameter_entries[p_idx].id;
}

CCEffectParameters::ParameterEntryKind CCEffectParameters::get_parameter_kind(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), ParameterEntryKind::ITEM);
	return parameter_entries[p_idx].kind;
}

void CCEffectParameters::set_item_parameter_options(int p_idx, const PackedStringArray &p_options) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	ERR_FAIL_COND_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::RANGE, vformat("Parameter '%s' is a range parameter!"));
	if (parameter_entries[p_idx].get_option_names() == p_options) {
		return;
	}
	parameter_entries.write[p_idx].create_options(parameter_entries[p_idx].id, p_options);
	build_parameter_map();
}

PackedStringArray CCEffectParameters::get_item_parameter_options(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), PackedStringArray());
	ERR_FAIL_COND_V_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::RANGE, PackedStringArray(), vformat("Parameter '%s' is a range parameter!"));
	return parameter_entries[p_idx].get_option_names();
}

void CCEffectParameters::set_range_parameter_min(int p_idx, uint32_t p_min) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	ERR_FAIL_COND_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::ITEM, vformat("Parameter '%s' is an item parameter!"));
	if (parameter_entries[p_idx].min == p_min) {
		return;
	}
	parameter_entries.write[p_idx].min = p_min;
	build_parameter_map();
}

void CCEffectParameters::set_range_parameter_max(int p_idx, uint32_t p_max) {
	if (p_idx < 0) {
		p_idx += get_parameter_count();
	}
	ERR_FAIL_INDEX(p_idx, parameter_entries.size());
	ERR_FAIL_COND_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::ITEM, vformat("Parameter '%s' is an item parameter!"));
	if (parameter_entries[p_idx].max == p_max) {
		return;
	}
	parameter_entries.write[p_idx].max = p_max;
	build_parameter_map();
}

uint32_t CCEffectParameters::get_range_parameter_min(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), 0);
	ERR_FAIL_COND_V_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::ITEM, 0, vformat("Parameter '%s' is an item parameter!"));
	return parameter_entries[p_idx].min;
}

uint32_t CCEffectParameters::get_range_parameter_max(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, parameter_entries.size(), 0);
	ERR_FAIL_COND_V_EDMSG(parameter_entries[p_idx].kind == ParameterEntryKind::ITEM, 0, vformat("Parameter '%s' is an item parameter!"));
	return parameter_entries[p_idx].max;
}

void CCEffectParameters::ParameterEntry::create_options(const String &p_id, const PackedStringArray &p_names) {
	options.clear();
	for (int i = 0; i < p_names.size(); i++) {
		ParameterOption option;
		option.id = vformat("%s_%s", p_id, p_names[i].to_lower());
		option.name = p_names[i];
		option.parent_id = p_id;
		options.append(option);
	}
}

PackedStringArray CCEffectParameters::ParameterEntry::get_option_names() const {
	PackedStringArray out;
	for (int i = 0; i < options.size(); i++) {
		out.append(options[i].name);
	}
	return out;
}

void CCEffectEntries::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &CCEffectEntries::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &CCEffectEntries::get_effects);
	ClassDB::bind_method(D_METHOD("get_manifest"), &CCEffectEntries::get_manifest);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_TYPE_STRING, vformat("%d/%d:CCEffect", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE)), "set_effects", "get_effects");
}

String CCEffectEntries::get_manifest() const {
	HashMap<String, Ref<CCEffect>> effects_by_id = HashMap<String, Ref<CCEffect>>();
	for (int i = 0; i < effects.size(); i++) {
		Ref<CCEffect> effect = Object::cast_to<CCEffect>(effects[i]);
		ERR_CONTINUE_EDMSG(effect == nullptr, vformat("Invalid value '%s' in effect entries!", UtilityFunctions::str(effects[i])));
		effects_by_id.insert(effect->get_id(), effect);
	}
	return JSONSerializer::serialize_string(Manifest(GLOBAL_GET("crowd_control/common/game_name"), effects_by_id));
}
