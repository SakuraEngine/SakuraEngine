#pragma once
#include "SkrDAScript/env.hpp"

namespace das { struct TypeDecl; class BuiltInFunction; }

namespace skr {
namespace das {

struct TypeDecl
{
    SKR_DASCRIPT_API static TypeDecl MakeHandleType(const Library* lib, const char8_t* name) SKR_NOEXCEPT;

    SKR_DASCRIPT_API TypeDecl get_first_type() SKR_NOEXCEPT;
    SKR_DASCRIPT_API void set_first_type(TypeDecl decl) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_smartptr() SKR_NOEXCEPT;
    SKR_DASCRIPT_API void set_is_smartptr(bool) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_constant() SKR_NOEXCEPT;
    SKR_DASCRIPT_API void set_is_constant(bool) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_ref() SKR_NOEXCEPT;
    SKR_DASCRIPT_API void set_is_ref(bool) SKR_NOEXCEPT;
    SKR_DASCRIPT_API bool is_ref_type() SKR_NOEXCEPT;

    template <typename T>
    static TypeDecl MakeType(const Library* lib);
    template <typename T>
    static TypeDecl MakeArgumentType(const Library* lib);

    SKR_DASCRIPT_API TypeDecl() SKR_NOEXCEPT;
    SKR_DASCRIPT_API TypeDecl(std::nullptr_t) SKR_NOEXCEPT;
    SKR_DASCRIPT_API ~TypeDecl() SKR_NOEXCEPT;

public: // none-export methods
    static TypeDecl _make(::das::TypeDecl* ptr);
    ::das::TypeDecl* _get() const;

private:
    TypeDecl(::das::TypeDecl* ptr) : ptr(ptr) {}
    ::das::TypeDecl* ptr = nullptr;
};

} // namespace das
} // namespace skr

#define SKR_DASCRIPT_DECLARE_TYPE_FACTORY(API,TYPE,CTYPE) \
namespace skr { \
namespace das { \
    struct Library; \
    struct TypeDecl; \
    template <typename TT> struct TypeFactory; \
    template <> \
    struct TypeFactory<CTYPE> { \
        API static TypeDecl make(const Library* library); \
    }; \
    template <typename TT> struct TypeName; \
    template <> \
    struct TypeName<CTYPE> { \
        constexpr static const char* name() { return #TYPE; } \
    }; \
};\
};

#define SKR_DASCRIPT_IMPLEMENT_TYPE_FACTORY(TYPE,CTYPE) \
namespace skr { \
namespace das { \
    TypeDecl TypeFactory<CTYPE>::make(const Library* library) { \
        return TypeDecl::MakeHandleType(library, (const char8_t*)#TYPE); \
    } \
};\
};

#define SKR_DASCRIPT_INLINE_TYPE_FACTORY(TYPE,CTYPE) \
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(,TYPE,CTYPE) \
namespace skr { \
namespace das { \
    inline TypeDecl TypeFactory<CTYPE>::make(const Library* library) { \
        return TypeDecl::MakeHandleType(library, (const char8_t*)#TYPE); \
    } \
};\
};

SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, uint8_t, uint8_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, int8_t, int8_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, uint16_t, uint16_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, int16_t, int16_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, uint32_t, uint32_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, int32_t, int32_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, uint64_t, uint64_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, int64_t, int64_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, char*, char*);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, const char*, const char*);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, float, float);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, double, double);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, void, void);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, void*, void*);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, skr_float2_t, skr_float2_t);
SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, skr_float3_t, skr_float3_t);
// SKR_DASCRIPT_DECLARE_TYPE_FACTORY(SKR_DASCRIPT_API, skr_float4_t, skr_float4_t);

namespace skr { 
namespace das { 

template <typename TT>
struct TypeFactory<TT*>  
{
    static FORCEINLINE TypeDecl make(const Library* library) 
    {
        auto pt = TypeFactory<void*>::make(library);
        if ( !std::is_void<TT>::value ) {
            pt.set_first_type(TypeFactory<TT>::make(library));
        }
        return pt;
    }
};

template <typename TT>
struct TypeFactory<const TT*>  
{
    static FORCEINLINE TypeDecl make(const Library* library) 
    {
        auto pt = TypeFactory<void*>::make(library);
        if ( !std::is_void<TT>::value ) {
            auto ft = TypeFactory<TT>::make(library);
            ft.set_is_constant(true);
            pt.set_first_type(ft);
        }
        pt.set_is_constant(true);
        return pt;
    }
};

template <typename TT>
struct TypeFactory<TT&>  
{
    static FORCEINLINE TypeDecl make(const Library* library) 
    {
        auto pt = TypeFactory<TT>::make(library);
        pt.set_is_ref(true);
        return pt;
    }
};

template <typename TT>
struct TypeFactory<const TT&>  
{
    static FORCEINLINE TypeDecl make(const Library* library) 
    {
        auto pt = TypeFactory<TT>::make(library);
        pt.set_is_ref(true);
        pt.set_is_constant(true);
        return pt;
    }
};

template <typename TT>
struct TypeFactory<const TT>  
{
    static FORCEINLINE TypeDecl make(const Library* library) 
    {
        auto pt = TypeFactory<TT>::make(library);
        pt.set_is_constant(true);
        return pt;
    }
};

};
};

namespace skr { 
namespace das { 

template <typename T>
TypeDecl TypeDecl::MakeType(const Library* lib)
{
    return TypeFactory<T>::make(lib);
}

template <typename T>
TypeDecl TypeDecl::MakeArgumentType(const Library* lib)
{
    auto tt = TypeFactory<T>::make(lib);
    if (tt.is_ref_type())
    {
        tt.set_is_ref(false);
    } 
    else if (!tt.is_ref()) {
        // note:
        //  C ++ does not differenciate between void foo ( Foo ); and void foo ( const Foo );
        //  DAS differenciates for pointers
        tt.set_is_constant(true);
    }
    return tt; 
}

};
};
