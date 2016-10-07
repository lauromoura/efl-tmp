#include "evas_common_private.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */

EOLIAN void
_efl_canvas_object_key_focus_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool focus)
{

   int event_id = 0;
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   event_id = _evas_object_event_new();
   if (obj->focused == focus) goto end;
   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_FOCUS_SET, 1, focus)) return;
   if (focus)
     {
        if (obj->layer->evas->focused)
          evas_object_focus_set(obj->layer->evas->focused, 0);
	
        if (obj->layer->evas->focused) goto end;
        obj->focused = 1;
        obj->layer->evas->focused = eo_obj;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_IN, NULL, event_id, EFL_CANVAS_OBJECT_EVENT_FOCUS_IN);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, eo_obj);
     }
   else
     {
        obj->focused = 0;
        obj->layer->evas->focused = NULL;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_OUT, NULL, event_id, EFL_CANVAS_OBJECT_EVENT_FOCUS_OUT);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, eo_obj);
     }

 end:
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

EOLIAN Eina_Bool
_efl_canvas_object_key_focus_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->focused;
}

EOLIAN Evas_Object*
_evas_canvas_focus_get(Eo *eo_obj EINA_UNUSED, Evas_Public_Data *e)
{
   return e->focused;
}
