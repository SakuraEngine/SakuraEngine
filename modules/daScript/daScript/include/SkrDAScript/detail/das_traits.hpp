#pragma once
#include "SkrDAScript/detail/cast.hpp"

namespace das
{
class Context; class SimNode; struct LineInfoArg;

RUNTIME_EXTERN_C SKR_DASCRIPT_API void ThrowCtxError(::das::Context& context, const char8_t* msg);
RUNTIME_EXTERN_C SKR_DASCRIPT_API skr::das::reg4f EvalNode(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API int32_t EvalInt(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API uint32_t EvalUInt(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API int64_t EvalInt64(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API int64_t EvalUInt64(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API int64_t EvalFloat(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API double EvalDouble(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API float EvalBool(::das::Context& context, ::das::SimNode* node);
RUNTIME_EXTERN_C SKR_DASCRIPT_API char* EvalPtr(::das::Context& context, ::das::SimNode* node);

} // namespace das

namespace skr {
namespace das {

template <typename TT>
struct EvalTT {
    static FORCEINLINE TT eval(::das::Context& context, ::das::SimNode* node)
    {
        return cast<TT>::to(::das::EvalNode(context, node));
    }
};
template <>
struct EvalTT<int32_t> {
    static FORCEINLINE int32_t eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalInt(context, node);
    }
};
template <>
struct EvalTT<uint32_t> {
    static FORCEINLINE uint32_t eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalUInt(context, node);
    }
};
template <>
struct EvalTT<int64_t> {
    static FORCEINLINE int64_t eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalInt64(context, node);
    }
};
template <>
struct EvalTT<uint64_t> {
    static FORCEINLINE uint64_t eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalUInt64(context, node);
    }
};
template <>
struct EvalTT<float> {
    static FORCEINLINE float eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalFloat(context, node);
    }
};
template <>
struct EvalTT<double> {
    static FORCEINLINE double eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalDouble(context, node);
    }
};
template <>
struct EvalTT<bool> {
    static FORCEINLINE bool eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalBool(context, node);
    }
};
template <>
struct EvalTT<char*> {
    static FORCEINLINE char* eval(::das::Context& context, ::das::SimNode* node)
    {
        return ::das::EvalPtr(context, node);
    }
};

} // namespace das
} // namespace skr
