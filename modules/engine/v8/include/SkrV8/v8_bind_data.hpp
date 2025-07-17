#pragma once
#include <SkrRTTR/script/script_binder.hpp>
#include <SkrRTTR/script/scriptble_object.hpp>

// v8 includes
#include <v8-persistent-handle.h>
#include <v8-template.h>

namespace skr
{
struct V8Isolate;
//===============================bind data===============================
struct V8BindDataMethod {
    ScriptBinderMethod                 binder;
    v8::Global<::v8::FunctionTemplate> v8_template;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataStaticMethod {
    ScriptBinderStaticMethod           binder;
    v8::Global<::v8::FunctionTemplate> v8_template;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataField {
    ScriptBinderField                  binder;
    v8::Global<::v8::FunctionTemplate> v8_template_getter;
    v8::Global<::v8::FunctionTemplate> v8_template_setter;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataStaticField {
    ScriptBinderStaticField            binder;
    v8::Global<::v8::FunctionTemplate> v8_template_getter;
    v8::Global<::v8::FunctionTemplate> v8_template_setter;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataProperty {
    ScriptBinderProperty               binder;
    v8::Global<::v8::FunctionTemplate> v8_template_getter;
    v8::Global<::v8::FunctionTemplate> v8_template_setter;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataStaticProperty {
    ScriptBinderStaticProperty         binder;
    v8::Global<::v8::FunctionTemplate> v8_template_getter;
    v8::Global<::v8::FunctionTemplate> v8_template_setter;
    // manager
    V8Isolate* manager = nullptr;
};
struct V8BindDataObject;
struct V8BindDataValue;
struct V8BindDataRecordBase {
    // v8 info
    v8::Global<::v8::FunctionTemplate> ctor_template;

    // native info
    bool                                   is_value = false;
    Map<String, V8BindDataMethod*>         methods;
    Map<String, V8BindDataField*>          fields;
    Map<String, V8BindDataStaticMethod*>   static_methods;
    Map<String, V8BindDataStaticField*>    static_fields;
    Map<String, V8BindDataProperty*>       properties;
    Map<String, V8BindDataStaticProperty*> static_properties;

    // manager
    V8Isolate* manager = nullptr;

    V8BindDataObject* as_object();
    V8BindDataValue*  as_value();

    inline ~V8BindDataRecordBase()
    {
        for (auto& pair : methods)
        {
            SkrDelete(pair.value);
        }
        for (auto& pair : fields)
        {
            SkrDelete(pair.value);
        }
        for (auto& pair : static_methods)
        {
            SkrDelete(pair.value);
        }
        for (auto& pair : static_fields)
        {
            SkrDelete(pair.value);
        }
        for (auto& pair : properties)
        {
            SkrDelete(pair.value);
        }
        for (auto& pair : static_properties)
        {
            SkrDelete(pair.value);
        }
    }
};
struct V8BindDataObject : V8BindDataRecordBase {
    inline V8BindDataObject()
    {
        is_value = false;
    }

    ScriptBinderObject* binder;
};
struct V8BindDataValue : V8BindDataRecordBase {
    inline V8BindDataValue()
    {
        is_value = true;
    }

    ScriptBinderValue* binder;
};
struct V8BindDataEnum {
    // v8 info
    v8::Global<v8::ObjectTemplate> enum_template;

    // manager
    V8Isolate* manager = nullptr;

    ScriptBinderEnum* binder;
};
inline V8BindDataObject* V8BindDataRecordBase::as_object()
{
    SKR_ASSERT(!is_value);
    return static_cast<V8BindDataObject*>(this);
}
inline V8BindDataValue* V8BindDataRecordBase::as_value()
{
    SKR_ASSERT(is_value);
    return static_cast<V8BindDataValue*>(this);
}

//===============================bind core===============================
struct V8BindCoreObject;
struct V8BindCoreValue;
struct V8BindCoreRecordBase {
    // type
    V8BindCoreObject* as_object();
    V8BindCoreValue*  as_value();

    // type
    bool is_value = false;

    // record data
    const skr::RTTRType* type = nullptr;
    void*                data = nullptr;

    // fields
    Map<void*, V8BindCoreValue*> cache_value_fields = {};

    // v8 info
    v8::Persistent<::v8::Object> v8_object   = {};
    V8Isolate*                   skr_isolate = nullptr;

    // manager
    V8Isolate* manager = nullptr;

    inline bool is_valid() const
    {
        return data != nullptr;
    }

    ~V8BindCoreRecordBase();

protected:
    void invalidate();
};
struct V8BindCoreObject : V8BindCoreRecordBase {
    inline V8BindCoreObject()
    {
        is_value = false;
    }

    // native info
    skr::ScriptbleObject* object    = nullptr;
    ScriptBinderObject*   binder    = nullptr;
    V8BindDataObject*     bind_data = nullptr;

    inline void invalidate()
    {
        V8BindCoreRecordBase::invalidate();
        object = nullptr;
    }
};
struct V8BindCoreValue : V8BindCoreRecordBase {
    inline V8BindCoreValue()
    {
        is_value = true;
    }

    // owner info
    enum class ESource
    {
        Invalid,
        Field,
        StaticField,
        Param,
        Create,
    };
    ESource               from             = ESource::Invalid;
    V8BindCoreRecordBase* from_field_owner = nullptr;
    ScriptBinderValue*    binder           = nullptr;
    V8BindDataValue*      bind_data        = nullptr;

    inline void invalidate()
    {
        V8BindCoreRecordBase::invalidate();
        from             = ESource::Invalid;
        from_field_owner = nullptr;
    }
};

inline V8BindCoreObject* V8BindCoreRecordBase::as_object()
{
    SKR_ASSERT(!is_value);
    return static_cast<V8BindCoreObject*>(this);
}
inline V8BindCoreValue* V8BindCoreRecordBase::as_value()
{
    SKR_ASSERT(is_value);
    return static_cast<V8BindCoreValue*>(this);
}
inline V8BindCoreRecordBase::~V8BindCoreRecordBase()
{
    for (auto& [field_ptr, field_core] : cache_value_fields)
    {
        field_core->invalidate();
    }
}
inline void V8BindCoreRecordBase::invalidate()
{
    for (auto& [field_ptr, field_core] : cache_value_fields)
    {
        field_core->invalidate();
    }
    data = nullptr;
}
} // namespace skr