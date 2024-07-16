#include "SkrV8/v8_bind_tools.hpp"

namespace skr::v8
{
// match tools
bool V8BindTools::match_type(::v8::Local<::v8::Value> v8_value, rttr::TypeSignatureView native_signature)
{
    if (native_signature.is_type())
    {
        // read type id
        native_signature.jump_modifier();
        GUID type_id;
        native_signature.read_type_id(type_id);

        // match primitive types
        switch (type_id.get_hash())
        {
            case rttr::type_id_of<int8_t>().get_hash():
                return v8_value->IsInt32();
            case rttr::type_id_of<int16_t>().get_hash():
                return v8_value->IsInt32();
            case rttr::type_id_of<int32_t>().get_hash():
                return v8_value->IsInt32();
            case rttr::type_id_of<uint8_t>().get_hash():
                return v8_value->IsUint32();
            case rttr::type_id_of<uint16_t>().get_hash():
                return v8_value->IsUint32();
            case rttr::type_id_of<uint32_t>().get_hash():
                return v8_value->IsUint32();
            case rttr::type_id_of<int64_t>().get_hash():
                return v8_value->IsBigInt();
            case rttr::type_id_of<uint64_t>().get_hash():
                return v8_value->IsBigInt();
            case rttr::type_id_of<float>().get_hash():
                return v8_value->IsNumber();
            case rttr::type_id_of<double>().get_hash():
                return v8_value->IsNumber();
            case rttr::type_id_of<bool>().get_hash():
                return v8_value->IsBoolean();
            default:
                break;
        }

        // TODO. match record type
        return false;
    }
    else
    {
        // TODO. match generic type
        return false;
    }
}

// convert tools
bool V8BindTools::native_to_v8_primitive(
    ::v8::Local<::v8::Context> context,
    ::v8::Isolate*             isolate,
    rttr::TypeSignatureView    signature,
    void*                      native_data,
    ::v8::Local<::v8::Value>&  out_v8_value)
{
    // only handle native type
    if (signature.is_type() && !signature.is_decayed_pointer())
    {
        // get type id
        signature.jump_modifier();
        GUID type_id;
        signature.read_type_id(type_id);

        switch (type_id.get_hash())
        {
            case rttr::type_id_of<int8_t>().get_hash():
                out_v8_value = ::v8::Integer::New(isolate, *reinterpret_cast<int8_t*>(native_data));
                return true;
            case rttr::type_id_of<int16_t>().get_hash():
                out_v8_value = ::v8::Integer::New(isolate, *reinterpret_cast<int16_t*>(native_data));
                return true;
            case rttr::type_id_of<int32_t>().get_hash():
                out_v8_value = ::v8::Integer::New(isolate, *reinterpret_cast<int32_t*>(native_data));
                return true;
            case rttr::type_id_of<uint8_t>().get_hash():
                out_v8_value = ::v8::Integer::NewFromUnsigned(isolate, *reinterpret_cast<uint8_t*>(native_data));
                return true;
            case rttr::type_id_of<uint16_t>().get_hash():
                out_v8_value = ::v8::Integer::NewFromUnsigned(isolate, *reinterpret_cast<uint16_t*>(native_data));
                return true;
            case rttr::type_id_of<uint32_t>().get_hash():
                out_v8_value = ::v8::Integer::NewFromUnsigned(isolate, *reinterpret_cast<uint32_t*>(native_data));
                return true;
            case rttr::type_id_of<int64_t>().get_hash():
                out_v8_value = ::v8::BigInt::New(isolate, *reinterpret_cast<int64_t*>(native_data));
                return true;
            case rttr::type_id_of<uint64_t>().get_hash():
                out_v8_value = ::v8::BigInt::NewFromUnsigned(isolate, *reinterpret_cast<uint64_t*>(native_data));
                return true;
            case rttr::type_id_of<float>().get_hash():
                out_v8_value = ::v8::Number::New(isolate, *reinterpret_cast<float*>(native_data));
                return true;
            case rttr::type_id_of<double>().get_hash():
                out_v8_value = ::v8::Number::New(isolate, *reinterpret_cast<double*>(native_data));
                return true;
            case rttr::type_id_of<bool>().get_hash():
                out_v8_value = ::v8::Boolean::New(isolate, *reinterpret_cast<bool*>(native_data));
                return true;
            default:
                break;
        }
    }
    return false;
}
bool V8BindTools::v8_to_native_primitive(
    ::v8::Local<::v8::Context> context,
    ::v8::Isolate*             isolate,
    rttr::TypeSignatureView    signature,
    ::v8::Local<::v8::Value>   v8_value,
    void*                      out_native_data)
{
    // only handle native type
    if (signature.is_type() && !signature.is_decayed_pointer())
    {
        // get type id
        signature.jump_modifier();
        GUID type_id;
        signature.read_type_id(type_id);

        // match primitive types
        switch (type_id.get_hash())
        {
            case rttr::type_id_of<int8_t>().get_hash():
                *reinterpret_cast<int8_t*>(out_native_data) = static_cast<int8_t>(v8_value->Int32Value(context).ToChecked());
                return true;
            case rttr::type_id_of<int16_t>().get_hash():
                *reinterpret_cast<int16_t*>(out_native_data) = static_cast<int16_t>(v8_value->Int32Value(context).ToChecked());
                return true;
            case rttr::type_id_of<int32_t>().get_hash():
                *reinterpret_cast<int32_t*>(out_native_data) = v8_value->Int32Value(context).ToChecked();
                return true;
            case rttr::type_id_of<uint8_t>().get_hash():
                *reinterpret_cast<uint8_t*>(out_native_data) = static_cast<uint8_t>(v8_value->Uint32Value(context).ToChecked());
                return true;
            case rttr::type_id_of<uint16_t>().get_hash():
                *reinterpret_cast<uint16_t*>(out_native_data) = static_cast<uint16_t>(v8_value->Uint32Value(context).ToChecked());
                return true;
            case rttr::type_id_of<uint32_t>().get_hash():
                *reinterpret_cast<uint32_t*>(out_native_data) = v8_value->Uint32Value(context).ToChecked();
                return true;
            case rttr::type_id_of<int64_t>().get_hash():
                *reinterpret_cast<int64_t*>(out_native_data) = v8_value->ToBigInt(context).ToLocalChecked()->Int64Value();
                return true;
            case rttr::type_id_of<uint64_t>().get_hash():

                *reinterpret_cast<uint64_t*>(out_native_data) = v8_value->ToBigInt(context).ToLocalChecked()->Uint64Value();
                return true;
            case rttr::type_id_of<float>().get_hash():
                *reinterpret_cast<float*>(out_native_data) = static_cast<float>(v8_value->NumberValue(context).ToChecked());
                return true;
            case rttr::type_id_of<double>().get_hash():
                *reinterpret_cast<double*>(out_native_data) = v8_value->NumberValue(context).ToChecked();
                return true;
            case rttr::type_id_of<bool>().get_hash():
                *reinterpret_cast<bool*>(out_native_data) = v8_value->BooleanValue(isolate);
                return true;
            default:
                break;
        }
    }
    return false;
}
} // namespace skr::v8