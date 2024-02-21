#ifndef CROWD_CONTROL_EFFECT_OVERRIDE_HPP
#define CROWD_CONTROL_EFFECT_OVERRIDE_HPP

#include <godot_cpp/templates/list.hpp>

#include "util/json_serializer.hpp"
#include "util/project_settings.hpp"

using namespace godot;

// Might wanna add a base class for these, but the JSONSerializer doesn't behave well with it

struct EffectChangePrice {
	struct EffectChangePriceEntry {
		String effect_id = "";
		String type = "game";
		uint32_t price = 0;

		void set(const String &p_id, const uint32_t &p_value) {
			effect_id = p_id;
			price = p_value;
		}

		constexpr static auto json_properties = std::make_tuple(
				json_property(&EffectChangePriceEntry::effect_id, "effectID"),
				json_property(&EffectChangePriceEntry::type, "type"),
				json_property(&EffectChangePriceEntry::price, "price"));
	};

	String game_pack_id = GLOBAL_GET("crowd_control/common/game_id");
	List<EffectChangePriceEntry> effect_overrides = List<EffectChangePriceEntry>();

	EffectChangePrice() {
		effect_overrides.push_back(EffectChangePriceEntry());
	}

	EffectChangePrice(const String &p_effect_id, uint32_t p_price) {
		EffectChangePriceEntry entry = EffectChangePriceEntry();
		entry.set(p_effect_id, p_price);
		effect_overrides.push_back(entry);
	}

	static EffectChangePrice deserialize(const Dictionary &data) {
		EffectChangePrice out;
		out.game_pack_id = data.get("gamePackID", "");
		//parameters
		out.effect_overrides = List<EffectChangePriceEntry>();
		Array args = data.get("effectOverrides", Array());
		for (int i = 0; i < args.size(); i++) {
			out.effect_overrides.push_back(JSONSerializer::deserialize<EffectChangePriceEntry>(args[i]));
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["gamePackID"] = game_pack_id;
		//parameters
		Array out_args;
		for (int i = 0; i < effect_overrides.size(); i++) {
			out_args.append(JSONSerializer::serialize<EffectChangePriceEntry>(effect_overrides[i]));
		}
		out["effectOverrides"] = out_args;
		return out;
	}
};

struct EffectChangeNonPoolable {
	struct EffectChangeNonPoolableEntry {
		String effect_id = "";
		String type = "game";
		bool non_poolable = false;

		void set(const String &p_id, const bool &p_value) {
			effect_id = p_id;
			non_poolable = p_value;
		}

		// for reasons unknown to me this *sometimes* throws an error in Visual Studio and Visual Studio Code
		// However the compiler is perfectly fine with this
		// There's nothing special about this class so I don't know what causes this
		constexpr static auto json_properties = std::make_tuple(
				json_property(&EffectChangeNonPoolableEntry::effect_id, "effectID"),
				json_property(&EffectChangeNonPoolableEntry::type, "type"),
				json_property(&EffectChangeNonPoolableEntry::non_poolable, "unpoolable"));
	};

	String game_pack_id = GLOBAL_GET("crowd_control/common/game_id");
	List<EffectChangeNonPoolableEntry> effect_overrides = List<EffectChangeNonPoolableEntry>();

	EffectChangeNonPoolable() {
		effect_overrides.push_back(EffectChangeNonPoolableEntry());
	}

	EffectChangeNonPoolable(const String &p_effect_id, bool p_non_poolable) {
		EffectChangeNonPoolableEntry entry = EffectChangeNonPoolableEntry();
		entry.set(p_effect_id, p_non_poolable);
		effect_overrides.push_back(entry);
	}

	static EffectChangeNonPoolable deserialize(const Dictionary &data) {
		EffectChangeNonPoolable out;
		out.game_pack_id = data.get("gamePackID", "");
		//parameters
		out.effect_overrides = List<EffectChangeNonPoolableEntry>();
		Array args = data.get("effectOverrides", Array());
		for (int i = 0; i < args.size(); i++) {
			out.effect_overrides.push_back(JSONSerializer::deserialize<EffectChangeNonPoolableEntry>(args[i]));
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["gamePackID"] = game_pack_id;
		//parameters
		Array out_args;
		for (int i = 0; i < effect_overrides.size(); i++) {
			out_args.append(JSONSerializer::serialize<EffectChangeNonPoolableEntry>(effect_overrides[i]));
		}
		out["effectOverrides"] = out_args;
		return out;
	}
};

struct EffectChangeSessionMax {
	struct EffectChangeSessionMaxEntry {
		String effect_id = "";
		String type = "game";
		uint32_t session_max = 0;

		void set(const String &p_id, const uint32_t &p_value) {
			effect_id = p_id;
			session_max = p_value;
		}

		constexpr static auto json_properties = std::make_tuple(
				json_property(&EffectChangeSessionMaxEntry::effect_id, "effectID"),
				json_property(&EffectChangeSessionMaxEntry::type, "type"),
				json_property(&EffectChangeSessionMaxEntry::session_max, "sessionMax"));
	};

	String game_pack_id = GLOBAL_GET("crowd_control/common/game_id");
	List<EffectChangeSessionMaxEntry> effect_overrides = List<EffectChangeSessionMaxEntry>();

	EffectChangeSessionMax() {
		effect_overrides.push_back(EffectChangeSessionMaxEntry());
	}

	EffectChangeSessionMax(const String &p_effect_id, uint32_t p_price) {
		EffectChangeSessionMaxEntry entry = EffectChangeSessionMaxEntry();
		effect_overrides.push_back(entry);
		entry.set(p_effect_id, p_price);
	}

	static EffectChangeSessionMax deserialize(const Dictionary &data) {
		EffectChangeSessionMax out;
		out.game_pack_id = data.get("gamePackID", "");
		//parameters
		out.effect_overrides = List<EffectChangeSessionMaxEntry>();
		Array args = data.get("effectOverrides", Array());
		for (int i = 0; i < args.size(); i++) {
			out.effect_overrides.push_back(JSONSerializer::deserialize<EffectChangeSessionMaxEntry>(args[i]));
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["gamePackID"] = game_pack_id;
		//parameters
		Array out_args;
		for (int i = 0; i < effect_overrides.size(); i++) {
			out_args.append(JSONSerializer::serialize<EffectChangeSessionMaxEntry>(effect_overrides[i]));
		}
		out["effectOverrides"] = out_args;
		return out;
	}
};

#endif // CROWD_CONTROL_EFFECT_OVERRIDE_HPP