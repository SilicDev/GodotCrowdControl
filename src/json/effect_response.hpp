#ifndef CROWD_CONTROL_EFFECT_RESPONSE_HPP
#define CROWD_CONTROL_EFFECT_RESPONSE_HPP

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/templates/list.hpp>

#include "classes/cc_effect.hpp"
#include "classes/cc_effect_instance.hpp"
#include "classes/cc_network_manager.hpp"
#include "util/json_serializer.hpp"

using namespace godot;

struct EffectResponseArgs {
	String id = "";
	String status;
	uint64_t stamp = 1;
	float time_remaining;
	String request;
	String message;

	EffectResponseArgs(const Ref<CCEffectInstance> &instance, const String &p_status) {
		status = p_status;
		request = instance->get_id();
		id = CCNetworkManager::generate_random_string(26);
		stamp = Time::get_singleton()->get_unix_time_from_system();
		if (instance->get_effect()->get_effect_type() == CCEffect::EffectType::TIMED) {
			Ref<CCEffectInstanceTimed> timed = Object::cast_to<CCEffectInstanceTimed>(instance.ptr());
			time_remaining = timed->get_time_left();
		}
	};

	EffectResponseArgs(){};

	constexpr static auto json_properties = std::make_tuple(
			json_property(&EffectResponseArgs::id, "id"),
			json_property(&EffectResponseArgs::status, "status"),
			json_property(&EffectResponseArgs::stamp, "stamp"),
			json_property(&EffectResponseArgs::time_remaining, "timeRemaining"),
			json_property(&EffectResponseArgs::request, "request"),
			json_property(&EffectResponseArgs::message, "message"));
};

struct EffectResponseCall {
	String method = "effectResponse";
	List<EffectResponseArgs> args = List<EffectResponseArgs>();
	String id = "";
	String type = "call";

	EffectResponseCall() {
		args.push_front(EffectResponseArgs());
	};

	static EffectResponseCall deserialize(const Dictionary &data) {
		EffectResponseCall out;
		out.method = data.get("method", "");
		//parameters
		out.args = List<EffectResponseArgs>();
		Array args = data.get("args", Array());
		for (int i = 0; i < args.size(); i++) {
			out.args.push_back(JSONSerializer::deserialize<EffectResponseArgs>(args[i]));
		}
		out.id = data.get("id", "");
		out.type = data.get("type", "");
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["method"] = method;
		//parameters
		Array out_args;
		for (int i = 0; i < args.size(); i++) {
			out_args.append(JSONSerializer::serialize<EffectResponseArgs>(args[i]));
		}
		out["args"] = out_args;
		out["id"] = id;
		out["type"] = type;
		return out;
	}
};

struct EffectResponse {
	String token = "";
	EffectResponseCall call = EffectResponseCall();

	EffectResponse() {
	}

	EffectResponse(const String &p_token, const Ref<CCEffectInstance> &p_instance, const String &p_status) {
		token = p_token;
		call.args[0] = EffectResponseArgs(p_instance, p_status);
	}

	constexpr static auto json_properties = std::make_tuple(
			json_property(&EffectResponse::token, "token"),
			json_property(&EffectResponse::call, "call"));
};

#endif // CROWD_CONTROL_EFFECT_RESPONSE_HPP