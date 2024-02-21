#ifndef CROWD_CONTROL_EFFECT_REPORT_HPP
#define CROWD_CONTROL_EFFECT_REPORT_HPP

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/templates/list.hpp>

#include "util/json_serializer.hpp"

using namespace godot;

struct EffectReportArgs {
	String id = "";
	String status;
	uint64_t stamp = 1;
	PackedStringArray ids;
	String effect_type = "game";
	String identifier_type = "effect";

	EffectReportArgs(const String &p_status, const PackedStringArray &p_ids) {
		status = p_status;
		ids = p_ids;
		id = CCNetworkManager::generate_random_string(26);
		stamp = Time::get_singleton()->get_unix_time_from_system();
	};

	EffectReportArgs(){};

	constexpr static auto json_properties = std::make_tuple(
			json_property(&EffectReportArgs::id, "id"),
			json_property(&EffectReportArgs::status, "status"),
			json_property(&EffectReportArgs::stamp, "stamp"),
			json_property(&EffectReportArgs::ids, "ids"),
			json_property(&EffectReportArgs::effect_type, "effectType"),
			json_property(&EffectReportArgs::identifier_type, "identifierType"));
};

struct EffectReportCall {
	String method = "effectReport";
	List<EffectReportArgs> args = List<EffectReportArgs>();
	String id = "string";
	String type = "call";

	EffectReportCall() {
		args.push_front(EffectReportArgs());
	};

	static EffectReportCall deserialize(const Dictionary &data) {
		EffectReportCall out;
		out.method = data.get("method", "");
		//parameters
		out.args = List<EffectReportArgs>();
		Array args = data.get("args", Array());
		for (int i = 0; i < args.size(); i++) {
			out.args.push_back(JSONSerializer::deserialize<EffectReportArgs>(args[i]));
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
			out_args.append(JSONSerializer::serialize<EffectReportArgs>(args[i]));
		}
		out["args"] = out_args;
		out["id"] = id;
		out["type"] = type;
		return out;
	}
};

struct EffectReport {
	String token = "";
	EffectReportCall call = EffectReportCall();

	EffectReport() {
	}

	EffectReport(const String &p_token, const Ref<CCEffect> &p_effect, const String &p_status) {
		token = p_token;
		call.args[0] = EffectReportArgs(p_status, Array::make(p_effect->get_id()));
	}

	constexpr static auto json_properties = std::make_tuple(
			json_property(&EffectReport::token, "token"),
			json_property(&EffectReport::call, "call"));
};

#endif // CROWD_CONTROL_EFFECT_REPORT_HPP