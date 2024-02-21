#ifndef CROWD_CONTROL_JSON_SERIALIZER
#define CROWD_CONTROL_JSON_SERIALIZER

#include <godot_cpp/classes/json.hpp>

#include <list>
#include <map>
#include <optional>
#include <type_traits>

using namespace godot;

// HC SVNT DRACONES

// https://stackoverflow.com/a/62763704
template <typename>
struct is_optional_impl {
	constexpr static bool value = false;
	using type = void;
};
template <typename T>
struct is_optional_impl<std::optional<T>> {
	constexpr static bool value = true;
	using type = std::remove_cv_t<T>;
};
template <>
struct is_optional_impl<std::nullopt_t> {
	constexpr static bool value = true;
	using type = std::nullopt_t;
};

template <typename T>
constexpr bool is_optional_v = is_optional_impl<std::remove_cv_t<T>>::value;
template <typename T>
using is_optional_t = typename is_optional_impl<std::remove_cv_t<T>>::type;
template <typename T>
using unwrap_optional_t = typename std::conditional<is_optional_v<T>, is_optional_t<T>, T>::type;
// For some reason std::_Is_any_of_v is inaccessible on linux so we define it here instead
#pragma region xtr1common
template <bool _First_value, class _First, class... _Rest>
struct my_Disjunction { // handle true trait or last trait
	using type = _First;
};

template <class _False, class _Next, class... _Rest>
struct my_Disjunction<false, _False, _Next, _Rest...> { // first trait is false, try the next trait
	using type = typename my_Disjunction<_Next::value, _Next, _Rest...>::type;
};

template <class... _Traits>
struct my_disjunction : std::false_type {}; // If _Traits is empty, false_type

template <class _First, class... _Rest>
struct my_disjunction<_First, _Rest...> : my_Disjunction<_First::value, _First, _Rest...>::type {
	// the first true trait in _Traits, or the last trait if none are true
};

template <class... _Traits>
inline constexpr bool disjunction_v = my_disjunction<_Traits...>::value;

template <typename Ty, typename... Types>
inline constexpr bool is_any_of_v = disjunction_v<std::is_same<Ty, Types>...>;
#pragma endregion xtr1common

template <typename Ty>
inline constexpr bool _is_trivially_serializable_v =
		std::is_arithmetic_v<Ty> ||
		is_any_of_v<
				// unwrap pointer
				std::remove_cv_t<Ty>,
				// complex type
				String, StringName, Array, Dictionary,
				// packed arrays
				PackedByteArray, PackedColorArray, PackedFloat32Array, PackedFloat64Array, PackedInt32Array, PackedInt64Array, PackedStringArray, PackedVector2Array, PackedVector3Array>;

template <typename Ty>
inline constexpr bool is_trivially_serializable_v = _is_trivially_serializable_v<Ty> || (is_optional_v<Ty> && _is_trivially_serializable_v<is_optional_t<Ty>>);

template <typename Ty, typename = void>
struct JSONSerializable : std::false_type {};

template <typename Ty>
struct JSONSerializable<Ty, std::void_t<decltype(Ty::deserialize(Dictionary())), decltype(Ty().serialize())>> : std::true_type {};

template <typename Ty, typename = void>
struct JSONPropertiesSerializable : std::false_type {};

template <typename Ty>
struct JSONPropertiesSerializable<Ty, std::void_t<decltype(Ty::json_properties), decltype(Ty())>> : std::true_type {};

template <typename Ty>
inline constexpr bool has_json_properties_v = JSONPropertiesSerializable<Ty>::value || (is_optional_v<Ty> && JSONPropertiesSerializable<is_optional_t<Ty>>::value);

template <typename Ty>
inline constexpr bool is_serializable_v = JSONSerializable<Ty>::value || (is_optional_v<Ty> && JSONSerializable<is_optional_t<Ty>>::value) || has_json_properties_v<Ty>;

// https://stackoverflow.com/a/34165367
template <typename Class, typename Ty>
struct JSONProperty {
	static_assert(is_trivially_serializable_v<Ty> || is_serializable_v<Ty>);
	constexpr JSONProperty(Ty Class::*p_member, const char *&p_name) :
			member{ p_member }, key{ p_name } {}
	using Type = Ty;
	const char *key;
	Ty Class::*member;
};

template <typename Class, typename Ty>
constexpr JSONProperty<Class, Ty> json_property(Ty Class::*p_member, const char *p_name) {
	return JSONProperty<Class, Ty>(p_member, p_name);
}

template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...>, F &&f) {
	(static_cast<void>(f(std::integral_constant<T, S>())), ...);
}

/// @brief Serializes objects into Json data and vice versa.
/// @details
/// Inspired by C#'s JsonConvert this class allows the automatic serialization and deserialization of objects.
/// It offers two different methods for that: Either by defining a json_properties static constexpr that holds
/// a std::tuple of JSONProperties, OR by defining a static deserialize function returning an instance  and a
/// serialize member function returning an array if complexer serialization is required.
class JSONSerializer {
public:
	template <typename T>
	static Dictionary serialize(const T &serializable, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		if (is_serializable_v<T>) {
			return serialize_properties<T>(serializable);
		}
		return Dictionary();
	}
	template <typename T>
	static T deserialize(const Dictionary &data, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		if (is_serializable_v<T>) {
			return deserialize_properties<T>(data);
		}
		T out;
		return out;
	}

	template <typename T>
	static String serialize_string(const T &serializable, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		if (is_serializable_v<T>) {
			return JSON::stringify(serialize_properties<T>(serializable));
		}
		return String();
	}

	template <typename T>
	static T deserialize_string(const String &data, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		if (is_serializable_v<T>) {
			return deserialize_properties<T>(JSON::parse_string(data));
		}
		T out;
		return out;
	}

private:
	template <typename T>
	static Dictionary serialize_properties(const T &serializable, typename std::enable_if<has_json_properties_v<T>, T>::type * = 0) {
		Dictionary out;

		constexpr uint32_t count = std::tuple_size<decltype(unwrap_optional_t<T>::json_properties)>::value;

		for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
			auto prop = std::get<i>(unwrap_optional_t<T>::json_properties);
			using Type = typename decltype(prop)::Type;
			write_property_data<Type>(serializable.*(prop.member), prop.key, out);
		});

		return out;
	}

	template <typename T>
	static void write_property_data(const T &serializable, const String &key, Dictionary &dict, typename std::enable_if<!is_optional_v<T>, T>::type * = 0) {
		dict[key] = get_property_data<T>(serializable);
	}

	template <typename T>
	static void write_property_data(const T &serializable, const String &key, Dictionary &dict, typename std::enable_if<is_optional_v<T>, is_optional_t<T>>::type * = 0) {
		if (serializable.has_value()) {
			dict[key] = get_property_data<is_optional_t<T>>(serializable.value());
		}
	}

	template <typename T>
	static T get_property_data(const T &serializable, typename std::enable_if<is_trivially_serializable_v<T>, T>::type * = 0) {
		return serializable;
	}

	template <typename T>
	static Dictionary get_property_data(const T &serializable, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		return serialize<T>(serializable);
	}

	template <typename T>
	static Dictionary serialize_properties(const T &serializable, typename std::enable_if<!has_json_properties_v<T> && !is_trivially_serializable_v<T> && !is_optional_v<T>, T>::type * = 0) {
		return serializable.serialize();
	}

	template <typename T>
	static Dictionary serialize_properties(const T &serializable, typename std::enable_if<!has_json_properties_v<T> && !is_trivially_serializable_v<T> && is_optional_v<T>, T>::type * = 0) {
		if (serializable.has_value()) {
			return serializable.serialize();
		}
		return Dictionary();
	}

	template <typename T>
	static T deserialize_properties(const Dictionary &data, typename std::enable_if<has_json_properties_v<T>, T>::type * = 0) {
		T out;

		constexpr uint32_t count = std::tuple_size<decltype(unwrap_optional_t<T>::json_properties)>::value;

		for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
			auto prop = std::get<i>(unwrap_optional_t<T>::json_properties);
			using Type = typename decltype(prop)::Type;
			// this is enough for optional handling as the = operator is overridden for them
			if (!is_optional_v<Type> || data.has(prop.key)) {
				out.*(prop.member) = get_property<unwrap_optional_t<Type>>(prop.key, data);
			}
		});

		return out;
	}

	template <typename T>
	static T deserialize_properties(const Dictionary &data, typename std::enable_if<!has_json_properties_v<T> && !is_trivially_serializable_v<T>, T>::type * = 0) {
		return T::deserialize(data);
	}

	template <typename T>
	static T get_property(const String &prop_key, const Dictionary &data, typename std::enable_if<is_trivially_serializable_v<T>, T>::type * = 0) {
		return static_cast<T>(data[prop_key]);
	}

	template <typename T>
	static T get_property(const String &prop_key, const Dictionary &data, typename std::enable_if<!is_trivially_serializable_v<T>, T>::type * = 0) {
		return deserialize<T>(data[prop_key]);
	}
};

#endif // CROWD_CONTROL_JSON_SERIALIZER