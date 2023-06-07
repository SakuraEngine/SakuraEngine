#pragma once
#include "misc/types.h"
#include "platform/guid.hpp"

namespace skr::gui
{
// GUID parse
namespace __help
{
constexpr int parse_hex_digit(const char8_t c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        return -1;
}
template <class T>
constexpr T parse_hex(const char8_t* ptr)
{
    constexpr size_t digits = sizeof(T) * 2;
    T result{};
    for (size_t i = 0; i < digits; ++i)
        result |= parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
    return result;
}
constexpr skr_guid_t make_guid_helper(const char8_t* begin)
{
    auto Data1 = parse_hex<uint32_t>(begin);
    begin += 8 + 1;
    auto Data2 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    auto Data3 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    uint8_t Data4[8] = {};
    Data4[0] = parse_hex<uint8_t>(begin);
    begin += 2;
    Data4[1] = parse_hex<uint8_t>(begin);
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
    return skr_guid_t(Data1, Data2, Data3, Data4);
}
} // namespace __help

// object & interface base
struct IObject {
    virtual skr_guid_t guid() const SKR_NOEXCEPT = 0;
    virtual void base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT = 0;
    virtual void* cast(skr_guid_t id) const SKR_NOEXCEPT = 0;
};

// helper
template <typename... Super>
struct BaseGUIDHelper {
    inline static constexpr size_t count = sizeof...(Super);
    inline static constexpr skr_guid_t base_guid[] = { Super::static_guid()... };
};
template <typename Base, typename... Super>
struct BaseCastHelper {
    inline static void* super_cast(skr_guid_t guid, const Base* self) SKR_NOEXCEPT
    {
        for (auto result : { Super::CastHelper::cast(guid, static_cast<const Super*>(self))... })
        {
            if (result != nullptr) return result;
        }
        return nullptr;
    }

    inline static void* cast(skr_guid_t guid, const Base* self) SKR_NOEXCEPT
    {
        if constexpr (sizeof...(Super) == 0)
            return guid == Base::static_guid() ? const_cast<Base*>(self) : nullptr;
        else
            return guid == Base::static_guid() ? const_cast<Base*>(self) : super_cast(guid, self);
    }
};

// cast
template <typename To, typename From>
To* SkrGUICast(From* from)
{
    if (from == nullptr) return nullptr;
    void* p = from->cast(To::static_guid());
    return p ? reinterpret_cast<To*>(p) : nullptr;
}
inline static constexpr skr_guid_t SkrGUITypeInfo(const IObject* obj) SKR_NOEXCEPT
{
    return obj->guid();
}

} // namespace skr::gui

// type marco
#define SKR_GUI_TYPE_BASE(__T, __GUID)                                                                                         \
    using CastHelper = BaseCastHelper<__T>;                                                                                    \
    inline static constexpr skr_guid_t static_guid() SKR_NOEXCEPT { return ::skr::gui::__help::make_guid_helper(u8##__GUID); } \
    inline static constexpr void static_base_guid(const skr_guid_t*& p, size_t n) SKR_NOEXCEPT                                 \
    {                                                                                                                          \
        p = nullptr;                                                                                                           \
        n = 0;                                                                                                                 \
    }                                                                                                                          \
    inline static void* cast(skr_guid_t id, const __T* self)                                                                   \
    {                                                                                                                          \
        return id == static_guid() ? const_cast<__T*>(self) : nullptr;                                                         \
    }                                                                                                                          \
    virtual skr_guid_t guid() const SKR_NOEXCEPT override { return static_guid(); }                                            \
    virtual void base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT override { static_base_guid(p, n); }             \
    virtual void* cast(skr_guid_t id) const SKR_NOEXCEPT override { return CastHelper::cast(id, this); }

#define SKR_GUI_TYPE(__T, __GUID, ...)                                                                             \
    using CastHelper = BaseCastHelper<__T, __VA_ARGS__>;                                                           \
    using GUIDHelper = BaseGUIDHelper<__VA_ARGS__>;                                                                \
    inline static constexpr skr_guid_t static_guid() { return ::skr::gui::__help::make_guid_helper(u8##__GUID); }  \
    inline static constexpr void static_base_guid(const skr_guid_t*& p, size_t n)                                  \
    {                                                                                                              \
        p = GUIDHelper::base_guid;                                                                                 \
        n = GUIDHelper::count;                                                                                     \
    }                                                                                                              \
    virtual skr_guid_t guid() const SKR_NOEXCEPT override { return static_guid(); }                                \
    virtual void base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT override { static_base_guid(p, n); } \
    virtual void* cast(skr_guid_t id) const SKR_NOEXCEPT override { return CastHelper::cast(id, this); }

// base marco
#define SKR_GUI_OBJECT_BASE : virtual public IObject
#define SKR_GUI_INTERFACE_BASE : virtual public IObject
#define SKR_GUI_OBJECT_BASE_WITH(...) : virtual public IObject, __VA_ARGS__
#define SKR_GUI_INTERFACE_BASE_WITH(...) : virtual public IObject, __VA_ARGS__

// type id
#define SKR_GUI_TYPE_ID ::skr_guid_t
// TypeID SKR_GUI_TYPE_ID(Object*)
#define SKR_GUI_TYPE_ID_OF ::skr::gui::SkrGUITypeInfo
// To* SKR_GUI_CAST<To>(From*)
#define SKR_GUI_CAST ::skr::gui::SkrGUICast