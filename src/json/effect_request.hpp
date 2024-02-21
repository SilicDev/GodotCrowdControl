#ifndef CROWD_CONTROL_EFFECT_REQUEST_HPP
#define CROWD_CONTROL_EFFECT_REQUEST_HPP

#include <godot_cpp/templates/hash_map.hpp>

#include "json/user.hpp"
#include "util/json_serializer.hpp"

using namespace godot;

struct ParameterEntry {
	String name;
	String title;
	String type;
	String value;

	// for reasons unknown to me this *sometimes* throws an error in Visual Studio and Visual Studio Code
	// However the compiler is perfectly fine with this
	// There's nothing special about this class so I don't know what causes this
	constexpr static auto json_properties = std::make_tuple(
			json_property(&ParameterEntry::name, "name"),
			json_property(&ParameterEntry::title, "title"),
			json_property(&ParameterEntry::type, "type"),
			json_property(&ParameterEntry::value, "value"));
};

struct Parameters {
	String name;
	String type;
	HashMap<String, ParameterEntry> options;

	static Parameters deserialize(const Dictionary &data) {
		Parameters out;
		out.name = data.get("name", "");
		out.type = data.get("type", "");
		//parameters
		out.options = HashMap<String, ParameterEntry>();
		Dictionary parameters = data.get("options", Dictionary());
		for (int i = 0; i < parameters.size(); i++) {
			String key = parameters.keys()[i];
			out.options.insert(key, JSONSerializer::deserialize<ParameterEntry>(parameters[key]));
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["name"] = name;
		out["type"] = type;
		//parameters
		Dictionary out_parameters;
		for (KeyValue<String, ParameterEntry> kv : options) {
			out_parameters[kv.key] = JSONSerializer::serialize<ParameterEntry>(kv.value);
		}
		out["options"] = out_parameters;
		return out;
	}
};

struct Effect {
	String effect_id;
	String type;
	String name;
	String note;
	PackedStringArray category;
	String description;
	uint32_t price;
	HashMap<String, Parameters> parameters;
	String image_url;

	static Effect deserialize(const Dictionary &data) {
		Effect out;
		out.effect_id = data.get("effectID", "");
		out.type = data.get("type", "");
		out.name = data.get("name", "");
		out.note = data.get("note", "");
		out.category = data.get("category", PackedStringArray());
		out.description = data.get("description", "");
		out.price = data.get("price", 1);
		//parameters
		out.parameters = HashMap<String, Parameters>();
		Dictionary parameters = data.get("parameters", Dictionary());
		for (int i = 0; i < parameters.size(); i++) {
			String key = parameters.keys()[i];
			out.parameters.insert(key, JSONSerializer::deserialize<Parameters>(parameters[key]));
		}
		out.image_url = data.get("image", "");
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["effectID"] = effect_id;
		out["type"] = type;
		out["name"] = name;
		out["note"] = note;
		out["category"] = category;
		out["description"] = description;
		out["price"] = price;
		//parameters
		Dictionary out_parameters;
		for (KeyValue<String, Parameters> kv : parameters) {
			out_parameters[kv.key] = JSONSerializer::serialize<Parameters>(kv.value);
		}
		out["parameters"] = out_parameters;
		out["image"] = image_url;
		return out;
	}
};

struct Game {
	String game_id;
	String name;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&Game::game_id, "gameID"),
			json_property(&Game::name, "name"));
};

struct GamePack {
	String game_pack_id;
	String name;
	String platform;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&GamePack::game_pack_id, "gamePackID"),
			json_property(&GamePack::name, "name"),
			json_property(&GamePack::platform, "platform"));
};

struct EffectRequest {
	String request_id;
	String game_session_id;
	String bank_id;
	String created_at;
	String updated_at;
	String type;
	User target;
	User requester;
	Game game;
	GamePack game_pack;
	Effect effect;
	uint32_t unit_cost;
	bool is_test;
	bool is_anonymous;
	HashMap<String, ParameterEntry> parameters;
	String status;
	uint32_t quantity;

	static EffectRequest deserialize(const Dictionary &data) {
		EffectRequest out;
		out.request_id = data.get("requestID", "");
		out.game_session_id = data.get("gameSessionID", "");
		out.bank_id = data.get("bankID", "");
		out.created_at = data.get("createdAt", "");
		out.updated_at = data.get("updatedAt", "");
		out.type = data.get("type", "");
		out.target = JSONSerializer::deserialize<User>(data.get("target", Dictionary()));
		out.requester = JSONSerializer::deserialize<User>(data.get("requester", Dictionary()));
		out.game = JSONSerializer::deserialize<Game>(data.get("game", Dictionary()));
		out.game_pack = JSONSerializer::deserialize<GamePack>(data.get("gamePack", Dictionary()));
		out.effect = JSONSerializer::deserialize<Effect>(data.get("effect", Dictionary()));
		out.unit_cost = data.get("unitPrice", 1);
		out.is_test = data.get("isTest", false);
		out.is_anonymous = data.get("anonymous", false);
		//parameters
		out.parameters = HashMap<String, ParameterEntry>();
		Dictionary parameters = data.get("parameters", Dictionary());
		for (int i = 0; i < parameters.size(); i++) {
			String key = parameters.keys()[i];
			out.parameters.insert(key, JSONSerializer::deserialize<ParameterEntry>(parameters[key]));
		}
		out.status = data.get("status", "");
		out.quantity = data.get("quantity", 1);
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["requestID"] = request_id;
		out["gameSessionID"] = game_session_id;
		out["bankID"] = bank_id;
		out["createdAt"] = created_at;
		out["updatedAt"] = updated_at;
		out["type"] = type;
		out["target"] = JSONSerializer::serialize<User>(target);
		out["requester"] = JSONSerializer::serialize<User>(requester);
		out["game"] = JSONSerializer::serialize<Game>(game);
		out["gamePack"] = JSONSerializer::serialize<GamePack>(game_pack);
		out["effect"] = JSONSerializer::serialize<Effect>(effect);
		out["unitPrice"] = unit_cost;
		out["isTest"] = is_test;
		out["anonymous"] = is_anonymous;
		//parameters
		Dictionary out_parameters;
		for (KeyValue<String, ParameterEntry> kv : parameters) {
			out_parameters[kv.key] = JSONSerializer::serialize<ParameterEntry>(kv.value);
		}
		out["parameters"] = out_parameters;
		out["status"] = status;
		out["quantity"] = quantity;
		return out;
	}
};

#endif // CROWD_CONTROL_EFFECT_REQUEST_HPP