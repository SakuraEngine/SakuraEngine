#include <SkrV8/bind_template/v8bt_function_ref.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <v8-function.h>

namespace skr
{
struct V8FunctionRefStackProxy
{
    v8::Local<v8::Function> v8_function;
    V8Isolate* isolate;
    FunctionRefMemory func_ref;

    static void* custom_mapping(void* obj)
    {
        return &reinterpret_cast<V8FunctionRefStackProxy*>(obj)->func_ref;
    }
    ~V8FunctionRefStackProxy() = default;

    inline void fill_func_ref()
    {
        func_ref._kind = EFunctionRefKind::StackProxy;
        func_ref._payload = (void*)this;
        func_ref._caller =
            (void*)+[](
                        void* payload,
                        Span<const StackProxy> params,
                        StackProxy return_value
                    ) {
                V8FunctionRefStackProxy* typed_payload = reinterpret_cast<V8FunctionRefStackProxy*>(payload);
                typed_payload->isolate->invoke_v8(
                    typed_payload->v8_function,
                    typed_payload->v8_function,
                    params,
                    return_value
                );
            };
    }
};

V8BTFunctionRef* V8BTFunctionRef::Create(V8Isolate* isolate, TypeSignature func_signature)
{
    V8BTFunctionRef* result = SkrNew<V8BTFunctionRef>();
    result->_func_signature = std::move(func_signature);
    result->set_isolate(isolate);
    return result;
}

// basic info
EV8BTKind V8BTFunctionRef::kind() const
{
    SKR_ASSERT(false && "V8BTFunctionRef is not allowed for cast");
    return EV8BTKind::Generic;
}
String V8BTFunctionRef::type_name() const
{
    return u8"FunctionRef<>";
}
String V8BTFunctionRef::cpp_namespace() const
{
    return u8"skr";
}

// error process
bool V8BTFunctionRef::any_error() const
{
    return false;
}
void V8BTFunctionRef::dump_error(V8ErrorBuilderTreeStyle& builder) const
{
    SKR_UNREACHABLE_CODE();
}

// convert helper
v8::Local<v8::Value> V8BTFunctionRef::to_v8(
    void* native_data
) const
{
    SKR_ASSERT(false && "pass FunctionRef to v8 is not allowed");
    return {};
}
bool V8BTFunctionRef::to_native(
    void* native_data,
    v8::Local<v8::Value> v8_value,
    bool is_init
) const
{
    SKR_ASSERT(false && "pass FunctionRef to native is not allowed");
    return false;
}

// invoke native api
bool V8BTFunctionRef::match_param(
    v8::Local<v8::Value> v8_param
) const
{
    return v8_param->IsFunction();
}
void V8BTFunctionRef::push_param_native(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp,
    v8::Local<v8::Value> v8_value
) const
{
    auto* proxy = stack.alloc_param<V8FunctionRefStackProxy>(
        EDynamicStackParamKind::Direct,
        nullptr,
        V8FunctionRefStackProxy::custom_mapping
    );
    proxy->isolate = isolate();
    proxy->v8_function = v8_value.As<v8::Function>();
    proxy->fill_func_ref();
}
void V8BTFunctionRef::push_param_native_pure_out(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
}
v8::Local<v8::Value> V8BTFunctionRef::read_return_native(
    DynamicStack& stack,
    const V8BTDataReturn& return_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
    return {};
}
v8::Local<v8::Value> V8BTFunctionRef::read_return_from_out_param(
    DynamicStack& stack,
    const V8BTDataParam& param_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
    return {};
}

// invoke v8 api
v8::Local<v8::Value> V8BTFunctionRef::make_param_v8(
    void* native_data,
    const V8BTDataParam& param_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
    return {};
}

// field api
v8::Local<v8::Value> V8BTFunctionRef::get_field(
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
    return {};
}
void V8BTFunctionRef::set_field(
    v8::Local<v8::Value> v8_value,
    void* obj,
    const RTTRType* obj_type,
    const V8BTDataField& field_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
}
v8::Local<v8::Value> V8BTFunctionRef::get_static_field(
    const V8BTDataStaticField& field_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
    return {};
}
void V8BTFunctionRef::set_static_field(
    v8::Local<v8::Value> v8_value,
    const V8BTDataStaticField& field_bind_tp
) const
{
    SKR_UNREACHABLE_CODE();
}

// check api
void V8BTFunctionRef::solve_invoke_behaviour(
    const V8BTDataParam& param_bind_tp,
    bool& appare_in_return,
    bool& appare_in_param
) const
{
    appare_in_return = false;
    appare_in_param = true;
}
bool V8BTFunctionRef::check_param(
    const V8BTDataParam& param_bind_tp,
    V8ErrorCache& errors
) const
{
    if (flag_any(param_bind_tp.inout_flag, ERTTRParamFlag::Out))
    {
        errors.error(u8"FunctionRef cannot have any out flag");
        return false;
    }
    return _basic_type_check(param_bind_tp.modifiers, errors);
}
bool V8BTFunctionRef::check_return(
    const V8BTDataReturn& return_bind_tp,
    V8ErrorCache& errors
) const
{
    errors.error(u8"FunctionRef cannot be used on return");
    return false;
}
bool V8BTFunctionRef::check_field(
    const V8BTDataField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    errors.error(u8"FunctionRef cannot be used on field");
    return false;
}
bool V8BTFunctionRef::check_static_field(
    const V8BTDataStaticField& field_bind_tp,
    V8ErrorCache& errors
) const
{
    errors.error(u8"FunctionRef cannot be used on field");
    return false;
}

// v8 export
bool V8BTFunctionRef::has_v8_export_obj() const
{
    return false;
}
v8::Local<v8::Value> V8BTFunctionRef::get_v8_export_obj() const
{
    SKR_UNREACHABLE_CODE();
    return {};
}
void V8BTFunctionRef::dump_ts_def(
    TSDefBuilder& builder
) const
{
    SKR_UNREACHABLE_CODE();
}
String V8BTFunctionRef::get_ts_type_name() const
{
    String ret_sig;
    String params_sig;

    // build param
    auto sig = _func_signature.view();
    uint32_t param_count;
    sig = sig.read_function_signature(param_count);

    // build return sig
    {
        auto return_sig = sig.jump_next_data();
        return_sig.jump_modifier();
        auto* return_bind_tp = isolate()->solve_bind_tp(return_sig);
        ret_sig = return_bind_tp->get_ts_type_name();
    }

    // build params sig
    for (uint32_t i = 0; i < param_count; ++i)
    {
        if (i > 0)
        {
            params_sig.append(u8", ");
        }

        // read param
        auto param_sig = sig.jump_next_data();
        param_sig.jump_modifier();
        auto* param_bind_tp = isolate()->solve_bind_tp(param_sig);

        // push param name
        params_sig.append(format(u8"${}", i));
        if (param_bind_tp->ts_is_nullable())
        {
            params_sig.append(u8"?");
        }

        params_sig.append(u8": ");

        // push param type
        params_sig.append(param_bind_tp->get_ts_type_name());
    }
    return format(
        u8"({}) => {}",
        params_sig,
        ret_sig
    );
}
bool V8BTFunctionRef::ts_is_nullable() const
{
    return true;
}

// helper
bool V8BTFunctionRef::_basic_type_check(
    const V8BTDataModifier& modifiers,
    V8ErrorCache& errors
) const
{
    if (modifiers.is_decayed_pointer())
    {
        errors.error(u8"cannot use FunctionRef as any ref type");
        return false;
    }
    return true;
}

} // namespace skr