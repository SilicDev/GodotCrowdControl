#ifndef CROWD_CONTROL_USER_INFO_HPP
#define CROWD_CONTROL_USER_INFO_HPP

#include "util/json_serializer.hpp"

using namespace godot;

struct UserOriginUserData {
	String offline_image_url;
	String description;
	String created_at;
	String profile_image_url;
	String id;
	String login;
	String display_name;
	String type;
	uint32_t view_count;
	String email;

	// for reasons unknown to me this throws an error in Visual Studio and Visual Studio Code
	// However the compiler is perfectly fine with this
	// There's nothing special about this class so I don't know what causes this
	constexpr static auto json_properties = std::make_tuple(
			json_property(&UserOriginUserData::offline_image_url, "offline_image_url"),
			json_property(&UserOriginUserData::description, "description"),
			json_property(&UserOriginUserData::created_at, "created_at"),
			json_property(&UserOriginUserData::profile_image_url, "profile_image_url"),
			json_property(&UserOriginUserData::id, "id"),
			json_property(&UserOriginUserData::login, "login"),
			json_property(&UserOriginUserData::display_name, "display_name"),
			json_property(&UserOriginUserData::type, "type"),
			json_property(&UserOriginUserData::view_count, "view_count"),
			json_property(&UserOriginUserData::email, "email"));
};

struct UserOriginData {
	String type;
	UserOriginUserData user;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&UserOriginData::type, "_type"),
			json_property(&UserOriginData::user, "user"));
};

struct UserInfoProfile {
	String created_at;
	String name;
	String origin_id;
	String image_url;
	String cc_uid;
	UserOriginData origin_data;
	String type;
	PackedStringArray roles;
	PackedStringArray subscriptions;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&UserInfoProfile::created_at, "createdAt"),
			json_property(&UserInfoProfile::name, "name"),
			json_property(&UserInfoProfile::origin_id, "originID"),
			json_property(&UserInfoProfile::image_url, "image"),
			json_property(&UserInfoProfile::cc_uid, "ccUID"),
			json_property(&UserInfoProfile::origin_data, "originData"),
			json_property(&UserInfoProfile::type, "type"),
			json_property(&UserInfoProfile::roles, "roles"),
			json_property(&UserInfoProfile::subscriptions, "subscriptions"));
};

struct UserInfo {
	UserInfoProfile profile;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&UserInfo::profile, "profile"));
};

#endif // CROWD_CONTROL_USER_INFO_HPP