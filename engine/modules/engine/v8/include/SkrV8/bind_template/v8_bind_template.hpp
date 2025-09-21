#pragma once
#include <SkrV8/v8_fwd.hpp>
#include <SkrCore/error_collector.hpp>
#include <SkrRTTR/script/scriptble_object.hpp>
#include <SkrV8/v8_tools.hpp>

// v8 includes
#include <v8-persistent-handle.h>
#include <v8-template.h>
#include <v8-external.h>

namespace skr
{
//===============================field & method data===============================
struct V8BTDataModifier
{
    bool is_pointer : 1 = false;
    bool is_ref : 1 = false;
    bool is_rvalue_ref : 1 = false;
    bool is_const : 1 = false;

    inline void solve(TypeSignatureView signature)
    {
        is_pointer = signature.is_pointer();
        is_ref = signature.is_ref();
        is_rvalue_ref = signature.is_rvalue_ref();
        is_const = signature.is_const();
    }
    inline bool is_any_ref() const
    {
        return is_ref || is_rvalue_ref;
    }
    inline bool is_decayed_pointer() const
    {
        return is_pointer || is_any_ref();
    }
};
struct V8BTDataField
{
    const RTTRType* field_owner = nullptr;
    const V8BindTemplate* bind_tp = nullptr;
    const RTTRFieldData* rttr_data = nullptr;
    V8BTDataModifier modifiers = {};
    V8ErrorCache errors = {};

    v8::Global<v8::FunctionTemplate> v8_tp_getter = {};
    v8::Global<v8::FunctionTemplate> v8_tp_setter = {};

    inline bool any_error() const { return errors.has_error(); }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const { builder.dump_errors(errors); }
    inline void* solve_address(void* obj, const RTTRType* obj_type) const
    {
        void* field_owner_address = obj_type->cast_to_base(field_owner->type_id(), obj);
        return rttr_data->get_address(field_owner_address);
    }
    void setup(
        V8Isolate* isolate,
        const RTTRFieldData* field_data,
        const RTTRType* owner
    );
};
struct V8BTDataStaticField
{
    const RTTRType* field_owner = nullptr;
    const V8BindTemplate* bind_tp = nullptr;
    const RTTRStaticFieldData* rttr_data = nullptr;
    V8BTDataModifier modifiers = {};
    V8ErrorCache errors = {};

    v8::Global<v8::FunctionTemplate> v8_tp_getter = {};
    v8::Global<v8::FunctionTemplate> v8_tp_setter = {};

    inline bool any_error() const { return errors.has_error(); }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const { builder.dump_errors(errors); }
    inline void* solve_address() const
    {
        return rttr_data->address;
    }
    void setup(
        V8Isolate* isolate,
        const RTTRStaticFieldData* field_data,
        const RTTRType* owner
    );
};
struct V8BTDataParam
{
    const V8BindTemplate* bind_tp = nullptr;
    const RTTRParamData* rttr_data = nullptr;
    uint32_t index = 0;
    V8BTDataModifier modifiers = {};
    ERTTRParamFlag inout_flag = ERTTRParamFlag::None;
    bool appare_in_return = false;
    bool appare_in_param = false;

    void setup(
        V8Isolate* isolate,
        const RTTRParamData* param_data,
        V8ErrorCache& errors
    );
    void setup(
        V8Isolate* isolate,
        const StackProxy* proxy,
        int32_t index,
        V8ErrorCache& errors
    );
};
struct V8BTDataReturn
{
    const V8BindTemplate* bind_tp = nullptr;
    bool pass_by_ref = false;
    V8BTDataModifier modifiers = {};
    bool is_void = false;

    void setup(
        V8Isolate* isolate,
        TypeSignatureView signature,
        V8ErrorCache& errors
    );
};
struct V8BTDataFunctionBase
{
    V8BTDataReturn return_data = {};
    Vector<V8BTDataParam> params_data = {};
    uint32_t params_count = 0;
    uint32_t return_count = 0;
    V8ErrorCache errors = {};
    inline bool any_error() const { return errors.has_error(); }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const { builder.dump_errors(errors); }

    bool call_v8_read_return(
        Span<const StackProxy> params,
        StackProxy return_value,
        v8::MaybeLocal<v8::Value> v8_return_value
    ) const;
    void call_v8_setup(
        V8Isolate* isolate,
        Span<const StackProxy> params,
        StackProxy return_value
    );
};
struct V8BTDataMethod : V8BTDataFunctionBase
{
    const RTTRType* method_owner = nullptr;
    const RTTRMethodData* rttr_data = nullptr;
    const RTTRMethodData* rttr_data_mixin_impl = nullptr;

    v8::Global<v8::FunctionTemplate> v8_tp = {};

    inline bool is_valid() const
    {
        return rttr_data != nullptr;
    }

    bool match_param(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    ) const;
    void call(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
        void* obj,
        const RTTRType* obj_type
    ) const;
    void setup(
        V8Isolate* isolate,
        const RTTRMethodData* method_data,
        const RTTRType* owner
    );
};
struct V8BTDataStaticMethod : V8BTDataFunctionBase
{
    const RTTRType* method_owner = nullptr;
    const RTTRStaticMethodData* rttr_data = nullptr;

    v8::Global<v8::FunctionTemplate> v8_tp = {};

    inline bool is_valid() const
    {
        return rttr_data != nullptr;
    }

    bool match(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    ) const;
    void call(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    ) const;
    void setup(
        V8Isolate* isolate,
        const RTTRStaticMethodData* method_data,
        const RTTRType* owner
    );
};
struct V8BTDataProperty
{
    V8BTDataMethod getter = {};
    V8BTDataMethod setter = {};
    V8ErrorCache errors = {};

    inline bool any_error() const
    {
        return errors.has_error() ||
            (getter.is_valid() && getter.any_error()) ||
            (setter.is_valid() && setter.any_error());
    }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const
    {
        if (errors.has_error())
        {
            builder.dump_errors(errors);
        }

        if (getter.is_valid() && getter.any_error())
        {
            builder.write_line(u8"getter");
            builder.indent([&]() {
                getter.dump_error(builder);
            });
        }
        if (setter.is_valid() && setter.any_error())
        {
            builder.write_line(u8"setter");
            builder.indent([&]() {
                setter.dump_error(builder);
            });
        }
    }

    inline const V8BindTemplate* proxy_bind_tp() const
    {
        auto getter_tp = getter_bind_tp();
        return getter_tp ? getter_tp : setter_bind_tp();
    }
    inline const V8BindTemplate* getter_bind_tp() const
    {
        if (getter.is_valid())
        {
            if (getter.return_data.is_void)
            {
                if (getter.params_data.size() > 0)
                {
                    return getter.params_data[0].bind_tp;
                }
            }
            else
            {
                return getter.return_data.bind_tp;
            }
        }
        return nullptr;
    }
    inline const V8BindTemplate* setter_bind_tp() const
    {
        if (setter.is_valid())
        {
            if (setter.params_data.size() > 0)
            {
                return setter.params_data[0].bind_tp;
            }
        }
        return nullptr;
    }

    void setup_getter(
        V8Isolate* isolate,
        const RTTRMethodData* method_data,
        const RTTRType* owner
    );
    void setup_setter(
        V8Isolate* isolate,
        const RTTRMethodData* method_data,
        const RTTRType* owner
    );
    void check_conflict();
};
struct V8BTDataStaticProperty
{
    V8BTDataStaticMethod getter = {};
    V8BTDataStaticMethod setter = {};
    V8ErrorCache errors = {};

    inline bool any_error() const
    {
        return errors.has_error() ||
            (getter.is_valid() && getter.any_error()) ||
            (setter.is_valid() && setter.any_error());
    }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const
    {
        if (errors.has_error())
        {
            builder.dump_errors(errors);
        }
        if (getter.is_valid() && getter.any_error())
        {
            builder.write_line(u8"getter");
            builder.indent([&]() {
                getter.dump_error(builder);
            });
        }
        if (setter.is_valid() && setter.any_error())
        {
            builder.write_line(u8"setter");
            builder.indent([&]() {
                setter.dump_error(builder);
            });
        }
    }

    inline const V8BindTemplate* proxy_bind_tp() const
    {
        auto getter_tp = getter_bind_tp();
        return getter_tp ? getter_tp : setter_bind_tp();
    }
    inline const V8BindTemplate* getter_bind_tp() const
    {
        if (getter.is_valid())
        {
            if (getter.return_data.is_void)
            {
                if (getter.params_data.size() > 0)
                {
                    return getter.params_data[0].bind_tp;
                }
            }
            else
            {
                return getter.return_data.bind_tp;
            }
        }
        return nullptr;
    }
    inline const V8BindTemplate* setter_bind_tp() const
    {
        if (setter.is_valid())
        {
            if (setter.params_data.size() > 0)
            {
                return setter.params_data[0].bind_tp;
            }
        }
        return nullptr;
    }

    void setup_getter(
        V8Isolate* isolate,
        const RTTRStaticMethodData* method_data,
        const RTTRType* owner
    );
    void setup_setter(
        V8Isolate* isolate,
        const RTTRStaticMethodData* method_data,
        const RTTRType* owner
    );
    void check_conflict();
};
struct V8BTDataCtor
{
    const RTTRCtorData* rttr_data = nullptr;
    Vector<V8BTDataParam> params_data = {};
    V8ErrorCache errors = {};

    v8::Global<v8::FunctionTemplate> v8_tp = {};

    inline bool is_valid() const
    {
        return rttr_data != nullptr;
    }
    inline bool any_error() const
    {
        return errors.has_error();
    }
    inline void dump_error(V8ErrorBuilderTreeStyle& builder) const
    {
        builder.dump_errors(errors);
    }

    bool match(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    ) const;
    void call(
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
        void* obj
    ) const;
    void setup(
        V8Isolate* isolate,
        const RTTRCtorData* ctor_data
    );
};

//===============================bind template===============================
enum class EV8BTKind
{
    Primitive,
    Enum,
    Mapping,
    Value,
    Object,
    Generic,
};

struct V8BindTemplate
{
    virtual ~V8BindTemplate() = default;

    // getter & setter
    inline V8Isolate* isolate() const { return _isolate; }
    inline void set_isolate(V8Isolate* isolate) { _isolate = isolate; }

    // basic info
    virtual EV8BTKind kind() const = 0;
    virtual String type_name() const = 0;
    virtual String cpp_namespace() const = 0;

    // error process
    virtual bool any_error() const = 0;
    virtual void dump_error(V8ErrorBuilderTreeStyle& builder) const = 0;

    // convert api
    virtual v8::Local<v8::Value> to_v8(
        void* native_data
    ) const = 0;
    virtual bool to_native(
        void* native_data,
        v8::Local<v8::Value> v8_value,
        bool is_init
    ) const = 0;

    // invoke native api
    virtual bool match_param(
        v8::Local<v8::Value> v8_param
    ) const = 0;
    virtual void push_param_native(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp,
        v8::Local<v8::Value> v8_value
    ) const = 0;
    virtual void push_param_native_pure_out(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp
    ) const = 0;
    virtual v8::Local<v8::Value> read_return_native(
        DynamicStack& stack,
        const V8BTDataReturn& return_bind_tp
    ) const = 0;
    virtual v8::Local<v8::Value> read_return_from_out_param(
        DynamicStack& stack,
        const V8BTDataParam& param_bind_tp
    ) const = 0;

    // invoke v8 api
    virtual v8::Local<v8::Value> make_param_v8(
        void* native_data,
        const V8BTDataParam& param_bind_tp
    ) const = 0;

    // field api
    virtual v8::Local<v8::Value> get_field(
        void* obj,
        const RTTRType* obj_type,
        const V8BTDataField& field_bind_tp
    ) const = 0;
    virtual void set_field(
        v8::Local<v8::Value> v8_value,
        void* obj,
        const RTTRType* obj_type,
        const V8BTDataField& field_bind_tp
    ) const = 0;
    virtual v8::Local<v8::Value> get_static_field(
        const V8BTDataStaticField& field_bind_tp
    ) const = 0;
    virtual void set_static_field(
        v8::Local<v8::Value> v8_value,
        const V8BTDataStaticField& field_bind_tp
    ) const = 0;

    // check api
    virtual void solve_invoke_behaviour(
        const V8BTDataParam& param_bind_tp,
        bool& appare_in_return,
        bool& appare_in_param
    ) const = 0;
    virtual bool check_param(
        const V8BTDataParam& param_bind_tp,
        V8ErrorCache& errors
    ) const = 0;
    virtual bool check_return(
        const V8BTDataReturn& return_bind_tp,
        V8ErrorCache& errors
    ) const = 0;
    virtual bool check_field(
        const V8BTDataField& field_bind_tp,
        V8ErrorCache& errors
    ) const = 0;
    virtual bool check_static_field(
        const V8BTDataStaticField& field_bind_tp,
        V8ErrorCache& errors
    ) const = 0;

    // v8 export
    virtual bool has_v8_export_obj() const = 0;
    virtual v8::Local<v8::Value> get_v8_export_obj() const = 0;
    virtual void dump_ts_def(
        TSDefBuilder& builder
    ) const = 0;
    virtual String get_ts_type_name() const = 0;
    virtual bool ts_is_nullable() const = 0;

    // cast helper
    template <typename T>
    inline bool is() const
    {
        return kind() == T::kKind;
    }
    template <typename T>
    inline T* as()
    {
        SKR_ASSERT(is<T>());
        return static_cast<T*>(this);
    }
    template <typename T>
    inline const T* as() const
    {
        SKR_ASSERT(is<T>());
        return static_cast<const T*>(this);
    }

private:
    V8Isolate* _isolate = nullptr;
};

// help functions
template <typename T>
inline T* get_bind_proxy(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    auto self = info.This();
    void* external_data = self->GetInternalField(0).As<External>()->Value();
    return reinterpret_cast<T*>(external_data);
}
template <typename T>
inline T* get_bind_proxy(const v8::Local<v8::Object>& obj)
{
    using namespace ::v8;

    void* external_data = obj->GetInternalField(0).As<External>()->Value();
    return reinterpret_cast<T*>(external_data);
}
template <typename T>
inline T* get_bind_data(const ::v8::FunctionCallbackInfo<::v8::Value>& info)
{
    using namespace ::v8;

    void* external_data = info.Data().As<External>()->Value();
    return reinterpret_cast<T*>(external_data);
}
} // namespace skr