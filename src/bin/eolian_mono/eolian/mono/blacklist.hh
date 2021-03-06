#ifndef EOLIAN_MONO_BLACKLIST_HH
#define EOLIAN_MONO_BLACKLIST_HH

#include "grammar/klass_def.hpp"
#include "grammar/context.hpp"
#include "name_helpers.hh"
#include "generation_contexts.hh"

namespace eolian_mono {

namespace blacklist {

namespace attributes = efl::eolian::grammar::attributes;

inline bool is_function_blacklisted(std::string const& c_name)
{
  return
    c_name == "efl_event_callback_array_priority_add"
    || c_name == "efl_player_position_get"
    || c_name == "efl_ui_widget_focus_set"
    || c_name == "efl_ui_widget_focus_get"
    || c_name == "efl_ui_text_password_get"
    || c_name == "efl_ui_text_password_set"
    || c_name == "elm_interface_scrollable_repeat_events_get"
    || c_name == "elm_interface_scrollable_repeat_events_set"
    || c_name == "elm_wdg_item_del"
    || c_name == "elm_interface_scrollable_mirrored_set"
    || c_name == "evas_obj_table_mirrored_get"
    || c_name == "evas_obj_table_mirrored_set"
    || c_name == "edje_obj_load_error_get"
    || c_name == "efl_ui_focus_user_parent_get"
    || c_name == "efl_canvas_object_scale_get" // duplicated signature
    || c_name == "efl_canvas_object_scale_set" // duplicated signature
    || c_name == "efl_access_parent_get"
    || c_name == "efl_access_name_get"
    || c_name == "efl_access_name_set"
    || c_name == "efl_access_root_get"
    || c_name == "efl_access_type_get"
    || c_name == "efl_access_role_get"
    || c_name == "efl_access_action_description_get"
    || c_name == "efl_access_action_description_set"
    || c_name == "efl_access_image_description_get"
    || c_name == "efl_access_image_description_set"
    || c_name == "efl_access_component_layer_get" // duplicated signature
    || c_name == "efl_access_component_alpha_get"
    || c_name == "efl_access_component_size_get"
    || c_name == "efl_ui_spin_button_loop_get"
    || c_name == "efl_ui_list_model_size_get"
    || c_name == "efl_ui_list_relayout_layout_do"
    || c_name == "efl_event_callback_forwarder_priority_add" // Depends on constants support.
    ;
}

template<typename Context>
inline bool is_function_blacklisted(attributes::function_def const& func, Context context)
{
  auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
  auto c_name = func.c_name;

  if (func.is_beta && !options.want_beta)
    return true;

  return is_function_blacklisted(c_name);
}


// Blacklist structs that require some kind of manual binding.
inline bool is_struct_blacklisted(std::string const& full_name)
{
   return full_name == "Efl.Event_Description"
       || full_name == "Eina.Binbuf"
       || full_name == "Eina.Strbuf"
       || full_name == "Eina.Slice"
       || full_name == "Eina.Rw_Slice"
       || full_name == "Eina.Promise"
       || full_name == "Eina.Value"
       || full_name == "Eina.Value_Type"
       || full_name == "Eina.Future";
}

inline bool is_struct_blacklisted(attributes::struct_def const& struct_)
{
   return is_struct_blacklisted(name_helpers::struct_full_eolian_name(struct_));
}

inline bool is_struct_blacklisted(attributes::regular_type_def const& struct_)
{
   return is_struct_blacklisted(name_helpers::type_full_eolian_name(struct_));
}

inline bool is_alias_blacklisted(attributes::alias_def const& alias)
{
   return name_helpers::alias_full_eolian_name(alias) == "Eina.Error";
}

inline bool is_property_blacklisted(std::string const& name)
{
    return name == "Efl.Input.Key.Key"
        || name == "Efl.Input.Hold.Hold"
        || name == "Efl.Text.Text";
}

template<typename Context>
inline bool is_property_blacklisted(attributes::property_def const& property, Context context)
{
    auto name = name_helpers::klass_full_concrete_or_interface_name(property.klass) + "." + name_helpers::property_managed_name(property);

    if (property.getter.is_engaged())
      if (is_function_blacklisted(*property.getter, context))
        return true;
    if (property.setter.is_engaged())
      if (is_function_blacklisted(*property.setter, context))
        return true;
    return is_property_blacklisted(name);
}

}

}

#endif
