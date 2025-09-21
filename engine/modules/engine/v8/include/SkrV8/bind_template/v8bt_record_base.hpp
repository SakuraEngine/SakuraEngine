#pragma once
#include <SkrV8/bind_template/v8_bind_template.hpp>

namespace skr
{
struct V8BTRecordBase : V8BindTemplate
{
    inline void call_dtor(void* address) const
    {
        if (_dtor)
        {
            _dtor(address);
        }
    }
    inline const V8BTDataMethod* find_method(
        StringView name
    ) const
    {
        auto found = _methods.find(name);
        return found ? &found.value() : nullptr;
    }

protected:
    void _setup(V8Isolate* isolate, const RTTRType* type);
    void _fill_template(
        v8::Local<v8::FunctionTemplate> ctor_template
    );
    bool _any_error() const;
    void _dump_error(V8ErrorBuilderTreeStyle& builder) const;
    void _dump_ts_def(TSDefBuilder& builder) const;

    static void _call_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _call_static_method(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_static_field(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _get_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);
    static void _set_static_prop(const ::v8::FunctionCallbackInfo<::v8::Value>& info);

protected:
    const RTTRType* _rttr_type = nullptr;
    bool _is_script_newable = false;
    RTTRInvokerDefaultCtor _default_ctor = nullptr;
    DtorInvoker _dtor = nullptr;

    V8BTDataCtor _ctor = {};
    Map<String, V8BTDataField> _fields = {};
    Map<String, V8BTDataStaticField> _static_fields = {};
    Map<String, V8BTDataMethod> _methods = {};
    Map<String, V8BTDataStaticMethod> _static_methods = {};
    Map<String, V8BTDataProperty> _properties = {};
    Map<String, V8BTDataStaticProperty> _static_properties = {};

    V8ErrorCache _errors = {};
};
} // namespace skr