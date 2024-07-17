#ifndef CROWD_CONTROL_STREAM_USER_HPP
#define CROWD_CONTROL_STREAM_USER_HPP

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include "util/binding_macros.hpp"

using namespace godot;
/**
 * Ideally I'd like to not expose this class/struct, it's merely an implementation detail the user barely needs to know about.
 * However it stores important data like sender data and icon. Perhaps a Dictionary can be used instead?
 */
class StreamUser : public RefCounted {
public:
	GDCLASS(StreamUser, RefCounted)

public:
	static Ref<StreamUser> make(String p_origin_site, String p_name, String p_display_name, String p_profile_icon_url, PackedStringArray p_roles, PackedStringArray p_subscriptions, String p_origin_id);
	static Ref<StreamUser> make(String p_name, String p_profile_icon_url, PackedStringArray p_roles);
	static Ref<StreamUser> make(String p_name, Ref<Texture2D> p_profile_icon);

protected:
	static void _bind_methods();

private:
	String name;
	String profile_icon_url;
	Ref<Texture2D> profile_icon;
	PackedStringArray roles;
	PackedStringArray subscriptions;
	int coins_spent = 0;
	String display_name;
	String origin_site = "";
	String origin_id;

public:
	String get_name() const { return name; }
	void set_name(const String &p_name) { name = p_name; }

	String get_profile_icon_url() const { return profile_icon_url; }
	void set_profile_icon_url(const String &p_profile_icon_url) { profile_icon_url = p_profile_icon_url; }

	Ref<Texture2D> get_profile_icon() const { return profile_icon; }
	void set_profile_icon(const Ref<Texture2D> &p_profile_icon) { profile_icon = p_profile_icon; }

	PackedStringArray get_roles() const { return roles; }
	void set_roles(const PackedStringArray &p_roles) { roles = p_roles; }

	PackedStringArray get_subscriptions() const { return subscriptions; }
	void set_subscriptions(const PackedStringArray &p_subscriptions) { subscriptions = p_subscriptions; }

	int get_coins_spent() const { return coins_spent; }
	void set_coins_spent(int p_coins_spent) { coins_spent = p_coins_spent; }

	String get_display_name() const { return display_name; }
	void set_display_name(const String &p_display_name) { display_name = p_display_name; }

	String get_origin_site() const { return origin_site; }
	void set_origin_site(const String &p_origin_site) { origin_site = p_origin_site; }

	String get_origin_id() const { return origin_id; }
	void set_origin_id(const String &p_origin_id) { origin_id = p_origin_id; }

	StreamUser(){};
	~StreamUser(){};
};

#endif // CROWD_CONTROL_STREAM_USER_HPP