#include <SkrV8/bind_template/v8_bind_template.hpp>
#include <SkrV8/v8_bind.hpp>
#include <SkrV8/v8_bind_proxy.hpp>
#include <SkrV8/v8_isolate.hpp>
#include <SkrV8/bind_template/v8bt_primitive.hpp>

// v8 includes
#include <libplatform/libplatform.h>
#include <v8-initialization.h>
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-container.h>

// helper
namespace skr::helper
{
inline static bool match_params(
    Span<const V8BTDataParam> params,
    uint32_t solved_param_count,
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
)
{
    // check param count
    if (solved_param_count != v8_stack.Length()) { return {}; }

    // check param type
    for (size_t i = 0; i < params.size(); i++)
    {
        auto& param_data = params[i];
        auto v8_arg = v8_stack[i];

        // skip pure out param
        if (param_data.inout_flag == ERTTRParamFlag::Out) { continue; }

        // do match
        bool matched = param_data.bind_tp->match_param(v8_arg);
        if (!matched) { return false; }
    }

    return true;
}
inline static v8::Local<v8::Value> read_return(
    DynamicStack& stack,
    Span<const V8BTDataParam> params,
    const V8BTDataReturn& return_bind_tp,
    uint32_t solved_return_count
)
{
    auto* isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (solved_return_count == 1)
    { // return single value
        if (return_bind_tp.is_void)
        { // read from out param
            for (const auto& param_bind_tp : params)
            {
                if (param_bind_tp.appare_in_return)
                {
                    return param_bind_tp.bind_tp->read_return_from_out_param(
                        stack,
                        param_bind_tp
                    );
                }
            }
        }
        else
        { // read return value
            if (stack.is_return_stored())
            {
                return return_bind_tp.bind_tp->read_return_native(
                    stack,
                    return_bind_tp
                );
            }
        }
    }
    else
    { // return param array
        v8::Local<v8::Array> out_array = v8::Array::New(isolate);
        uint32_t cur_index = 0;

        // try read return value
        if (!return_bind_tp.is_void)
        {
            if (stack.is_return_stored())
            {
                v8::Local<v8::Value> out_value = return_bind_tp.bind_tp->read_return_native(
                    stack,
                    return_bind_tp
                );
                out_array->Set(context, cur_index, out_value).Check();
                ++cur_index;
            }
        }

        // read return value from out param
        for (const auto& param_bind_tp : params)
        {
            if (param_bind_tp.appare_in_return)
            {
                // clang-format off
                out_array->Set(
                    context, 
                    cur_index, 
                param_bind_tp.bind_tp->read_return_from_out_param(
                    stack, 
                    param_bind_tp
                )).Check();
                // clang-format on
                ++cur_index;
            }
        }

        return out_array;
    }

    return {};
}
} // namespace skr::helper

namespace skr
{
//===============================V8BTDataField===============================
void V8BTDataField::setup(
    V8Isolate* isolate,
    const RTTRFieldData* field_data,
    const RTTRType* owner
)
{
    {
        auto type_sig_view = field_data->type.view();
        type_sig_view.jump_modifier();
        bind_tp = isolate->solve_bind_tp(type_sig_view);
    }
    field_owner = owner;
    rttr_data = field_data;
    modifiers.solve(field_data->type.view());
    bind_tp->check_field(*this, errors);
}

//===============================V8BTDataStaticField===============================
void V8BTDataStaticField::setup(
    V8Isolate* isolate,
    const RTTRStaticFieldData* field_data,
    const RTTRType* owner
)
{
    {
        auto type_sig_view = field_data->type.view();
        type_sig_view.jump_modifier();
        bind_tp = isolate->solve_bind_tp(type_sig_view);
    }
    field_owner = owner;
    rttr_data = field_data;
    modifiers.solve(field_data->type.view());
    bind_tp->check_static_field(*this, errors);
}

//===============================V8BTDataParam===============================
void V8BTDataParam::setup(
    V8Isolate* isolate,
    const RTTRParamData* param_data,
    V8ErrorCache& errors
)
{
    {
        auto type_sig_view = param_data->type.view();
        type_sig_view.jump_modifier();
        bind_tp = isolate->solve_bind_tp(type_sig_view);
    }
    rttr_data = param_data;
    index = param_data->index;

    // solve modifier
    {
        auto type_sig_view = param_data->type.view();
        modifiers.solve(type_sig_view);
        bool has_param_out = flag_all(param_data->flag, ERTTRParamFlag::Out);
        bool has_param_in = flag_all(param_data->flag, ERTTRParamFlag::In);

        // check pointer level
        if (type_sig_view.decayed_pointer_level() > 1)
        {
            errors.error(u8"param {}: pointer level greater than 1", index);
            return;
        }

        // solve inout flag
        if (!has_param_in && !has_param_out)
        { // use param default inout flag
            if (modifiers.is_any_ref() && !modifiers.is_const)
            {
                inout_flag |= ERTTRParamFlag::In | ERTTRParamFlag::Out;
            }
        }
        else
        {
            if (has_param_in)
            {
                inout_flag |= ERTTRParamFlag::In;
            }
            if (has_param_out)
            {
                inout_flag |= ERTTRParamFlag::Out;
                if (!modifiers.is_any_ref() || modifiers.is_const)
                {
                    errors.error(u8"param {}: only T& can be out param", index);
                }
            }
        }
    }

    bind_tp->solve_invoke_behaviour(
        *this,
        appare_in_return,
        appare_in_param
    );
    bind_tp->check_param(*this, errors);
}

void V8BTDataParam::setup(
    V8Isolate* isolate,
    const StackProxy* proxy,
    int32_t index,
    V8ErrorCache& errors
)
{
    RTTRParamData param_data = {};
    param_data.type = proxy->signature;
    param_data.index = index;
    format_to(param_data.name, u8"#{}", index);
    setup(isolate, &param_data, errors);
    rttr_data = nullptr;
}

//===============================V8BTDataParam===============================
void V8BTDataReturn::setup(
    V8Isolate* isolate,
    TypeSignatureView signature,
    V8ErrorCache& errors
)
{
    {
        auto type_sig_view = signature;
        modifiers.solve(type_sig_view);
        type_sig_view.jump_modifier();
        bind_tp = isolate->solve_bind_tp(type_sig_view);
    }
    pass_by_ref = signature.is_decayed_pointer();

    TypeSignatureTyped<void> void_sig;
    is_void = signature.equal(void_sig);

    bind_tp->check_return(*this, errors);
}

//===============================V8BTDataFunctionBase===============================
bool V8BTDataFunctionBase::call_v8_read_return(
    Span<const StackProxy> params,
    StackProxy return_value,
    v8::MaybeLocal<v8::Value> v8_return_value
) const
{
    auto* isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (return_count == 0)
    {
        return true;
    }
    else if (return_count == 1)
    {
        if (v8_return_value.IsEmpty()) { return false; }

        if (return_data.is_void)
        { // read to out param
            for (const auto& param : params_data)
            {
                if (param.appare_in_return)
                {
                    return param.bind_tp->to_native(
                        params[param.index].data,
                        v8_return_value.ToLocalChecked(),
                        true
                    );
                }
            }

            // must checked before
            SKR_UNREACHABLE_CODE()
            return false;
        }

        else
        {
            return return_data.bind_tp->to_native(
                return_value.data,
                v8_return_value.ToLocalChecked(),
                false
            );
        }
    }
    else
    {
        if (v8_return_value.IsEmpty()) { return false; }
        if (!v8_return_value.ToLocalChecked()->IsArray()) { return false; }
        v8::Local<v8::Array> v8_array = v8_return_value.ToLocalChecked().As<v8::Array>();
        if (v8_array.IsEmpty() || v8_array->Length() != return_count) { return false; }

        uint32_t cur_index = 0;

        // read return value
        bool success = true;
        if (!return_data.is_void)
        {
            success &= return_data.bind_tp->to_native(
                return_value.data,
                v8_array->Get(context, cur_index).ToLocalChecked(),
                false
            );
            ++cur_index;
        }

        // read out param
        for (const auto& param : params_data)
        {
            if (param.appare_in_return)
            {
                success &= param.bind_tp->to_native(
                    params[param.index].data,
                    v8_array->Get(context, cur_index).ToLocalChecked(),
                    true
                );
                ++cur_index;
            }
        }

        return success;
    }
}
void V8BTDataFunctionBase::call_v8_setup(
    V8Isolate* isolate,
    Span<const StackProxy> params,
    StackProxy return_value
)
{
    uint32_t param_index = 0;
    for (const auto& param : params)
    {
        params_data.add_default().ref().setup(
            isolate,
            &param,
            param_index,
            errors
        );
        ++param_index;
    }

    if (return_value)
    {
        return_data.setup(isolate, return_value.signature, errors);
    }
    else
    {
        TypeSignatureTyped<void> sig;
        return_data.setup(isolate, sig, errors);
    }

    // solve count
    return_count = return_data.is_void ? 0 : 1;
    params_count = 0;
    for (const auto& param : params_data)
    {
        if (param.appare_in_param) ++params_count;
        if (param.appare_in_return) ++return_count;
    }
}

//===============================V8BTDataMethod===============================
bool V8BTDataMethod::match_param(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
) const
{
    return helper::match_params(
        params_data,
        params_count,
        v8_stack
    );
}
void V8BTDataMethod::call(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
    void* obj,
    const RTTRType* obj_type
) const
{
    DynamicStack native_stack;

    // push param
    uint32_t v8_stack_index = 0;
    for (const auto& param : params_data)
    {
        if (param.inout_flag == ERTTRParamFlag::Out)
        { // pure out param, we will push a dummy xvalue
            param.bind_tp->push_param_native_pure_out(
                native_stack,
                param
            );
        }
        else
        {
            param.bind_tp->push_param_native(
                native_stack,
                param,
                v8_stack[v8_stack_index]
            );
            ++v8_stack_index;
        }
    }

    // get call obj address
    void* owner_address = obj_type->cast_to_base(obj_type->type_id(), obj);

    // invoke
    native_stack.return_behaviour_store();
    if (rttr_data_mixin_impl)
    { // mixin case, invoke minin impl
        rttr_data_mixin_impl->dynamic_stack_invoke(owner_address, native_stack);
    }
    else
    { // normal case
        rttr_data->dynamic_stack_invoke(owner_address, native_stack);
    }

    // read return
    auto return_value = helper::read_return(
        native_stack,
        params_data,
        return_data,
        return_count
    );
    if (!return_value.IsEmpty())
    {
        v8_stack.GetReturnValue().Set(return_value);
    }
}
void V8BTDataMethod::setup(
    V8Isolate* isolate,
    const RTTRMethodData* method_data,
    const RTTRType* owner
)
{
    method_owner = owner;
    rttr_data = method_data;

    // setup info
    return_data.setup(isolate, method_data->ret_type, errors);
    for (const auto* param : method_data->param_data)
    {
        auto& param_binder = params_data.add_default().ref();
        param_binder.setup(isolate, param, errors);
    }

    // solve count
    return_count = return_data.is_void ? 0 : 1;
    params_count = 0;
    for (const auto& param : params_data)
    {
        if (param.appare_in_param) ++params_count;
        if (param.appare_in_return) ++return_count;
    }
}

//===============================V8BTDataStaticMethod===============================
bool V8BTDataStaticMethod::match(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
) const
{
    return helper::match_params(
        params_data,
        params_count,
        v8_stack
    );
}
void V8BTDataStaticMethod::call(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
) const
{
    DynamicStack native_stack;

    // push param
    uint32_t v8_stack_index = 0;
    for (const auto& param : params_data)
    {
        if (param.inout_flag == ERTTRParamFlag::Out)
        { // pure out param, we will push a dummy xvalue
            param.bind_tp->push_param_native_pure_out(
                native_stack,
                param
            );
        }
        else
        {
            param.bind_tp->push_param_native(
                native_stack,
                param,
                v8_stack[v8_stack_index]
            );
            ++v8_stack_index;
        }
    }

    // invoke
    native_stack.return_behaviour_store();
    rttr_data->dynamic_stack_invoke(native_stack);

    // read return
    auto return_value = helper::read_return(
        native_stack,
        params_data,
        return_data,
        return_count
    );
    if (!return_value.IsEmpty())
    {
        v8_stack.GetReturnValue().Set(return_value);
    }
}
void V8BTDataStaticMethod::setup(
    V8Isolate* isolate,
    const RTTRStaticMethodData* method_data,
    const RTTRType* owner
)
{
    method_owner = owner;
    rttr_data = method_data;
    return_data.setup(isolate, method_data->ret_type, errors);
    for (const auto* param : method_data->param_data)
    {
        auto& param_binder = params_data.add_default().ref();
        param_binder.setup(isolate, param, errors);
    }

    // solve count
    return_count = return_data.is_void ? 0 : 1;
    params_count = 0;
    for (const auto& param : params_data)
    {
        if (param.appare_in_param) ++params_count;
        if (param.appare_in_return) ++return_count;
    }
}

//===============================V8BTDataProperty===============================
void V8BTDataProperty::setup_getter(
    V8Isolate* isolate,
    const RTTRMethodData* method_data,
    const RTTRType* owner
)
{
    getter.setup(isolate, method_data, owner);

    // check param count
    if (getter.params_count != 0)
    {
        errors.error(
            u8"getter param count must be 0, but got {}",
            getter.params_count
        );
    }
    // check return type
    if (getter.return_count != 1)
    {
        errors.error(
            u8"getter return count must be 1, but got {}",
            getter.return_count
        );
    }
}
void V8BTDataProperty::setup_setter(
    V8Isolate* isolate,
    const RTTRMethodData* method_data,
    const RTTRType* owner
)
{
    setter.setup(isolate, method_data, owner);

    // check param count
    if (setter.params_count != 1)
    {
        errors.error(
            u8"setter param count must be 1, but got {}",
            setter.params_count
        );
    }
    // check return type
    if (setter.return_count != 0)
    {
        errors.error(
            u8"setter return count must be 0, but got {}",
            setter.return_count
        );
    }
}
void V8BTDataProperty::check_conflict()
{
    auto getter_tp = getter_bind_tp();
    auto setter_tp = setter_bind_tp();

    if (getter_tp && setter_tp)
    {
        bool prop_mismatch = false;
        if (getter_tp != setter_tp)
        { // maybe mismatch
            if (
                getter_tp->is<V8BTPrimitive>() &&
                setter_tp->is<V8BTPrimitive>()
            )
            {
                bool getter_is_str = getter_tp->as<V8BTPrimitive>()->is_string();
                bool setter_is_str = setter_tp->as<V8BTPrimitive>()->is_string();

                if (getter_is_str && setter_is_str)
                { // optimize for string
                }
                else
                {
                    prop_mismatch = true;
                }
            }
            else
            {
                prop_mismatch = true;
            }
        }
        if (prop_mismatch)
        {
            errors.error(
                u8"getter and setter type mismatch, getter '{}', setter '{}'",
                getter.rttr_data->name,
                setter.rttr_data->name
            );
        }
    }
}

//===============================V8BTDataStaticProperty===============================
void V8BTDataStaticProperty::setup_getter(
    V8Isolate* isolate,
    const RTTRStaticMethodData* method_data,
    const RTTRType* owner
)
{
    getter.setup(isolate, method_data, owner);

    // check param count
    if (getter.params_count != 0)
    {
        errors.error(
            u8"getter param count must be 0, but got {}",
            getter.params_count
        );
    }
    // check return type
    if (getter.return_count != 1)
    {
        errors.error(
            u8"getter return count must be 1, but got {}",
            getter.return_count
        );
    }
}
void V8BTDataStaticProperty::setup_setter(
    V8Isolate* isolate,
    const RTTRStaticMethodData* method_data,
    const RTTRType* owner
)
{
    setter.setup(isolate, method_data, owner);

    // check param count
    if (setter.params_count != 1)
    {
        errors.error(
            u8"setter param count must be 1, but got {}",
            setter.params_count
        );
    }
    // check return type
    if (setter.return_count != 0)
    {
        errors.error(
            u8"setter return count must be 0, but got {}",
            setter.return_count
        );
    }
}
void V8BTDataStaticProperty::check_conflict()
{
    auto getter_tp = getter_bind_tp();
    auto setter_tp = setter_bind_tp();

    if (getter_tp && setter_tp)
    {
        bool prop_mismatch = false;
        if (getter_tp != setter_tp)
        { // maybe mismatch
            if (
                getter_tp->is<V8BTPrimitive>() &&
                setter_tp->is<V8BTPrimitive>()
            )
            {
                bool getter_is_str = getter_tp->as<V8BTPrimitive>()->is_string();
                bool setter_is_str = setter_tp->as<V8BTPrimitive>()->is_string();

                if (getter_is_str && setter_is_str)
                { // optimize for string
                }
                else
                {
                    prop_mismatch = true;
                }
            }
            else
            {
                prop_mismatch = true;
            }
        }
        if (prop_mismatch)
        {
            errors.error(
                u8"getter and setter type mismatch, getter '{}', setter '{}'",
                getter.rttr_data->name,
                setter.rttr_data->name
            );
        }
    }
}

//===============================V8BTDataCtor===============================
bool V8BTDataCtor::match(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
) const
{
    return helper::match_params(
        params_data,
        params_data.size(),
        v8_stack
    );
}
void V8BTDataCtor::call(
    const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack,
    void* obj
) const
{
    DynamicStack native_stack;

    // push param
    uint32_t v8_stack_index = 0;
    for (const auto& param : params_data)
    {
        param.bind_tp->push_param_native(
            native_stack,
            param,
            v8_stack[v8_stack_index]
        );
        ++v8_stack_index;
    }

    // invoke
    native_stack.return_behaviour_discard();
    rttr_data->dynamic_stack_invoke(obj, native_stack);
}
void V8BTDataCtor::setup(
    V8Isolate* isolate,
    const RTTRCtorData* ctor_data
)
{
    rttr_data = ctor_data;

    // export params
    for (const auto* param : ctor_data->param_data)
    {
        auto& param_binder = params_data.add_default().ref();
        param_binder.setup(isolate, param, errors);
    }

    // check out flag
    for (const auto& param_binder : params_data)
    {
        if (flag_all(param_binder.inout_flag, ERTTRParamFlag::Out))
        {
            errors.error(
                u8"param {}: ctor param cannot has out flag",
                param_binder.index
            );
        }
    }
}
} // namespace skr