#pragma once
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API IObject {
    virtual ~IObject() = default;

    //=> IObject API
    virtual RecordType* get_record_type() const = 0;
    virtual void*       get_head_ptr() const    = 0;
    //=> IObject API

    //=> Helper API
    template <typename TO>
    TO* type_cast()
    {
        BaseInfo result;
        if (get_record_type()->find_base(RTTRTraits<TO>::get_type(), result))
        {
            uint8_t* p_head = reinterpret_cast<uint8_t*>(get_head_ptr());
            return reinterpret_cast<TO*>(p_head + result.offset);
        }
        else
        {
            return nullptr;
        }
    }
    template <typename TO>
    const TO* type_cast() const
    {
        return const_cast<IObject*>(this)->type_cast<TO>();
    }
    template <typename TO>
    TO* type_cast_fast() const { return type_cast<TO>(); }
    template <typename TO>
    TO* type_cast_fast() { return type_cast<TO>(); }
    template <typename TO>
    bool type_is() const noexcept
    {
        return type_cast<TO>() != nullptr;
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
            return static_cast<RecordType*>(RTTRTraits<ThisType>::get_type());        \
        }                                                                             \
        void* get_head_ptr() const override { return const_cast<void*>((const void*)this); }
#else
    #define SKR_RTTR_GENERATE_BODY()                                                  \
        ::skr::rttr::RecordType* get_record_type() const override { return nullptr; } \
        void*                    get_head_ptr() const override { return nullptr; }
#endif

// TODO. remove it
#include "SkrRT/type/type_id.hpp"
namespace skr::type
{
template <>
struct type_id<::skr::rttr::IObject> {
    inline static SKR_CONSTEXPR skr_guid_t get()
    {
        using namespace skr::guid::literals;
        return u8"19246699-65f8-4c0b-a82e-7886a0cb315d"_guid;
    }
    inline static SKR_CONSTEXPR std::string_view str()
    {
        return "19246699-65f8-4c0b-a82e-7886a0cb315d";
    }
};
template <>
struct SKR_RUNTIME_API type_register<::skr::rttr::IObject> {
    static void instantiate_type(RecordType* type)
    {
    }
    inline static constexpr skr_guid_t get_id()
    {
        using namespace skr::guid::literals;
        return u8"19246699-65f8-4c0b-a82e-7886a0cb315d"_guid;
    }
};
} // namespace skr::type
