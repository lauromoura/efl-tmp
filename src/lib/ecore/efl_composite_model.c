#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Efl.h>
#include <Ecore.h>

#include "ecore_private.h"

#include "efl_composite_model.eo.h"

#define _CHILD_INDEX "child.index"

typedef struct _Efl_Composite_Model_Data Efl_Composite_Model_Data;

struct _Efl_Composite_Model_Data
{
   Efl_Model *source;

   unsigned int index;

   Eina_Bool need_index : 1;
   Eina_Bool set_index : 1;
};

static void
_efl_composite_model_efl_object_destructor(Eo *obj, Efl_Composite_Model_Data *pd)
{
   if (pd->source)
     {
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILD_ADDED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILD_REMOVED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_PROPERTIES_CHANGED, obj);

        efl_unref(pd->source);
        pd->source = NULL;
     }

   efl_destructor(efl_super(obj, EFL_COMPOSITE_MODEL_CLASS));
}

static Efl_Object *
_efl_composite_model_efl_object_finalize(Eo *obj, Efl_Composite_Model_Data *pd)
{
   if (pd->source == NULL)
     {
        ERR("Source of the composite model wasn't defined at construction time.");
        return NULL;
     }

   return obj;
}

static void
_efl_composite_model_index_set(Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd, unsigned int index)
{
   if (pd->set_index || !pd->source)
     return ;
   pd->index = index;
   pd->set_index = EINA_TRUE;
}

static unsigned int
_efl_composite_model_index_get(const Eo *obj, Efl_Composite_Model_Data *pd)
{
   Eina_Value *fetch = NULL;
   unsigned int r = 0xFFFFFFFF;

   if (pd->set_index)
     return pd->index;
   if (pd->need_index)
     return 0xFFFFFFFF;

   fetch = efl_model_property_get(obj, _CHILD_INDEX);
   if (!eina_value_uint_convert(fetch, &r))
     return 0xFFFFFFFF;
   eina_value_free(fetch);

   return r;
}

static void
_efl_composite_model_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd, Efl_Model *model)
{
   Eina_Iterator *properties;
   const char *property;

   if (pd->source != NULL)
     {
        ERR("Source already set for composite model. It can only be set once.");
        return ;
     }
   pd->source = efl_ref(model);

   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILD_ADDED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILD_REMOVED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);

   pd->need_index = EINA_TRUE;

   properties = efl_model_properties_get(pd->source);
   EINA_ITERATOR_FOREACH(properties, property)
     {
        if (!strcmp(property, _CHILD_INDEX))
          {
             pd->need_index = EINA_FALSE;
             break;
          }
     }
   eina_iterator_free(properties);
}

static Efl_Model *
_efl_composite_model_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   return pd->source;
}

static Eina_Future *
_efl_composite_model_efl_model_property_set(Eo *obj, Efl_Composite_Model_Data *pd,
                                            const char *property, Eina_Value *value)
{
   if (pd->need_index && !strcmp(property, _CHILD_INDEX))
     {
        if (pd->set_index || !pd->source)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
        if (!eina_value_uint_convert(value, &pd->index))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);
        pd->set_index = EINA_TRUE;
        return efl_loop_future_resolved(obj, eina_value_uint_init(pd->index));
     }
   return efl_model_property_set(pd->source, property, value);
}

static Eina_Value *
_efl_composite_model_efl_model_property_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd,
                                            const char *property)
{
   if (pd->need_index && !strcmp(property, _CHILD_INDEX))
     {
        if (pd->set_index)
          return eina_value_uint_new(pd->index);
        return eina_value_error_new(EAGAIN);
     }
   return efl_model_property_get(pd->source, property);
}

static Eina_Iterator *
_efl_composite_model_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   if (pd->need_index)
     {
        static const char *composite_properties[] = {
          _CHILD_INDEX
        };

        return eina_multi_iterator_new(efl_model_properties_get(pd->source),
                                       EINA_C_ARRAY_ITERATOR_NEW(composite_properties));
     }
   return efl_model_properties_get(pd->source);
}

static unsigned int
_efl_composite_model_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   return efl_model_children_count_get(pd->source);
}

typedef struct _Efl_Composite_Model_Slice_Request Efl_Composite_Model_Slice_Request;
struct _Efl_Composite_Model_Slice_Request
{
   const Efl_Class *self;
   Eo *parent;
   unsigned int start;
};

static Eina_Value
_efl_composite_model_then(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Composite_Model_Slice_Request *req = data;
   unsigned int i, len;
   Eina_Value r = EINA_VALUE_EMPTY;
   Eo *target = NULL;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        Eo *composite;

        // First set the Model to be used as a source so that we the newly object
        // can know if it needs to retain the information regarding its index.
        composite = efl_add(req->self, req->parent,
                            efl_ui_view_model_set(efl_added, target),
                            efl_composite_model_index_set(efl_added, req->start + i));

        eina_value_array_append(&r, composite);
     }

   return r;
}

static void
_efl_composite_model_clean(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Composite_Model_Slice_Request *req = data;

   efl_unref(req->parent);
   free(data);
}

static Eina_Future *
_efl_composite_model_efl_model_children_slice_get(Eo *obj,
                                                  Efl_Composite_Model_Data *pd,
                                                  unsigned int start,
                                                  unsigned int count)
{
   Efl_Composite_Model_Slice_Request *req;
   Eina_Future *f;

   f = efl_model_children_slice_get(pd->source, start, count);
   if (!f) return NULL;

   req = malloc(sizeof (Efl_Composite_Model_Slice_Request));
   if (!req) return efl_loop_future_rejected(obj, ENOMEM);

   req->self = efl_class_get(obj);
   req->parent = efl_ref(obj);
   req->start = start;

   return efl_future_then(obj, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                          .success = _efl_composite_model_then,
                          .free = _efl_composite_model_clean,
                          .data = req);
}

static Efl_Object *
_efl_composite_model_efl_model_child_add(Eo *obj EINA_UNUSED,
                                         Efl_Composite_Model_Data *pd)
{
   return efl_model_child_add(pd->source);
}

static void
_efl_composite_model_efl_model_child_del(Eo *obj EINA_UNUSED,
                                         Efl_Composite_Model_Data *pd,
                                         Efl_Object *child)
{
   efl_model_child_del(pd->source, child);
}

#include "efl_composite_model.eo.c"
