#ifndef CROWD_CONTROL_MANIFEST_HPP
#define CROWD_CONTROL_MANIFEST_HPP

#include <godot_cpp/templates/hash_map.hpp>

#include "classes/cc_effect.hpp"
#include "util/json_serializer.hpp"

#include <optional>

struct MetaData {
	String platform = "PC";
	String name;
	String safe_name;
	String notes;
	PackedStringArray connector;
	String emulator;
	bool patch;
	String guide;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&MetaData::platform, "platform"),
			json_property(&MetaData::name, "name"),
			json_property(&MetaData::safe_name, "safename"),
			json_property(&MetaData::notes, "notes"),
			json_property(&MetaData::connector, "connector"),
			json_property(&MetaData::emulator, "emulator"),
			json_property(&MetaData::patch, "patch"),
			json_property(&MetaData::guide, "guide"));
};

struct EffectManifest {
	struct Quantity {
		uint32_t min = 1;
		uint32_t max = 99;

		Quantity(){};
		Quantity(uint32_t p_min, uint32_t p_max) {
			min = p_min;
			max = p_max;
		}

		constexpr static auto json_properties = std::make_tuple(
				json_property(&Quantity::min, "min"),
				json_property(&Quantity::max, "max"));
	};

	struct Duration {
		uint64_t value = 1;

		Duration(){};
		Duration(uint64_t p_duration) {
			value = p_duration;
		}

		constexpr static auto json_properties = std::make_tuple(json_property(&Duration::value, "value"));
	};

	bool inactive;
	bool disabled;
	bool no_pooling;
	String name;
	String sort_name;
	String note;
	PackedStringArray category;
	String description;
	uint32_t price = 1;
	int32_t moral;
	// TODO: Handle optionals in JSONSerializer instead
	std::optional<Dictionary> parameters;
	std::optional<Quantity> quantity;
	std::optional<Duration> duration;

	EffectManifest(){};
	EffectManifest(Ref<CCEffect> effect) {
		inactive = !effect->is_sellable();
		disabled = !effect->is_visible();
		name = effect->get_name();
		category = effect->get_categories();
		description = effect->get_description();
		price = effect->get_price();
		no_pooling = effect->is_pooling_disabled();
		moral = effect->get_morality();
		if (moral == CCEffect::Morality::EVIL) {
			moral = -1;
		}
		if (effect->get_effect_type() == CCEffect::EffectType::TIMED) {
			Ref<CCEffectTimed> timed = Object::cast_to<CCEffectTimed>(effect.ptr());
			duration = Duration(timed->get_duration());
		} else if (effect->get_effect_type() == CCEffect::EffectType::PARAMETERS) {
			Ref<CCEffectParameters> params = Object::cast_to<CCEffectParameters>(effect.ptr());
			parameters = Dictionary();
			for (KeyValue<String, CCEffectParameters::ParameterEntry> kv : params->get_parameter_entries_by_id()) {
				if (kv.value.id == "quantity") {
					quantity = Quantity(kv.value.min, kv.value.max);
					continue;
				}
				Dictionary param;
				param["name"] = kv.value.name;
				if (kv.value.kind == CCEffectParameters::ParameterEntryKind::ITEM) {
					param["type"] = "options";
					Dictionary options;
					for (int i = 0; i < kv.value.options.size(); i++) {
					}
					param["options"] = options;
				} else if (kv.value.kind == CCEffectParameters::ParameterEntryKind::RANGE) {
					param["type"] = "quantity";
					Dictionary quantity;
					quantity["min"] = kv.value.min;
					quantity["max"] = kv.value.max;
					param["quantity"] = quantity;
				}
				parameters.value()[kv.value.id] = param;
			}
		} else if (effect->get_effect_type() == CCEffect::EffectType::BID_WAR) {
			Ref<CCEffectBidWar> bid_war = Object::cast_to<CCEffectBidWar>(effect.ptr());
			parameters = Dictionary();
			Dictionary data;
			data["name"] = vformat("%s Options", bid_war->get_name());
			data["type"] = "options";
			Dictionary options;
			for (KeyValue<String, CCEffectBidWar::BidWarEntry> kv : bid_war->get_entries_by_id()) {
				Dictionary option;
				option["name"] = kv.value.name;
				options[kv.key] = option;
			}
			data["options"] = options;
			parameters.value()[vformat("%s_options", bid_war->get_id())] = data;
		}
	}

	/*static EffectManifest deserialize(const Dictionary &data) {
		EffectManifest out;
		out.inactive = data["inactive"];
		out.disabled = data["disabled"];
		out.no_pooling = data["unpoolable"];
		out.name = data["name"];
		out.sort_name = data["sortName"];
		out.note = data["note"];
		out.category = data["category"];
		out.description = data["description"];
		out.price = data["price"];
		out.moral = data["moral"];
		if (data.has("parameters")) {
			out.parameters = data["parameters"];
		} else {
			out.parameters = std::nullopt;
		}
		if (data.has("quantity")) {
			out.quantity = JSONSerializer::deserialize<Quantity>(data["quantity"]);
		} else {
			out.quantity = std::nullopt;
		}
		if (data.has("duration")) {
			out.duration = JSONSerializer::deserialize<Duration>(data["duration"]);
		} else {
			out.duration = std::nullopt;
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		out["inactive"] = inactive;
		out["disabled"] = disabled;
		out["unpoolable"] = no_pooling;
		out["name"] = name;
		out["sortName"] = sort_name;
		out["note"] = note;
		out["category"] = category;
		out["description"] = description;
		out["price"] = price;
		out["moral"] = moral;
		if (parameters.has_value()) {
			out["parameters"] = parameters.value();
		}
		if (quantity.has_value()) {
			out["quantity"] = JSONSerializer::serialize<Quantity>(quantity.value());
		}
		if (duration.has_value()) {
			out["duration"] = JSONSerializer::serialize<Duration>(duration.value());
		}
		return out;
	}*/

	constexpr static auto json_properties = std::make_tuple(
			json_property(&EffectManifest::inactive, "inactive"),
			json_property(&EffectManifest::disabled, "disabled"),
			json_property(&EffectManifest::no_pooling, "unpoolable"),
			json_property(&EffectManifest::name, "name"),
			json_property(&EffectManifest::sort_name, "sortName"),
			json_property(&EffectManifest::note, "note"),
			json_property(&EffectManifest::category, "category"),
			json_property(&EffectManifest::description, "description"),
			json_property(&EffectManifest::price, "price"),
			json_property(&EffectManifest::moral, "moral"),
			json_property(&EffectManifest::parameters, "parameters"),
			json_property(&EffectManifest::quantity, "quantity"),
			json_property(&EffectManifest::duration, "duration"));
};

struct Effects {
	HashMap<String, EffectManifest> game_effects = HashMap<String, EffectManifest>();

	static Effects deserialize(const Dictionary &data) {
		Effects out;
		out.game_effects = HashMap<String, EffectManifest>();
		Dictionary game = data.get("game", Dictionary());
		for (int i = 0; i < game.size(); i++) {
			String key = game.keys()[i];
			out.game_effects.insert(key, JSONSerializer::deserialize<EffectManifest>(game[key]));
		}
		return out;
	}

	Dictionary serialize() const {
		Dictionary out;
		Dictionary game;
		for (KeyValue<String, EffectManifest> kv : game_effects) {
			game[kv.key] = JSONSerializer::serialize<EffectManifest>(kv.value);
		}
		out["game"] = game;
		return out;
	}
};

struct Manifest {
	MetaData meta;
	Effects effects;

	Manifest(){};

	Manifest(String p_game_name, HashMap<String, Ref<CCEffect>> p_effects) {
		meta = MetaData();
		meta.name = p_game_name;
		effects = Effects();
		for (KeyValue<String, Ref<CCEffect>> kv : p_effects) {
			effects.game_effects.insert(kv.key, EffectManifest(kv.value));
		}
	}

	constexpr static auto json_properties = std::make_tuple(
			json_property(&Manifest::meta, "meta"),
			json_property(&Manifest::effects, "effects"));
};

#endif // CROWD_CONTROL_MANIFEST_HPP