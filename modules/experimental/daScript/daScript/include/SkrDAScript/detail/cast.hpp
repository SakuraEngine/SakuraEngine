#pragma once
#include "SkrDAScript/detail/platform.hpp"

namespace skr {
namespace das {

FORCEINLINE void * v_extract_ptr(vec4i a) {
#if INTPTR_MAX == INT32_MAX
    return (void*)v_extract_xi(a);
#else
    return (void*)v_extract_xi64(a);
#endif

}

#if defined(_MSC_VER) && !defined(__clang__) && INTPTR_MAX == INT32_MAX//MSVC generates flawed code, for example in _builtin_binary_save, so make it out of line
extern vec4i VECTORCALL v_ldu_ptr(const void * a);
#else
FORCEINLINE vec4i VECTORCALL v_ldu_ptr(const void * a) {
#if INTPTR_MAX == INT32_MAX
    return v_seti_x((int32_t)a);
#else
    return v_ldui_half(&a);
#endif
}
#endif

template <typename TT>
struct cast;

template <typename TT>
struct has_cast {
private:
    static int detect(...);
    template<typename U>
    static decltype(cast<U>::from(declval<U>())) detect(const U&);
public:
    enum { value = std::is_same<reg4f, decltype(detect(declval<TT>()))>::value };
};

template <typename TT>
struct cast <const TT> : cast<TT> {};

template <typename TT>
FORCEINLINE reg4f cast_result ( TT arg ) {
    return cast<TT>::from(arg);
}

template <typename TT>
struct cast <TT *> {
    static FORCEINLINE TT * to ( reg4f a )               { return (TT *) v_extract_ptr(v_cast_vec4i((a))); }
    static FORCEINLINE reg4f from ( const TT * p )       { return v_cast_vec4f(v_ldu_ptr((const void *)p)); }
};

template <typename TT>
struct cast <TT &> {
    static FORCEINLINE TT & to ( reg4f a )               { return *(TT *) v_extract_ptr(v_cast_vec4i((a))); }
    static FORCEINLINE reg4f from ( const TT & p )       { return v_cast_vec4f(v_ldu_ptr((const void *)&p)); }
};

template <typename TT>
struct cast <const TT *> {
    static FORCEINLINE const TT * to ( reg4f a )         { return (const TT *) v_extract_ptr(v_cast_vec4i((a))); }
    static FORCEINLINE reg4f from ( const TT * p )       { return v_cast_vec4f(v_ldu_ptr((const void *)p)); }
};

template <typename TT>
struct cast <const TT &> {
    static FORCEINLINE const TT & to ( reg4f a )         { return *(const TT *) v_extract_ptr(v_cast_vec4i((a))); }
    static FORCEINLINE reg4f from ( const TT & p )       { return v_cast_vec4f(v_ldu_ptr((const void *)&p)); }
};

template <>
struct cast <reg4f> {
    static FORCEINLINE reg4f to ( reg4f x )              { return x; }
    static FORCEINLINE reg4f from ( reg4f x )            { return x; }
};

template <>
struct cast <bool> {
    static FORCEINLINE bool to ( reg4f x )               { return (bool) v_extract_xi(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( bool x )             { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <int8_t> {
    static FORCEINLINE int8_t to ( reg4f x )             { return int8_t(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( int8_t x )           { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <uint8_t> {
    static FORCEINLINE uint8_t to ( reg4f x )            { return uint8_t(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( uint8_t x )          { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <int16_t> {
    static FORCEINLINE int16_t to ( reg4f x )            { return int16_t(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( int16_t x )          { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <uint16_t> {
    static FORCEINLINE uint16_t to ( reg4f x )           { return uint16_t(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( uint16_t x )         { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <int32_t> {
    static FORCEINLINE int32_t to ( reg4f x )            { return v_extract_xi(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( int32_t x )          { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <uint32_t> {
    static FORCEINLINE uint32_t to ( reg4f x )           { return v_extract_xi(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( uint32_t x )         { return v_cast_vec4f(v_seti_x(x)); }
};

template <>
struct cast <int64_t> {
    static FORCEINLINE int64_t to ( reg4f x )            { return v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( int64_t x )          { return v_cast_vec4f(v_ldui_half(&x)); }
};

template <>
struct cast <char> {
    static FORCEINLINE char to ( reg4f x )            { return char(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( char x )          { return v_cast_vec4f(v_seti_x(x)); }
};


#if defined(_MSC_VER)

template <>
struct cast <long> {
    static FORCEINLINE long to ( reg4f x )               { return (long) v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( long x )             { return v_cast_vec4f(v_ldui_half(&x)); }
};

template <>
struct cast <unsigned long> {
    static FORCEINLINE unsigned long to ( reg4f x )      { return (unsigned long) v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( unsigned long x )    { return v_cast_vec4f(v_ldui_half(&x)); }
};

template <>
struct cast <long double> {
    static FORCEINLINE long double to(reg4f x)            { union { reg4f v; long double t; } A; A.v = x; return A.t; }
    static FORCEINLINE reg4f from ( long double x )       { union { reg4f v; long double t; } A; A.t = x; return A.v; }
};

template <>
struct cast <wchar_t> {
    static FORCEINLINE wchar_t to ( reg4f x )           { return wchar_t(v_extract_xi(v_cast_vec4i(x))); }
    static FORCEINLINE reg4f from ( wchar_t x )         { return v_cast_vec4f(v_seti_x(x)); }
};

#endif

#if defined(__APPLE__)
#if __LP64__
    template <>
    struct cast <size_t> {
        static FORCEINLINE size_t to ( reg4f x )           { return v_extract_xi64(v_cast_vec4i(x)); }
        static FORCEINLINE reg4f from ( size_t x )         { return v_cast_vec4f(v_ldui_half(&x)); }
    };
#else
    template <>
    struct cast <size_t> {
        static FORCEINLINE size_t to ( reg4f x )           { return v_extract_xi(v_cast_vec4i(x)); }
        static FORCEINLINE reg4f from ( size_t x )         { return v_cast_vec4f(v_seti_x(x)); }
    };
#endif
#endif

template <>
struct cast <uint64_t> {
    static FORCEINLINE uint64_t to ( reg4f x )           { return v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( uint64_t x )         { return v_cast_vec4f(v_ldui_half(&x)); }
};

#if defined(__linux__)
template <>
struct cast <long long int> {
    static FORCEINLINE long long int to ( reg4f x )            { return v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( long long int x )          { return v_cast_vec4f(v_ldui_half(&x)); }
};
template <>
struct cast <unsigned long long int> {
    static FORCEINLINE unsigned long long int to ( reg4f x )           { return v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( unsigned long long int x )         { return v_cast_vec4f(v_ldui_half(&x)); }
};
#endif

#ifdef _EMSCRIPTEN_VER

template <>
struct cast <unsigned long> {
    static FORCEINLINE unsigned long to ( reg4f x )      { return (unsigned long) v_extract_xi64(v_cast_vec4i(x)); }
    static FORCEINLINE reg4f from ( unsigned long x )    { return v_cast_vec4f(v_ldui_half(&x)); }
};

#endif

template <>
struct cast <float> {
    static FORCEINLINE float to ( reg4f x )              { return v_extract_x(x); }
    static FORCEINLINE reg4f from ( float x )            { return v_set_x(x); }
};

template <>
struct cast <double> {
    static FORCEINLINE double to(reg4f x)                { union { reg4f v; double t; } A; A.v = x; return A.t; }
    static FORCEINLINE reg4f from ( double x )           { union { reg4f v; double t; } A; A.t = x; return A.v; }
};

} // namespace das
} // namespace skr
