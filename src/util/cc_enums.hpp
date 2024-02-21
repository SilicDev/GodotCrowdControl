#ifndef CROWD_CONTROL_CC_ENUMS_HPP
#define CROWD_CONTROL_CC_ENUMS_HPP

#include <godot_cpp/core/class_db.hpp>

enum BroadcasterType {
	COMMUNITY,
	AFFILIATE,
	PARTNER,
};

enum Platform {
	TWITCH,
	YOUTUBE,
	DISCORD,
};

enum TargetServer {
	PRODUCTION,
	STAGING,
	DEV,
};

enum EffectInstanceStatus {
	SUCCESS,
	FAIL_TEMPORARILY,
	FAIL_PERMANENTLY,
	TIMED_BEGIN,
	TIMED_PAUSE,
	TIMED_RESUME,
	TIMED_END,
};

//VARIANT_ENUM_CAST(BroadcasterType)
VARIANT_ENUM_CAST(Platform)
VARIANT_ENUM_CAST(TargetServer)

#endif // CROWD_CONTROL_CC_ENUMS_HPP