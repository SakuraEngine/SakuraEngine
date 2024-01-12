#pragma once
#include <new> // opeartor placement new
#include "SkrDAScript/detail/das_traits.hpp"
#include "SkrDAScript/detail/smart_ptr.hpp"

namespace skr {
namespace das {
template <typename TT>
struct cast_arg {
    static FORCEINLINE TT to(::das::Context& ctx, ::das::SimNode* x) 
    {
        return EvalTT<TT>::eval(ctx,x);
    }
};

template <>
struct cast_arg<::das::Context*> {
    static FORCEINLINE ::das::Context* to(::das::Context & ctx, ::das::SimNode*) { return &ctx; }
};

template <>
struct cast_arg<reg4f> {
    static FORCEINLINE reg4f to(::das::Context & ctx, ::das::SimNode* x) 
    {
        return::das::EvalNode(ctx, x);
    }
};

template <typename TT>
struct cast_res {
    static FORCEINLINE reg4f from ( TT x, ::das::Context*) { return cast<TT>::from(x); }
};

template <typename R, typename ...Args, size_t... I>
FORCEINLINE R CallStaticFunction(R(*fn) (Args...), ::das::Context& ctx, ::das::SimNode** args, std::index_sequence<I...>) 
{
    return fn(cast_arg<Args>::to(ctx,args[I])...);
}

template <typename R, typename ...Args>
FORCEINLINE R CallStaticFunction(R(*fn)(Args...), ::das::Context& ctx, ::das::SimNode** args ) 
{
    return CallStaticFunction(fn,ctx,args, std::make_index_sequence<sizeof...(Args)>());
}

template <typename FunctionType>
struct ImplCallStaticFunction;

template <typename R, typename ...Args>
struct ImplCallStaticFunction<R (*)(Args...)> {
    static FORCEINLINE vec4f call( R (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        // The function is attempting to return result by value,
        // which type is not compatible with the current binding (missing cast<>).
        // To bind functions that return non-vec4f types by value on the stack,
        // you need to use the SimNode_ExtFuncCallAndCopyOrMove template argument
        // to copy or move the returned value.
        return cast_res<R>::from(CallStaticFunction<R,Args...>(fn,ctx,args),&ctx);
    }
};

template <typename ...Args>
struct ImplCallStaticFunction<void (*)(Args...)> {
    static FORCEINLINE vec4f call ( void (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) 
    {
        CallStaticFunction<void,Args...>(fn,ctx,args);
        return v_zero();
    }
};

template <typename T>
struct is_any_pointer {
    enum { value = std::is_pointer<T>::value || ::skr::das::is_smart_ptr<T>::value };
};

template <typename CType, bool Pointer, bool IsEnum, typename Result, typename ...Args>
struct ImplCallStaticFunctionImpl {
    static FORCEINLINE CType call( Result (*fn)(Args...), ::das::Context& context, ::das::SimNode ** ) {
        ::das::ThrowCtxError(context, u8"internal integration error");
        return CType();
    }
};

template <typename CType, typename Result, typename ...Args>   // any pointer
struct ImplCallStaticFunctionImpl<CType, true, false, Result, Args...> {
    static FORCEINLINE CType call ( Result (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args) {
        return (CType) CallStaticFunction<Result,Args...>(fn,ctx,args);
    }
};


template <typename CType, typename Result, typename ...Args>   // any enum
struct ImplCallStaticFunctionImpl<CType, false, true, Result, Args...> {
    static FORCEINLINE CType call ( Result (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        return (CType) CallStaticFunction<Result,Args...>(fn,ctx,args);
    }
};

template <typename Result, typename ...Args>
struct ImplCallStaticFunctionImpl<Result, false, false, Result, Args...> {   // no cast
    static FORCEINLINE Result call ( Result (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        return CallStaticFunction<Result,Args...>(fn,ctx,args);;
    }
};

// note: this is here because SimNode_At and such can call evalInt, while index is UInt
//  this is going to be allowed for now, since the fix will result either in duplicating SimNode_AtU or a cast node
template <typename ...Args>
struct ImplCallStaticFunctionImpl<int32_t, false, false, uint32_t, Args...> {   // int <- uint
    static FORCEINLINE int32_t call ( uint32_t (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        return CallStaticFunction<uint32_t,Args...>(fn,ctx,args);;
    }
};

template <bool Pointer, bool IsEnum, typename ...Args> // void
struct ImplCallStaticFunctionImpl<void,Pointer,IsEnum,void,Args...> {
    static FORCEINLINE void call ( void (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        CallStaticFunction<void,Args...>(fn,ctx,args);
    }
};

template <typename FuncType, typename CType>
struct ImplCallStaticFunctionImm;

template <typename R, typename ...Args, typename CType>
struct ImplCallStaticFunctionImm<R (*)(Args...),CType>
    : ImplCallStaticFunctionImpl<
        CType,
        is_any_pointer<R>::value && is_any_pointer<CType>::value,
        std::is_enum<R>::value,
        R,
        Args...> {
};

template <typename FunctionType>
struct ImplCallStaticFunctionAndCopy;

template <typename R, typename ...Args>
struct ImplCallStaticFunctionAndCopy < R (*)(Args...) > {
    static FORCEINLINE void call ( R (*fn)(Args...), ::das::Context & ctx, void * res, ::das::SimNode ** args ) {
        using result = typename std::remove_const<R>::type;
        // note: copy is closer to AOT, but placement new is correct behavior under simulation
        // *((result *)res) = CallStaticFunction<R,Args...>(fn,ctx,args);
        new (res) result ( CallStaticFunction<R,Args...>(fn,ctx,args) );
    }
};

template <typename FunctionType>
struct ImplCallStaticFunctionRef;

template <typename R, typename ...Args>
struct ImplCallStaticFunctionRef < R (*)(Args...) > {
    static FORCEINLINE char * call ( R (*fn)(Args...), ::das::Context & ctx, ::das::SimNode ** args ) {
        return (char *) & CallStaticFunction<R,Args...>(fn,ctx,args);
    }
};

}
}