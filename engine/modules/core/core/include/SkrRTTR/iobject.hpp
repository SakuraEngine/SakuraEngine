#pragma once
#include <SkrBase/type_info.hpp>
#include "SkrRTTR/type.hpp"
#include <SkrRTTR/type_registry.hpp>
#include "SkrRTTR/iobject.generated.h"

//!===================================== 比较良好的模式 =====================================
//! pooling 通过向全局 push 钩子拦截对象的创建来实现，直接内置在 Object 系统中，这样可以有最好的联动和编码体验
//! 关于内存管理，是否考虑使用 ObjHost<T> 和 ObjObs<T> 来进行管理，这种方式更契合 C++ 的编码思路，同时能限制混乱的产生
//!=======================================================================================

// iobject
namespace skr
{
struct [[sattr(
    guid = "3740620f-714d-4d78-b47e-095f256ba4a7"
)]] SKR_CORE_API IObject
{
    virtual ~IObject() = default;

    //=> IObject API
    virtual GUID iobject_get_typeid() const = 0;
    virtual void* iobject_get_head_ptr() const = 0;
    //=> IObject API

    //=> Helper API
    template <typename TO>
    TO* type_cast();
    template <typename TO>
    const TO* type_cast() const;
    template <typename TO>
    const TO* type_cast_fast() const;
    template <typename TO>
    TO* type_cast_fast();
    template <typename TO>
    bool type_is() const noexcept;
    bool type_is(const GUID& guid) const;
    //=> Helper API

    // disable default new/delete, please use SkrNewObj/SkrDeleteObj or RC<T> instead
    inline static void* operator new(std::size_t, void* p) { return p; }
    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;
};

} // namespace skr

// delete traits
template <std::derived_from<::skr::IObject> T>
struct SkrDeleteTraits<T>
{
    SKR_FORCEINLINE static void* get_free_ptr(T* p)
    {
        return p->iobject_get_head_ptr();
    }
};

namespace skr
{
// helper api
template <typename TO>
inline TO* IObject::type_cast()
{
    auto from_type = get_type_from_guid(this->iobject_get_typeid());
    void* cast_p = from_type->cast_to_base(::skr::type_id_of<TO>(), this->iobject_get_head_ptr());
    return reinterpret_cast<TO*>(cast_p);
}
template <typename TO>
inline const TO* IObject::type_cast() const
{
    return const_cast<IObject*>(this)->type_cast<TO>();
}
template <typename TO>
inline const TO* IObject::type_cast_fast() const
{
    return type_cast<TO>();
}
template <typename TO>
inline TO* IObject::type_cast_fast()
{
    return type_cast<TO>();
}
template <typename TO>
inline bool IObject::type_is() const noexcept
{
    return type_cast<TO>() != nullptr;
}
inline bool IObject::type_is(const GUID& guid) const
{
    auto from_type = get_type_from_guid(this->iobject_get_typeid());
    return from_type->cast_to_base(guid, this->iobject_get_head_ptr());
}
} // namespace skr