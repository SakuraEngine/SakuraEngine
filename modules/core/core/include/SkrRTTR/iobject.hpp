#pragma once
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type/record_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
struct SKR_CORE_API IObject {
    virtual ~IObject() = default;

    //=> IObject API
    virtual RecordType* get_record_type() const = 0;
    virtual void*       get_head_ptr() const    = 0;
    //=> IObject API

    //=> Helper API
    template <typename TO>
    inline TO* type_cast()
    {
        auto  from_type = get_record_type();
        auto  to_type   = get_type_from_guid(skr::rttr::type_id_of<TO>());
        void* cast_p    = from_type->cast_to(to_type, get_head_ptr());
        return reinterpret_cast<TO*>(cast_p);
    }
    template <typename TO>
    inline const TO* type_cast() const
    {
        return const_cast<IObject*>(this)->type_cast<TO>();
    }
    template <typename TO>
    inline const TO* type_cast_fast() const { return type_cast<TO>(); }
    template <typename TO>
    inline TO* type_cast_fast() { return type_cast<TO>(); }
    template <typename TO>
    inline bool type_is() const noexcept
    {
        return type_cast<TO>() != nullptr;
    }
    inline bool type_is(const GUID& guid) const
    {
        auto from_type = get_record_type();
        auto to_type   = get_type_from_guid(guid);
        return from_type->cast_to(to_type, get_head_ptr());
    }
    inline GUID type_id() const
    {
        return get_record_type()->type_id();
    }
    //=> Helper API
};
} // namespace skr::rttr

SKR_RTTR_TYPE(IObject, "19246699-65f8-4c0b-a82e-7886a0cb315d")

#ifndef __meta__
    #define SKR_RTTR_GENERATE_BODY()                                                  \
        ::skr::rttr::RecordType* get_record_type() const override                     \
        {                                                                             \
            using namespace skr::rttr;                                                \
            using ThisType = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
            return static_cast<RecordType*>(type_of<ThisType>());                     \
        }                                                                             \
        void* get_head_ptr() const override { return const_cast<void*>((const void*)this); }
#else
    #define SKR_RTTR_GENERATE_BODY()                                                  \
        ::skr::rttr::RecordType* get_record_type() const override { return nullptr; } \
        void*                    get_head_ptr() const override { return nullptr; }
#endif