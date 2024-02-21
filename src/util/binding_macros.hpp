#ifndef CROWD_CONTROL_BINDING_MACROS_HPP
#define CROWD_CONTROL_BINDING_MACROS_HPP

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Only use during development, generally considered a code smell
#pragma region dev_only

#define GETSET(property, type)               \
	type get_##property() const {            \
		return property;                     \
	}                                        \
                                             \
	void set_##property(type p_##property) { \
		property = p_##property;             \
	}
#define GETSET_BOOL(property)                \
	bool is_##property() const {             \
		return property;                     \
	}                                        \
                                             \
	void set_##property(bool p_##property) { \
		property = p_##property;             \
	}
#define GETSET_CPTR(property, type)                 \
	type get_##property() const {                   \
		return property;                            \
	}                                               \
                                                    \
	void set_##property(const type &p_##property) { \
		property = p_##property;                    \
	}

#define BIND_GETSET(property, setter_arg)                                          \
	ClassDB::bind_method(D_METHOD("set_" #property, setter_arg), &set_##property); \
	ClassDB::bind_method(D_METHOD("get_" #property), &get_##property);

#define BIND_GETSET_BOOL(property, setter_arg)                                     \
	ClassDB::bind_method(D_METHOD("set_" #property, setter_arg), &set_##property); \
	ClassDB::bind_method(D_METHOD("is_" #property), &is_##property);

#define BIND_GET(property) \
	ClassDB::bind_method(D_METHOD("get_" #property), &get_##property);

#define BIND_GET_BOOL(property) \
	ClassDB::bind_method(D_METHOD("is_" #property), &is_##property);

#define BIND_PROPERTY(property, setter_arg, type) \
	BIND_GETSET(property, setter_arg)             \
	ADD_PROPERTY(PropertyInfo(type, #property), "set_" #property, "get_" #property);

#define BIND_PROPERTY_BOOL(property, setter_arg) \
	BIND_GETSET_BOOL(property, setter_arg)       \
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, #property), "set_" #property, "is_" #property);

#define BIND_PROPERTY_HINT(property, setter_arg, type, hint) \
	BIND_GETSET(property, setter_arg)                        \
	ADD_PROPERTY(PropertyInfo(type, #property, hint), "set_" #property, "get_" #property);

#define BIND_PROPERTY_RANGE(property, setter_arg, type, start, stop, step) \
	BIND_GETSET(property, setter_arg)                                      \
	ADD_PROPERTY(PropertyInfo(type, #property, PROPERTY_HINT_RANGE, #start "," #stop "," #step), "set_" #property, "get_" #property);

#define BIND_PROPERTY_ENUM(property, setter_arg, enum_names) \
	BIND_GETSET(property, setter_arg)                        \
	ADD_PROPERTY(PropertyInfo(Variant::INT, #property, PROPERTY_HINT_ENUM, enum_names), "set_" #property, "get_" #property);

#define BIND_PROPERTY_RESOURCE(property, setter_arg, class_name) \
	BIND_GETSET(property, setter_arg)                            \
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, #property, PROPERTY_HINT_RESOURCE_TYPE, #class_name), "set_" #property, "get_" #property);

#define BIND_PROPERTY_TYPED_ARRAY(property, setter_arg, type) \
	BIND_GETSET(property, setter_arg)                         \
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, #property, PROPERTY_HINT_ARRAY_TYPE, #type), "set_" #property, "get_" #property);

#pragma endregion

#define ADD_ARRAY_COUNT(m_label, m_count_property, m_count_property_setter, m_count_property_getter, m_prefix) add_property_array_count(get_class_static(), m_label, m_count_property, StringName(m_count_property_setter), StringName(m_count_property_getter), m_prefix)

void add_property_array_count(const StringName &p_class, const String &p_label, const StringName &p_count_property, const StringName &p_count_setter, const StringName &p_count_getter, const String &p_array_element_prefix, uint32_t p_count_usage = PROPERTY_USAGE_DEFAULT);

#endif // CROWD_CONTROL_BINDING_MACROS_HPP