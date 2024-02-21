#ifndef CROWD_CONTROL_USER_HPP
#define CROWD_CONTROL_USER_HPP

#include "util/json_serializer.hpp"

using namespace godot;

struct User {
	String cc_uid;
	String name;
	String profile;
	String origin_id;
	PackedStringArray roles;
	PackedStringArray subscriptions;
	String image_url;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&User::cc_uid, "ccUID"),
			json_property(&User::name, "name"),
			json_property(&User::profile, "profile"),
			json_property(&User::origin_id, "originID"),
			json_property(&User::roles, "roles"),
			json_property(&User::subscriptions, "subscriptions"),
			json_property(&User::image_url, "image"));
};

#endif // CROWD_CONTROL_USER_HPP