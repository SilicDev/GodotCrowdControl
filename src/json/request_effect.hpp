#ifndef CROWD_CONTROL_REQUEST_EFFECT_HPP
#define CROWD_CONTROL_REQUEST_EFFECT_HPP

#include "util/json_serializer.hpp"

using namespace godot;

struct RequestSourceDetails {
	String type = "crowd-control-test";

	// for reasons unknown to me this throws an error in Visual Studio and Visual Studio Code
	// However the compiler is perfectly fine with this
	// There's nothing special about this class so I don't know what causes this
	constexpr static auto json_properties = std::make_tuple(
			json_property(&RequestSourceDetails::type, "type"));
};

struct RequestEffect {
	String game_session_id;
	RequestSourceDetails source_details = RequestSourceDetails();
	String effect_type = "game";
	String effect_id;

	RequestEffect(){

	};

	RequestEffect(const String &p_game_session_id, const String &p_effect_id) {
		game_session_id = p_game_session_id;
		effect_id = p_effect_id;
	};

	constexpr static auto json_properties = std::make_tuple(
			json_property(&RequestEffect::game_session_id, "gameSessionID"),
			json_property(&RequestEffect::source_details, "sourceDetails"),
			json_property(&RequestEffect::effect_type, "effectType"),
			json_property(&RequestEffect::effect_id, "effectID"));
};

#endif // CROWD_CONTROL_REQUEST_EFFECT_HPP