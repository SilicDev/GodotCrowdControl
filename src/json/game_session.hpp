#ifndef CROWD_CONTROL_GAME_SESSION_HPP
#define CROWD_CONTROL_GAME_SESSION_HPP

#include "util/json_serializer.hpp"

using namespace godot;

struct StartSession {
	String game_pack_id;
	PackedStringArray effect_report_args;

	StartSession(){};

	StartSession(const String &p_game_pack_id, const PackedStringArray &p_effect_report_args) {
		game_pack_id = p_game_pack_id;
		effect_report_args = p_effect_report_args;
	}

	// for reasons unknown to me this throws an error in Visual Studio and Visual Studio Code
	// However the compiler is perfectly fine with this
	// There's nothing special about this class so I don't know what causes this
	constexpr static auto json_properties = std::make_tuple(
			json_property(&StartSession::game_pack_id, "gamePackID"),
			json_property(&StartSession::effect_report_args, "effectReportArgs"));
};

struct StopSession {
	String game_session_id;

	StopSession(){};

	StopSession(const String &p_game_session_id) {
		game_session_id = p_game_session_id;
	}

	// for reasons unknown to me this throws an error in Visual Studio and Visual Studio Code
	// However the compiler is perfectly fine with this
	// There's nothing special about this class so I don't know what causes this
	constexpr static auto json_properties = std::make_tuple(
			json_property(&StopSession::game_session_id, "gameSessionID"));
};

#endif // CROWD_CONTROL_GAME_SESSION_HPP