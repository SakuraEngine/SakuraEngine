#pragma once
#include "v8-template.h"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/export/export_data.hpp"

namespace skr::v8
{
struct SKR_V8_API V8BindTools {
    // match helper
    static bool match_type(::v8::Local<::v8::Value> v8_value, rttr::TypeSignatureView native_signature);
    template <typename Data>
    static bool match_params(const Data& data, const ::v8::FunctionCallbackInfo<::v8::Value>& info);

    // convert helper
    // TODO. rvalue 和 move 语义在脚本中不予支持，因为脚本没有 move 的概念，一旦发生 move 容易产生 UB
    static bool native_to_v8_primitive(
        ::v8::Local<::v8::Context> context,
        ::v8::Isolate*             isolate,
        rttr::TypeSignatureView    signature,
        void*                      native_data,
        ::v8::Local<::v8::Value>&  out_v8_value);
    static bool v8_to_native_primitive(
        ::v8::Local<::v8::Context> context,
        ::v8::Isolate*             isolate,
        rttr::TypeSignatureView    signature,
        ::v8::Local<::v8::Value>   v8_value,
        void*                      out_native_data);

    // call helper
    struct ParamWriter {
        inline rttr::EParamHolderType operator()(void* data, uint64_t size, uint64_t align)
        {
            auto native_signature = param_data->type.view();

            if (v8_value.IsEmpty() || v8_value->IsUndefined())
            {
                // use default value
                SKR_ASSERT(param_data->make_default);
                param_data->make_default(data);
                return native_signature.is_decayed_pointer() ? rttr::EParamHolderType::xvalue : rttr::EParamHolderType::value;
            }
            else
            {
                if (native_signature.is_type())
                {
                    // export primitive type
                    // primitive type can only export as value
                    auto pure_type_signature = native_signature;
                    pure_type_signature.jump_modifier();
                    if (v8_to_native_primitive(
                            v8_context,
                            v8_isolate,
                            pure_type_signature,
                            v8_value,
                            data))
                    {
                        return native_signature.is_decayed_pointer() ? rttr::EParamHolderType::xvalue : rttr::EParamHolderType::value;
                    }

                    // TODO. export record type
                }
                else
                {
                    // TODO. export generic type
                }
            }
            return {};
        }

        ::v8::Local<::v8::Value>   v8_value;
        ::v8::Local<::v8::Context> v8_context;
        ::v8::Isolate*             v8_isolate;
        const rttr::ParamData*     param_data;
    };
    struct ParamReader {
        inline void operator()(void* data, uint64_t size, uint64_t align, rttr::EParamHolderType type)
        {
            // TODO. handle param with out flag
        }

        ::v8::Local<::v8::Value> v8_value;
        const rttr::ParamData*   param_data;
    };
    struct RetReader {
        inline void operator()(void* data, uint64_t size, uint64_t align)
        {
            if (ret_type.is_type())
            {
                ::v8::Local<::v8::Value> out_value;

                if (native_to_v8_primitive(
                        v8_context,
                        v8_isolate,
                        ret_type,
                        data,
                        out_value))
                {
                    callback_info->GetReturnValue().Set(out_value);
                }

                // TODO. export record type
            }
            else
            {
                // TODO. export generic type
            }
        }

        const ::v8::FunctionCallbackInfo<::v8::Value>* callback_info;
        const rttr::TypeSignatureView                  ret_type;
        ::v8::Local<::v8::Context>                     v8_context;
        ::v8::Isolate*                                 v8_isolate;
    };

    inline static void call_ctor(
        void*                                          obj,
        const rttr::CtorData&                          data,
        const ::v8::FunctionCallbackInfo<::v8::Value>& info,
        ::v8::Local<::v8::Context>                     context,
        ::v8::Isolate*                                 isolate)
    {
        InlineVector<ParamWriter, 16>      writer;
        InlineVector<ParamReader, 16>      reader;
        InlineVector<rttr::ParamProxy, 16> stack_proxies;

        // combine proxy
        auto call_param_count = info.Length();
        for (size_t i = 0; i < data.param_data.size(); ++i)
        {
            const auto& param_data = data.param_data[i];

            // combine writer
            ParamWriter& local_writer = writer.emplace().ref();
            local_writer.param_data   = &param_data;
            local_writer.v8_context   = context;
            local_writer.v8_isolate   = isolate;
            local_writer.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine reader
            ParamReader& local_reader = reader.emplace().ref();
            local_reader.param_data   = &param_data;
            local_reader.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine stack proxy
            rttr::ParamProxy& local_proxy = stack_proxies.emplace().ref();
            local_proxy.reader            = local_reader;
            local_proxy.writer            = local_writer;
        }

        // call
        rttr::StackProxy stack_proxy{
            nullptr,
            stack_proxies
        };
        data.stack_proxy_invoke(obj, stack_proxy);
    }
    inline static void call_method(
        void*                                          obj,
        const rttr::MethodData&                        data,
        const ::v8::FunctionCallbackInfo<::v8::Value>& info,
        ::v8::Local<::v8::Context>                     context,
        ::v8::Isolate*                                 isolate)
    {
        InlineVector<ParamWriter, 16>      writer;
        InlineVector<ParamReader, 16>      reader;
        InlineVector<rttr::ParamProxy, 16> stack_proxies;

        // combine proxy
        auto call_param_count = info.Length();
        for (size_t i = 0; i < data.param_data.size(); ++i)
        {
            const auto& param_data = data.param_data[i];

            // combine writer
            ParamWriter& local_writer = writer.emplace().ref();
            local_writer.param_data   = &param_data;
            local_writer.v8_context   = context;
            local_writer.v8_isolate   = isolate;
            local_writer.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine reader
            ParamReader& local_reader = reader.emplace().ref();
            local_reader.param_data   = &param_data;
            local_reader.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine stack proxy
            rttr::ParamProxy local_proxy = stack_proxies.emplace().ref();
            local_proxy.reader           = local_reader;
            local_proxy.writer           = local_writer;
        }

        // call
        RetReader ret_reader = {
            &info,
            data.ret_type,
            context,
            isolate
        };
        rttr::StackProxy stack_proxy{
            ret_reader,
            stack_proxies
        };
        data.stack_proxy_invoke(obj, stack_proxy);
    }
    inline static void call_static_method(
        const rttr::StaticMethodData&                  data,
        const ::v8::FunctionCallbackInfo<::v8::Value>& info,
        ::v8::Local<::v8::Context>                     context,
        ::v8::Isolate*                                 isolate)
    {
        InlineVector<ParamWriter, 16>      writer;
        InlineVector<ParamReader, 16>      reader;
        InlineVector<rttr::ParamProxy, 16> stack_proxies;

        // combine proxy
        auto call_param_count = info.Length();
        for (size_t i = 0; i < data.param_data.size(); ++i)
        {
            const auto& param_data = data.param_data[i];

            // combine writer
            ParamWriter& local_writer = writer.emplace().ref();
            local_writer.param_data   = &param_data;
            local_writer.v8_context   = context;
            local_writer.v8_isolate   = isolate;
            local_writer.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine reader
            ParamReader& local_reader = reader.emplace().ref();
            local_reader.param_data   = &param_data;
            local_reader.v8_value     = i < call_param_count ? info[i] : ::v8::Local<::v8::Value>{};

            // combine stack proxy
            rttr::ParamProxy local_proxy = stack_proxies.emplace().ref();
            local_proxy.reader           = local_reader;
            local_proxy.writer           = local_writer;
        }

        // call
        RetReader ret_reader = {
            &info,
            data.ret_type,
            context,
            isolate
        };
        rttr::StackProxy stack_proxy{
            ret_reader,
            stack_proxies
        };
        data.stack_proxy_invoke(stack_proxy);
    }
};
} // namespace skr::v8

// inline impl
namespace skr::v8
{
template <typename Data>
inline bool V8BindTools::match_params(const Data& data, const ::v8::FunctionCallbackInfo<::v8::Value>& info)
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

        // use default value
        if (call_value->IsUndefined())
        {
            if (!data.param_data[i].make_default)
            {
                return false;
            }
        }

        // match type
        if (!match_type(call_value, native_signature))
        {
            return false;
        }
    }

    return true;
}
} // namespace skr::v8