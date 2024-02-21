#include "stream_user.hpp"

Ref<StreamUser> StreamUser::make(String p_origin_site, String p_name, String p_display_name, String p_profile_icon_url, PackedStringArray p_roles, PackedStringArray p_subscriptions, String p_origin_id) {
	Ref<StreamUser> user;
	user.instantiate();
	user->origin_site = p_origin_site;
	user->name = p_name;
	user->display_name = p_display_name;
	user->profile_icon_url = p_profile_icon_url;
	user->roles = p_roles;
	user->subscriptions = p_subscriptions;
	user->origin_id = p_origin_id;
	return user;
}

Ref<StreamUser> StreamUser::make(String p_name, String p_profile_icon_url, PackedStringArray p_roles) {
	Ref<StreamUser> user;
	user.instantiate();
	user->name = p_name;
	user->profile_icon_url = p_profile_icon_url;
	return user;
}

Ref<StreamUser> StreamUser::make(String p_name, Ref<Texture2D> p_profile_icon) {
	Ref<StreamUser> user;
	user.instantiate();
	user->name = p_name;
	user->profile_icon = p_profile_icon;
	return user;
}

void StreamUser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_name", "login_name"), &StreamUser::set_name);
	ClassDB::bind_method(D_METHOD("get_name"), &StreamUser::get_name);

	ClassDB::bind_method(D_METHOD("set_profile_icon_url", "url"), &StreamUser::set_profile_icon_url);
	ClassDB::bind_method(D_METHOD("get_profile_icon_url"), &StreamUser::get_profile_icon_url);

	ClassDB::bind_method(D_METHOD("set_roles", "role_arr"), &StreamUser::set_roles);
	ClassDB::bind_method(D_METHOD("get_roles"), &StreamUser::get_roles);

	ClassDB::bind_method(D_METHOD("set_subscriptions", "subs"), &StreamUser::set_subscriptions);
	ClassDB::bind_method(D_METHOD("get_subscriptions"), &StreamUser::get_subscriptions);

	ClassDB::bind_method(D_METHOD("set_profile_icon", "icon"), &StreamUser::set_profile_icon);
	ClassDB::bind_method(D_METHOD("get_profile_icon"), &StreamUser::get_profile_icon);

	ClassDB::bind_method(D_METHOD("set_coins_spent", "coins"), &StreamUser::set_coins_spent);
	ClassDB::bind_method(D_METHOD("get_coins_spent"), &StreamUser::get_coins_spent);

	ClassDB::bind_method(D_METHOD("set_display_name", "user_name"), &StreamUser::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &StreamUser::get_display_name);

	ClassDB::bind_method(D_METHOD("set_origin_site", "site"), &StreamUser::set_origin_site);
	ClassDB::bind_method(D_METHOD("get_origin_site"), &StreamUser::get_origin_site);

	ClassDB::bind_method(D_METHOD("set_origin_id", "id"), &StreamUser::set_origin_id);
	ClassDB::bind_method(D_METHOD("get_origin_id"), &StreamUser::get_origin_id);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "profile_icon_url"), "set_profile_icon_url", "get_profile_icon_url");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "roles"), "set_roles", "get_roles");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "subscriptions"), "set_subscriptions", "get_subscriptions");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "profile_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_profile_icon", "get_profile_icon");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "coins_spent"), "set_coins_spent", "get_coins_spent");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "origin_site"), "set_origin_site", "get_origin_site");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "origin_id"), "set_origin_id", "get_origin_id");

	ADD_SIGNAL(MethodInfo("icon_downloaded"));
}