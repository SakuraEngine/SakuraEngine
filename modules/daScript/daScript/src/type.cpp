#include "types.hpp"

namespace skr {
namespace das {

TypeDecl TypeDecl::_make(::das::TypeDecl* ptr)
{
    return TypeDecl(ptr);
}

::das::TypeDecl* TypeDecl::_get() const
{
    return ptr;
}

TypeDecl::TypeDecl(const TypeDecl& t) SKR_NOEXCEPT
{
    ptr = t.ptr;
    ptr->addRef();
}

TypeDecl& TypeDecl::operator=(const TypeDecl& t) SKR_NOEXCEPT
{
    ptr = t.ptr;
    ptr->addRef();
    return *this;
}

TypeDecl::TypeDecl() SKR_NOEXCEPT {}
TypeDecl::TypeDecl(std::nullptr_t) SKR_NOEXCEPT {}

TypeDecl::~TypeDecl() SKR_NOEXCEPT
{
    SKR_ASSERT(ptr->use_count());
    ptr->delRef();
}

TypeDecl TypeDecl::MakeHandleType(const Library* lib, const char8_t* name) SKR_NOEXCEPT
{
    auto t = ::das::makeHandleType(static_cast<const LibraryImpl*>(lib)->libGroup, (const char*)name);
    return TypeDecl::_make(t.orphan());
}

TypeDecl TypeDecl::get_first_type() SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    return TypeDecl::_make(This->firstType.orphan());
}

void TypeDecl::set_first_type(TypeDecl decl) SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    This->firstType = (::das::TypeDecl*)decl.ptr;;
}

bool TypeDecl::is_smartptr() SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    return This->smartPtr;
}

void TypeDecl::set_is_smartptr(bool o) SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    This->smartPtr = o;
}

bool TypeDecl::is_constant() SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    return This->constant;
}

void TypeDecl::set_is_constant(bool o) SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    This->constant = o;
}

bool TypeDecl::is_ref() SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    return This->ref;
}

void TypeDecl::set_is_ref(bool o) SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    This->ref = o;
}

bool TypeDecl::is_ref_type() SKR_NOEXCEPT
{
    auto This = (::das::TypeDecl*)ptr;
    return This->isRefType();
}

} // namespace das
} // namespace skr

template<typename T> struct NoneVoidType { using Type = T; };
template<> struct NoneVoidType<void> { using Type = int; };

#define SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(DAS_TYPE,CTYPE) \
namespace skr { \
namespace das { \
    TypeDecl TypeFactory<CTYPE>::make(const Library* library) { \
        if constexpr (!std::is_void_v<DAS_TYPE> && !std::is_void_v<CTYPE>) \
            static_assert(sizeof(NoneVoidType<DAS_TYPE>::Type) == sizeof(NoneVoidType<CTYPE>::Type), \
            "Type size mismatching!");\
            static_assert(alignof(NoneVoidType<DAS_TYPE>::Type) == alignof(NoneVoidType<CTYPE>::Type), \
            "Type alignment mismatching!");\
        auto Lib = (const LibraryImpl*)library; \
        auto Ptr = ::das::typeFactory<DAS_TYPE>::make(Lib->libGroup).orphan(); \
        return TypeDecl::_make(Ptr); \
    } \
};\
};

SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(uint8_t, uint8_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(int8_t, int8_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(uint16_t, uint16_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(int16_t, int16_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(uint32_t, uint32_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(int32_t, int32_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(uint64_t, uint64_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(int64_t, int64_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(float, float);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(double, double);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(void, void);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(void*, void*);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(::das::float2, skr_float2_t);
SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(::das::float3, skr_float3_t);
// SKR_DASCRIPT_IMPLEMENT_BASE_TYPE_FACTORY(::das::float4, skr_float4_t);
