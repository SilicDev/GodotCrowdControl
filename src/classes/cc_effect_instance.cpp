#include "cc_effect_instance.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "classes/cc_effect/cc_effect.hpp"
#include "classes/cc_effect/cc_effect_bid_war.hpp"
#include "classes/cc_effect/cc_effect_parameters.hpp"
#include "classes/cc_effect/cc_effect_timed.hpp"
#include "classes/stream_user.hpp"

void CCEffectInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_test"), &CCEffectInstance::is_test);
	ClassDB::bind_method(D_METHOD("get_effect"), &CCEffectInstance::get_effect);
	ClassDB::bind_method(D_METHOD("get_user"), &CCEffectInstance::get_user);
	ClassDB::bind_method(D_METHOD("get_id"), &CCEffectInstance::get_id);
	ClassDB::bind_method(D_METHOD("get_retry_count"), &CCEffectInstance::get_retry_count);
	ClassDB::bind_method(D_METHOD("get_start_time"), &CCEffectInstance::get_start_time);

	ClassDB::bind_method(D_METHOD("get_effect_id"), &CCEffectInstance::get_effect_id);
}

StringName CCEffectInstance::get_effect_id() const {
	ERR_FAIL_COND_V_EDMSG(effect.is_null(), "", "No effect associated with this instance!");
	return effect->get_id();
}

void CCEffectInstanceTimed::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_active"), &CCEffectInstanceTimed::is_active);
	ClassDB::bind_method(D_METHOD("get_effect_timed"), &CCEffectInstanceTimed::get_effect_timed);
	ClassDB::bind_method(D_METHOD("get_end_time"), &CCEffectInstanceTimed::get_end_time);
	ClassDB::bind_method(D_METHOD("get_time_left"), &CCEffectInstanceTimed::get_time_left);
	ClassDB::bind_method(D_METHOD("is_paused"), &CCEffectInstanceTimed::is_paused);

	ClassDB::bind_method(D_METHOD("should_be_running"), &CCEffectInstanceTimed::should_be_running);
}

Ref<CCEffectTimed> CCEffectInstanceTimed::get_effect_timed() const {
	return Object::cast_to<CCEffectTimed>(get_effect().ptr());
}

void CCEffectInstanceTimed::set_effect_timed(const Ref<CCEffectTimed> &p_effect_timed) {
	set_effect(p_effect_timed);
}

bool CCEffectInstanceTimed::is_paused() const {
	ERR_FAIL_COND_V_EDMSG(get_effect_timed().is_null(), false, "No effect associated with this instance!");
	return get_effect_timed()->is_paused();
}

void CCEffectInstanceTimed::set_paused(bool paused) {
	ERR_FAIL_COND_EDMSG(get_effect_timed().is_null(), "No effect associated with this instance!");
	get_effect_timed()->set_paused(paused);
}

bool CCEffectInstanceTimed::should_be_running() const {
	ERR_FAIL_COND_V_EDMSG(get_effect_timed().is_null(), false, "No effect associated with this instance!");
	return get_effect_timed()->should_be_running();
}

uint32_t CCEffectInstanceBidWar::get_bid_amount() const {
	Ref<CCEffectBidWar> effect = Object::cast_to<CCEffectBidWar>(get_effect().ptr());
	return effect->get_bid_amount(bid_id);
}

String CCEffectInstanceBidWar::get_bid_name() const {
	ERR_FAIL_COND_V_EDMSG(get_effect().is_null(), "", "No effect associated with this instance!");
	Ref<CCEffectBidWar> effect = Object::cast_to<CCEffectBidWar>(get_effect().ptr());
	return effect->get_entries_by_id()[bid_id].name;
}

void CCEffectInstanceBidWar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_bid_id"), &CCEffectInstanceBidWar::get_bid_id);
	ClassDB::bind_method(D_METHOD("get_bid_amount"), &CCEffectInstanceBidWar::get_bid_amount);
	ClassDB::bind_method(D_METHOD("get_bid_name"), &CCEffectInstanceBidWar::get_bid_name);
}

Variant CCEffectInstanceParameters::get_parameter(const String &p_param) const {
	ERR_FAIL_COND_V_EDMSG(get_effect().is_null(), Variant(), "No effect associated with this instance!");
	if (parameters.has(p_param)) {
		if (parameters[p_param].type == "quantity") {
			return parameters[p_param].value.to_int();
		}
		return parameters[p_param].value;
	} else {
		Ref<CCEffectParameters> param_effect = Object::cast_to<CCEffectParameters>(get_effect().ptr());
		if (param_effect->parameter_entries_by_id.has(p_param)) {
			return parameters[p_param].value;
		}
		for (KeyValue<String, CCEffectParameters::ParameterEntry> kv : param_effect->parameter_entries_by_id) {
			if (kv.value.name == p_param) {
				return parameters[kv.value.name].value;
			}
		}
	}
	return Variant();
}

PackedStringArray CCEffectInstanceParameters::get_parameter_names() const {
	ERR_FAIL_COND_V_EDMSG(get_effect().is_null(), PackedStringArray(), "No effect associated with this instance!");
	PackedStringArray out;
	for (KeyValue<String, ParameterEntry> kv : parameters) {
		out.append(kv.key);
	}
	Ref<CCEffectParameters> param_effect = Object::cast_to<CCEffectParameters>(get_effect().ptr());
	for (KeyValue<String, CCEffectParameters::ParameterEntry> kv : param_effect->parameter_entries_by_id) {
		// Prevent duplicates
		if (!out.has(kv.key)) {
			out.append(kv.key);
		}
	}
	return out;
}

String CCEffectInstanceParameters::get_option_name(const String &p_param) const {
	ERR_FAIL_COND_V_EDMSG(get_effect().is_null(), "", "No effect associated with this instance!");
	if (parameters.has(p_param)) {
		if (parameters[p_param].type == "quantity") {
			return "";
		}
		return parameters[p_param].name;
	} else {
		Ref<CCEffectParameters> param_effect = Object::cast_to<CCEffectParameters>(get_effect().ptr());
		if (param_effect->parameter_entries_by_id.has(p_param)) {
			return parameters[p_param].name;
		}
		for (KeyValue<String, CCEffectParameters::ParameterEntry> kv : param_effect->parameter_entries_by_id) {
			if (kv.value.name == p_param) {
				return parameters[kv.value.name].name;
			}
		}
	}
	return "";
}

String CCEffectInstanceParameters::get_parameter_title(const String &p_param) const {
	ERR_FAIL_COND_V_EDMSG(get_effect().is_null(), "", "No effect associated with this instance!");
	if (parameters.has(p_param)) {
		if (parameters[p_param].type == "quantity") {
			return "Quantity";
		}
		return parameters[p_param].title;
	} else {
		Ref<CCEffectParameters> param_effect = Object::cast_to<CCEffectParameters>(get_effect().ptr());
		if (param_effect->parameter_entries_by_id.has(p_param)) {
			return parameters[p_param].title;
		}
		for (KeyValue<String, CCEffectParameters::ParameterEntry> kv : param_effect->parameter_entries_by_id) {
			if (kv.value.name == p_param) {
				return parameters[kv.value.name].title;
			}
		}
	}
	return "";
}

void CCEffectInstanceParameters::assign_parameters(const HashMap<String, ParameterEntry> &p_params) {
	parameters = p_params;
}

void CCEffectInstanceParameters::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_parameter", "id"), &CCEffectInstanceParameters::get_parameter);
	ClassDB::bind_method(D_METHOD("get_parameter_names"), &CCEffectInstanceParameters::get_parameter_names);
	ClassDB::bind_method(D_METHOD("get_option_name", "id"), &CCEffectInstanceParameters::get_option_name);
}