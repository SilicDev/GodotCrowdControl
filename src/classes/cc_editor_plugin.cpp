#include "cc_editor_plugin.hpp"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/callable.hpp>

#include "classes/cc_effect.hpp"

void CCEditorInspectorPlugin::_create_manifest(Ref<CCEffectEntries> p_entries) {
	ERR_FAIL_NULL_EDMSG(p_entries, "No effect entries loaded!");
	ERR_FAIL_NULL_EDMSG(manifest_info, "Unable to show manifest info!");
	String text = p_entries->get_manifest();
	manifest_text->set_text(text);
	DisplayServer::get_singleton()->clipboard_set(text);
	manifest_info->popup_centered();
}

bool CCEditorInspectorPlugin::_can_handle(Object *p_object) const {
	return Object::cast_to<CCEffectEntries>(p_object) != nullptr;
}

void CCEditorInspectorPlugin::_parse_end(Object *p_object) {
	Ref<CCEffectEntries> entries = Object::cast_to<CCEffectEntries>(p_object);
	if (entries != nullptr) {
		Label *label = memnew(Label);
		label->set_text("Actions");
		label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		add_custom_control(label);

		Button *button = memnew(Button);
		button->set_text("JSON Manifest");
		// Callable.bind is broken in 4.1 so we use bindv to have the array handle the Variants
		button->connect("pressed", Callable(this, "__on_manifest").bindv(Array::make(entries)));
		add_custom_control(button);
		if (manifest_info == nullptr) {
			manifest_info = memnew(AcceptDialog);
			manifest_info->set_min_size(Vector2i(500, 300));
			// add it to the editor root so we inherit the theme
			plugin->get_editor_interface()->get_base_control()->add_child(manifest_info);

			manifest_text = memnew(RichTextLabel);
			manifest_text->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
			manifest_text->set_selection_enabled(true);
			manifest_text->set_scroll_active(true);
			manifest_info->add_child(manifest_text, false, Node::INTERNAL_MODE_FRONT);
			manifest_info->move_child(manifest_text, 1);
		}
	}
}

void CCEditorInspectorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("__on_manifest", "entries"), &CCEditorInspectorPlugin::_create_manifest);
}

CCEditorInspectorPlugin::CCEditorInspectorPlugin(CCEditorPlugin *p_plugin) {
	plugin = p_plugin;
}

void CCEditorPlugin::_edit(Object *p_object) {
	if (!p_object) {
		// cleanup
		return;
	}
}

void CCEditorPlugin::_make_visible(bool p_visible) {
	if (p_visible) {
		// make visible
	}
}

bool CCEditorPlugin::_handles(Object *p_object) const {
	return p_object->is_class("CCEffectEntries");
}

CCEditorPlugin::CCEditorPlugin() {
	inspector_plugin = memnew(CCEditorInspectorPlugin(this));
	call_deferred("add_inspector_plugin", inspector_plugin);
}

CCEditorPlugin::~CCEditorPlugin() {
	call_deferred("remove_inspector_plugin", inspector_plugin);
}
