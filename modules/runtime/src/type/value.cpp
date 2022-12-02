#include "type/type.hpp"

skr_value_t::skr_value_t(const skr_value_t& other)
{
    _Copy(other);
}

skr_value_t::skr_value_t(const skr_value_ref_t& ref)
{
    _Copy(ref);
}

skr_value_t::skr_value_t(skr_value_t&& other)
{
    _Move(std::move(other));
}

skr_value_t& skr_value_t::operator=(const skr_value_t& other)
{
    Reset();
    _Copy(other);
    return *this;
}

skr_value_t& skr_value_t::operator=(skr_value_t&& other)
{
    Reset();
    _Move(std::move(other));
    return *this;
}

skr_value_t& skr_value_t::operator=(const skr_value_ref_t& other)
{
    Reset();
    _Copy(other);
    return *this;
}

void* skr_value_t::Ptr()
{
    if (!type)
        return nullptr;
    if (type->Size() < smallSize)
        return &_smallObj[0];
    else
        return _ptr;
}

const void* skr_value_t::Ptr() const
{
    if (!type)
        return nullptr;
    if (type->Size() < smallSize)
        return &_smallObj[0];
    else
        return _ptr;
}

void skr_value_t::Reset()
{
    if (!type)
        return;
    if (type->Size() < smallSize)
        type->Destruct(&_smallObj[0]);
    else
    {
        type->Destruct(_ptr);
        sakura_free(_ptr);
    }
    type = nullptr;
}

size_t skr_value_t::Hash() const
{
    if (!type)
        return 0;
    return type->Hash(Ptr(), 0);
}

skr::string skr_value_t::ToString() const
{
    if (!type)
        return {};
    return type->ToString(Ptr());
}

void* skr_value_t::_Alloc()
{
    if (!type)
        return nullptr;
    auto size = type->Size();
    if (size < smallSize)
        return &_smallObj[0];
    else
        return _ptr = sakura_malloc(size);
}

void skr_value_t::_Copy(const skr_value_t& other)
{
    type = other.type;
    if (!type)
        return;
    auto ptr = _Alloc();
    type->Copy(ptr, other.Ptr());
}

void skr_value_t::_Copy(const skr_value_ref_t& other)
{
    type = other.type;
    if (!type)
        return;
    auto ptr = _Alloc();
    type->Copy(ptr, other.ptr);
}

void skr_value_t::_Move(skr_value_t&& other)
{
    type = other.type;
    if (!type)
        return;
    auto ptr = _Alloc();
    type->Move(ptr, other.Ptr());
    other.Reset();
}

// value ref
skr_value_ref_t::skr_value_ref_t(void* address, const skr_type_t* inType)
{
    ptr = address;
    type = inType;
}

skr_value_ref_t::skr_value_ref_t(skr_value_t& v)
{
    ptr = v.Ptr();
    type = v.type;
}
skr_value_ref_t::skr_value_ref_t(skr_value_ref_t& other)
{
    ptr = other.ptr;
    type = other.type;
}
skr_value_ref_t::skr_value_ref_t(const skr_value_ref_t& other)
{
    ptr = other.ptr;
    type = other.type;
}
skr_value_ref_t& skr_value_ref_t::operator=(const skr_value_ref_t& other)
{
    ptr = other.ptr;
    type = other.type;
    return *this;
}
skr_value_ref_t& skr_value_ref_t::operator=(skr_value_ref_t& other)
{
    ptr = other.ptr;
    type = other.type;
    return *this;
}

void skr_value_ref_t::Reset()
{
    type = nullptr;
    ptr = nullptr;
}

size_t skr_value_ref_t::Hash() const
{
    if (!type)
        return 0;
    return type->Hash(ptr, 0);
}

skr::string skr_value_ref_t::ToString() const
{
    if (!type)
        return {};
    return type->ToString(ptr);
}

skr_value_ref_t::~skr_value_ref_t()
{
    Reset();
}
bool skr_value_ref_t::operator==(const skr_value_ref_t& other)
{
    return ptr == other.ptr && type == other.type;
}
bool skr_value_ref_t::operator!=(const skr_value_ref_t& other)
{
    return !((*this) == other);
}

skr_value_ref_t::operator bool() const { return HasValue(); }
bool skr_value_ref_t::HasValue() const { return type != nullptr; }