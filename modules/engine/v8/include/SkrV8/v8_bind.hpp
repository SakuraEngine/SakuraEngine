#pragma once
#include <SkrRTTR/script/scriptble_object.hpp>
#include <SkrRTTR/script/script_binder.hpp>
#include <SkrRTTR/script/script_tools.hpp>
#include <SkrRTTR/script/stack_proxy.hpp>

// v8 includes
#include <v8-isolate.h>
#include <v8-platform.h>
#include <v8-primitive.h>

namespace skr
{
struct V8Isolate;

struct V8MethodMatchResult {
    bool    matched     = false;
    int32_t match_score = 0;
};
struct V8Bind {
    // primitive to v8
    static v8::Local<v8::Value>  to_v8(int32_t v);
    static v8::Local<v8::Value>  to_v8(int64_t v);
    static v8::Local<v8::Value>  to_v8(uint32_t v);
    static v8::Local<v8::Value>  to_v8(uint64_t v);
    static v8::Local<v8::Value>  to_v8(double v);
    static v8::Local<v8::Value>  to_v8(bool v);
    static v8::Local<v8::String> to_v8(StringView view, bool as_literal = false);

    // primitive to native
    static bool to_native(v8::Local<v8::Value> v8_value, int8_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, int16_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, int32_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, int64_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, uint8_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, uint16_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, uint32_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, uint64_t& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, float& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, double& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, bool& out_v);
    static bool to_native(v8::Local<v8::Value> v8_value, skr::String& out_v);

    // enum convert
    static v8::Local<v8::Value> to_v8(EnumValue value);

    // match type
    static V8MethodMatchResult match(
        ScriptBinderRoot     binder,
        v8::Local<v8::Value> v8_value
    );
    static V8MethodMatchResult match(
        const ScriptBinderParam& binder,
        v8::Local<v8::Value>     v8_value
    );
    static V8MethodMatchResult match(
        const ScriptBinderReturn& binder,
        v8::Local<v8::Value>      v8_value
    );
    static V8MethodMatchResult match(
        const ScriptBinderField& binder,
        v8::Local<v8::Value>     v8_value
    );
    static V8MethodMatchResult match(
        const ScriptBinderStaticField& binder,
        v8::Local<v8::Value>           v8_value
    );
    static V8MethodMatchResult match(
        const Vector<ScriptBinderParam>&               param_binders,
        uint32_t                                       solved_param_count,
        const ::v8::FunctionCallbackInfo<::v8::Value>& v8_stack
    );

    // namespace helper
    static v8::Local<v8::Value> export_namespace_node(
        const ScriptNamespaceNode* node,
        V8Isolate*                 skr_isolate
    );

private:
    // match helper
    static V8MethodMatchResult _match_primitive(const ScriptBinderPrimitive& binder, v8::Local<v8::Value> v8_value);
    static V8MethodMatchResult _match_mapping(const ScriptBinderMapping& binder, v8::Local<v8::Value> v8_value);
    static V8MethodMatchResult _match_object(const ScriptBinderObject& binder, v8::Local<v8::Value> v8_value);
    static V8MethodMatchResult _match_value(const ScriptBinderValue& binder, v8::Local<v8::Value> v8_value);
};
} // namespace skr

// to v8
namespace skr
{
inline v8::Local<v8::Value> V8Bind::to_v8(int32_t v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::Integer::New(isolate, v);
}
inline v8::Local<v8::Value> V8Bind::to_v8(int64_t v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::BigInt::New(isolate, v);
}
inline v8::Local<v8::Value> V8Bind::to_v8(uint32_t v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::Integer::NewFromUnsigned(isolate, v);
}
inline v8::Local<v8::Value> V8Bind::to_v8(uint64_t v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::BigInt::NewFromUnsigned(isolate, v);
}
inline v8::Local<v8::Value> V8Bind::to_v8(double v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::Number::New(isolate, v);
}
inline v8::Local<v8::Value> V8Bind::to_v8(bool v)
{
    auto isolate = v8::Isolate::GetCurrent();
    return v8::Boolean::New(isolate, v);
}
inline v8::Local<v8::String> V8Bind::to_v8(StringView view, bool as_literal)
{
    auto isolate = v8::Isolate::GetCurrent();
    // clang-format off
    return v8::String::NewFromUtf8(
        isolate,
        view.data_raw(),
        as_literal ? v8::NewStringType::kInternalized : v8::NewStringType::kNormal,
        view.length_buffer()
    ).ToLocalChecked();
    // clang-format on
}
} // namespace skr

// to native
namespace skr
{
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, int8_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsInt32())
    {
        out_v = (int8_t)v8_value->Int32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, int16_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsInt32())
    {
        out_v = (int16_t)v8_value->Int32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, int32_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsInt32())
    {
        out_v = (int32_t)v8_value->Int32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, int64_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsBigInt())
    {
        out_v = (int64_t)v8_value->ToBigInt(context).ToLocalChecked()->Int64Value();
        return true;
    }
    else if (v8_value->IsInt32())
    {
        out_v = (int64_t)v8_value->Int32Value(context).ToChecked();
        return true;
    }
    else if (v8_value->IsUint32())
    {
        out_v = (int64_t)v8_value->Uint32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, uint8_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsUint32())
    {
        out_v = (uint8_t)v8_value->Uint32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, uint16_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsUint32())
    {
        out_v = (uint16_t)v8_value->Uint32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, uint32_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsUint32())
    {
        out_v = (uint32_t)v8_value->Uint32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, uint64_t& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsBigInt())
    {
        out_v = (uint64_t)v8_value->ToBigInt(context).ToLocalChecked()->Uint64Value();
        return true;
    }
    else if (v8_value->IsUint32())
    {
        out_v = (uint64_t)v8_value->Uint32Value(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, float& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsNumber())
    {
        out_v = (float)v8_value->NumberValue(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, double& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    if (v8_value->IsNumber())
    {
        out_v = (double)v8_value->NumberValue(context).ToChecked();
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, bool& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();

    if (v8_value->IsBoolean())
    {
        out_v = v8_value->BooleanValue(isolate);
        return true;
    }
    return false;
}
inline bool V8Bind::to_native(v8::Local<v8::Value> v8_value, skr::String& out_v)
{
    auto isolate = v8::Isolate::GetCurrent();

    if (v8_value->IsString())
    {
        out_v = skr::String::From(*v8::String::Utf8Value(isolate, v8_value));
        return true;
    }
    return false;
}
} // namespace skr

// enum convert
namespace skr
{
inline v8::Local<v8::Value> V8Bind::to_v8(EnumValue enum_value)
{
    auto isolate = v8::Isolate::GetCurrent();

    switch (enum_value.underlying_type())
    {
    case EEnumUnderlyingType::INT8:
    case EEnumUnderlyingType::INT16:
    case EEnumUnderlyingType::INT32: {
        int32_t value;
        SKR_ASSERT(enum_value.cast_to(value));
        return v8::Integer::New(isolate, value);
        break;
    }
    case EEnumUnderlyingType::UINT8:
    case EEnumUnderlyingType::UINT16:
    case EEnumUnderlyingType::UINT32: {
        uint32_t value;
        SKR_ASSERT(enum_value.cast_to(value));
        return v8::Integer::New(isolate, value);
        break;
    }
    case EEnumUnderlyingType::INT64: {
        int64_t value;
        SKR_ASSERT(enum_value.cast_to(value));
        return v8::BigInt::New(isolate, value);
        break;
    }
    case EEnumUnderlyingType::UINT64: {
        uint64_t value;
        SKR_ASSERT(enum_value.cast_to(value));
        return v8::BigInt::New(isolate, value);
        break;
    }
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
} // namespace skr