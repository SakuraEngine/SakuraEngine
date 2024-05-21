#pragma once
#include "v8-template.h"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/export/export_data.hpp"

namespace skr::v8
{
struct V8BindTools {
    template <typename Data>
    inline static bool match_param(const Data& data, const ::v8::FunctionCallbackInfo<::v8::Value>& info)
    {
        using namespace ::v8;
        auto call_length   = info.Length();
        auto native_length = data.param_data.size();

        // length > data, must be unmatched
        if (call_length > native_length)
        {
            return false;
        }

        // match default param
        for (size_t i = call_length; i < native_length; ++i)
        {
            if (!data.param_data[i].make_default)
            {
                return false;
            }
        }

        // match signature
        for (size_t i = 0; i < call_length; ++i)
        {
            Local<Value>            call_value       = info[i];
            rttr::TypeSignatureView native_signature = data.param_data[i].type.view();
            if (call_value->IsInt32())
            {
                // type category unmatched
                if (!native_signature.is_type()) return false;

                // match int8 int16 int32
                native_signature.jump_modifier();
                GUID type_id;
                native_signature.read_type_id(type_id);
                if (type_id != rttr::type_id_of<int8_t>() &&
                    type_id != rttr::type_id_of<int16_t>() &&
                    type_id != rttr::type_id_of<int32_t>())
                {
                    return false;
                }
            }
            else if (call_value->IsUint32())
            {
                // type category unmatched
                if (!native_signature.is_type()) return false;

                // match uint8 uint16 uint32
                native_signature.jump_modifier();
                GUID type_id;
                native_signature.read_type_id(type_id);
                if (type_id != rttr::type_id_of<uint8_t>() &&
                    type_id != rttr::type_id_of<uint16_t>() &&
                    type_id != rttr::type_id_of<uint32_t>())
                {
                    return false;
                }
            }
            else if (call_value->IsBigInt())
            {
                // type category unmatched
                if (!native_signature.is_type()) return false;

                // match int64 uint64
                native_signature.jump_modifier();
                GUID type_id;
                native_signature.read_type_id(type_id);
                if (type_id != rttr::type_id_of<int64_t>() &&
                    type_id != rttr::type_id_of<uint64_t>())
                {
                    return false;
                }
            }
            else if (call_value->IsNumber())
            {
                // type category unmatched
                if (!native_signature.is_type()) return false;

                // match float double
                native_signature.jump_modifier();
                GUID type_id;
                native_signature.read_type_id(type_id);
                if (type_id != rttr::type_id_of<float>() &&
                    type_id != rttr::type_id_of<double>())
                {
                    return false;
                }
            }
            else if (call_value->IsBoolean())
            {
                // type category unmatched
                if (!native_signature.is_type()) return false;

                // match bool
                native_signature.jump_modifier();
                GUID type_id;
                native_signature.read_type_id(type_id);
                if (type_id != rttr::type_id_of<bool>())
                {
                    return false;
                }
            }
            else if (call_value->IsUndefined())
            {
                // match default
                if (!data.param_data[i].make_default)
                {
                    return false;
                }
            }
        }

        return true;
    }

    struct ParamWriter {
        inline rttr::EParamHolderType operator()(void* data, uint64_t size, uint64_t align)
        {
            return {};
        }

        ::v8::Local<::v8::Value> v8_value;
        const rttr::ParamData*   param_data;
    };

    struct ParamReader {
        inline void operator()(void* data, uint64_t size, uint64_t align, rttr::EParamHolderType type)
        {
        }

        ::v8::Local<::v8::Value> v8_value;
        const rttr::ParamData*   param_data;
    };

    inline static void call_ctor(void* obj, const rttr::CtorData& data, const ::v8::FunctionCallbackInfo<::v8::Value>& info)
    {
        InlineVector<ParamWriter, 16>      writer;
        InlineVector<ParamReader, 16>      reader;
        InlineVector<rttr::ParamProxy, 16> stack_proxies;

        auto call_param_count = info.Length();
        for (size_t i = 0; i < data.param_data.size(); ++i)
        {
            const auto& param_data = data.param_data[i];
        }
    }
};
} // namespace skr::v8