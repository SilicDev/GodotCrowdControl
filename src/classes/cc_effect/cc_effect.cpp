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

	GDVIRTUAL_BIND(_can_run)
	GDVIRTUAL_BIND(_trigger, "effect")

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

bool CCEffect::can_run() {
	bool res;
	if (GDVIRTUAL_CALL(_can_run, res)) {
		return res;
	}
	return true;
}

CCEffect::EffectResult CCEffect::trigger(Ref<CCEffectInstance> effect) {
	int res;
	GDVIRTUAL_REQUIRED_CALL(_trigger, effect, res);
	return (EffectResult)res;
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
