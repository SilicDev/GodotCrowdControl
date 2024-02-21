#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "classes/cc_editor_plugin.hpp"
#include "classes/cc_effect_instance.hpp"
#include "classes/cc_network_manager.hpp"
#include "classes/crowd_control.hpp"
#include "classes/stream_user.hpp"

using namespace godot;

void gdextension_initialize(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(StreamUser);
		GDREGISTER_CLASS(CCEffect);
		GDREGISTER_CLASS(CCEffectTimed);
		GDREGISTER_CLASS(CCEffectBidWar);
		GDREGISTER_CLASS(CCEffectParameters);
		GDREGISTER_CLASS(CCEffectInstance);
		GDREGISTER_CLASS(CCEffectInstanceTimed);
		GDREGISTER_CLASS(CCEffectInstanceBidWar);
		GDREGISTER_CLASS(CCEffectInstanceParameters);
		GDREGISTER_CLASS(CCEffectEntries);
		GDREGISTER_CLASS(CrowdControl);
		GDREGISTER_ABSTRACT_CLASS(CCNetworkManager);
		if (!Engine::get_singleton()->has_singleton("CrowdControl")) {
			Engine::get_singleton()->register_singleton("CrowdControl", CrowdControl::get_singleton());
		}
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(CCEditorPlugin);
		GDREGISTER_CLASS(CCEditorInspectorPlugin);
		EditorPlugins::add_by_type<CCEditorPlugin>();
	}
}

void gdextension_terminate(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		Engine::get_singleton()->unregister_singleton("CrowdControl");
		memdelete(CrowdControl::get_singleton());
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT gdextension_init(GDExtensionInterfaceGetProcAddress p_interface, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

	init_obj.register_initializer(gdextension_initialize);
	init_obj.register_terminator(gdextension_terminate);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

	return init_obj.init();
}
}
