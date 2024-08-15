#include "cc_effect_parameters.hpp"

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