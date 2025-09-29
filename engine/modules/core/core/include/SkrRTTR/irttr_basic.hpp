#pragma once
#include <SkrBase/type_info.hpp>
#include "SkrRTTR/type.hpp"
#include <SkrRTTR/type_registry.hpp>
#include "SkrRTTR/irttr_basic.generated.h"

namespace skr
{
struct [[sattr(
    guid = "3740620f-714d-4d78-b47e-095f256ba4a7"
)]] SKR_CORE_API IRTTRBasic
{
    virtual ~IRTTRBasic();

    //=> IRTTRBasic API
    static const RTTRType* StaticType();
    virtual const RTTRType* rttr_get_type() const = 0;
    virtual GUID rttr_get_typeid() const = 0;
    virtual void* rttr_get_head_ptr() const = 0;
    //=> IRTTRBasic API

    //=> Helper API
    template <typename TO>
    TO* rttr_cast();
    template <typename TO>
    const TO* rttr_cast() const;
    template <typename TO>
    bool rttr_is() const noexcept;
    bool rttr_is(const GUID& guid) const;
    //=> Helper API

    // disable default new/delete, please use SkrNewObj/SkrDeleteObj or RC<T> instead
    inline static void* operator new(std::size_t, void* p) { return p; }
    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;
};

} // namespace skr

// delete traits
template <std::derived_from<::skr::IRTTRBasic> T>
struct SkrDeleteTraits<T>
{
    SKR_FORCEINLINE static void* get_free_ptr(T* p)
    {
        return p->rttr_get_head_ptr();
    }
};

// helper api
namespace skr
{
template <typename TO>
inline TO* IRTTRBasic::rttr_cast()
{
    auto from_type = get_type_from_guid(this->rttr_get_typeid());
    void* cast_p = from_type->cast_to_base(::skr::type_id_of<TO>(), this->rttr_get_head_ptr());
    return reinterpret_cast<TO*>(cast_p);
}
template <typename TO>
inline const TO* IRTTRBasic::rttr_cast() const
{
    return const_cast<IRTTRBasic*>(this)->rttr_cast<TO>();
}
template <typename TO>
inline bool IRTTRBasic::rttr_is() const noexcept
{
    return rttr_cast<TO>() != nullptr;
}
inline bool IRTTRBasic::rttr_is(const GUID& guid) const
{
    auto from_type = get_type_from_guid(this->rttr_get_typeid());
    return from_type->cast_to_base(guid, this->rttr_get_head_ptr());
}
} // namespace skr