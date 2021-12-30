#pragma once
#include <EASTL/tuple.h>

/* clang-format off */
// utilx
namespace swa
{
namespace m3
{
namespace detail
{
    typedef uint64_t *stack_type;
    typedef void *mem_type;
    template<typename T, typename...> struct first_type { typedef T type; };
    typedef const void *(*m3_api_raw_fn)(IM3Runtime, uint64_t *, void *);
    template<typename T>
    void arg_from_stack(T &dest, stack_type &_sp, mem_type mem) {
        m3ApiGetArg(T, tmp);
        dest = tmp;
    }
    template<typename T>
    void arg_from_stack(T* &dest, stack_type &_sp, mem_type _mem) {
        m3ApiGetArgMem(T*, tmp);
        dest = tmp;
    };
    template<typename T>
    void arg_from_stack(const T* &dest, stack_type &_sp, mem_type _mem) {
        m3ApiGetArgMem(const T*, tmp);
        dest = tmp;
    };
    template<char c>
    struct m3_sig {
        static const char value = c;
    };
    template<typename T> struct m3_type_to_sig;
    template<> struct m3_type_to_sig<uint8_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<uint16_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<uint32_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<uint64_t> : m3_sig<'I'> {};
    template<> struct m3_type_to_sig<int8_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<int16_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<int32_t> : m3_sig<'i'> {};
    template<> struct m3_type_to_sig<int64_t> : m3_sig<'I'> {};
    template<> struct m3_type_to_sig<float>   : m3_sig<'f'> {};
    template<> struct m3_type_to_sig<double>  : m3_sig<'F'> {};
    template<> struct m3_type_to_sig<void>    : m3_sig<'v'> {};
    template<> struct m3_type_to_sig<void *>  : m3_sig<'*'> {};
    template<> struct m3_type_to_sig<const void *> : m3_sig<'*'> {};
    template<typename Ret, typename ... Args>
    struct m3_signature {
        constexpr static size_t n_args = sizeof...(Args);
        constexpr static const char value[n_args + 4] = {
                m3_type_to_sig<Ret>::value,
                '(',
                m3_type_to_sig<Args>::value...,
                ')',
                0
        };
    };
    template <typename ...Args>
    static void get_args_from_stack(stack_type &sp, mem_type mem, eastl::tuple<Args...> &tuple) {
        eastl::apply([&](auto &... item) {
            (arg_from_stack(item, sp, mem), ...);
        }, tuple);
    }
    template<typename Func>
    struct wrap_helper;

    template <typename Ret, typename ...Args>
    struct wrap_helper<Ret(Args...)> {
        using Func = Ret(Args...);
        static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, stack_type _sp, mem_type mem) {
            eastl::tuple<Args...> args;
            // The order here matters: m3ApiReturnType should go before calling get_args_from_stack,
            // since both modify `_sp`, and the return value on the stack is reserved before the arguments.
            m3ApiReturnType(Ret);
            get_args_from_stack(_sp, mem, args);
            Func* function = reinterpret_cast<Func*>(_ctx->userdata);
            Ret r = eastl::apply(function, args);
            m3ApiReturn(r);
        }
    };

    template <typename ...Args>
    struct wrap_helper<void(Args...)> {
        using Func = void(Args...);
        static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, stack_type sp, mem_type mem) {
            eastl::tuple<Args...> args;
            get_args_from_stack(sp, mem, args);
            Func* function = reinterpret_cast<Func*>(_ctx->userdata);
            eastl::apply(function, args);
            m3ApiSuccess();
        }
    };
}
    template<typename Func>
    class utilx;
    template<typename Ret, typename ... Args>
    class utilx<Ret(Args...)> {
    public:
        constexpr static size_t n_args = sizeof...(Args);
        constexpr static const char signature[n_args + 4] = detail::m3_signature<Ret, Args...>::value;
        constexpr static const M3RawCall raw_call = &detail::wrap_helper<Ret(Args...)>::wrap_fn;
        FORCEINLINE static void fill_linkage(SWAHostFunctionDescriptor& linkage,
                                 const char * module_name,
                                 const char * function_name,
                                 Ret (*function)(Args...))
        {
            linkage.function_name = function_name;
            linkage.module_name = module_name;
            linkage.proc = reinterpret_cast<void*>(function);
            linkage.backend_wrappers.m3 = &detail::wrap_helper<Ret(Args...)>::wrap_fn;
            linkage.signatures.m3 = detail::m3_signature<Ret, Args...>::value;
        }
    };
} // namespace m3
} // namespace swa