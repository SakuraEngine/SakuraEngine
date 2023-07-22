#pragma once
#include "SkrGui/module.configure.h"
#include "SkrRT/misc/types.h"
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/platform/configure.h"
#include "SkrRT/misc/log.hpp"

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
    T                result{};
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
    Data4[0]         = parse_hex<uint8_t>(begin);
    begin += 2;
    Data4[1] = parse_hex<uint8_t>(begin);
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
    return skr_guid_t(Data1, Data2, Data3, Data4);
}
} // namespace __help

// object & interface base
struct SKR_GUI_API IObject {
    virtual ~IObject()                                                                          = default;
    virtual skr_guid_t zz_internal_guid() const SKR_NOEXCEPT                                    = 0;
    virtual void       zz_internal_base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT = 0;
    virtual void*      zz_internal_cast(skr_guid_t id) const SKR_NOEXCEPT                       = 0;
};

// helper
template <typename... Super>
struct BaseGUIDHelper {
    inline static constexpr size_t     count       = sizeof...(Super);
    inline static constexpr skr_guid_t base_guid[] = { Super::zz_internal_static_guid()... };
};
template <typename Base, typename... Super>
struct BaseCastHelper {
    inline static void* super_cast(skr_guid_t guid, const Base* self) SKR_NOEXCEPT
    {
        for (auto result : { Super::zz_internal_castHelper::cast(guid, static_cast<const Super*>(self))... })
        {
            if (result != nullptr) return result;
        }
        return nullptr;
    }

    inline static void* cast(skr_guid_t guid, const Base* self) SKR_NOEXCEPT
    {
        if constexpr (sizeof...(Super) == 0)
            return guid == Base::zz_internal_static_guid() ? const_cast<Base*>(self) : nullptr;
        else
            return guid == Base::zz_internal_static_guid() ? const_cast<Base*>(self) : super_cast(guid, self);
    }
};

// cast
template <typename To, typename From>
inline To* SkrGUICast(From* from) SKR_NOEXCEPT
{
    if (from == nullptr) return nullptr;
    void* p = from->zz_internal_cast(To::zz_internal_static_guid());
    return p ? reinterpret_cast<To*>(p) : nullptr;
}
template <typename To, typename From>
inline To* SkrGUICastFast(From* from) SKR_NOEXCEPT
{
#ifdef _DEBUG
    if (from == nullptr) return nullptr;
    void* p = from->zz_internal_cast(To::zz_internal_static_guid());
    if (p == nullptr)
    {
        SKR_LOG_ERROR(u8"SkrGUICastFast failed, from: %s, to: %s", typeid(From).name(), typeid(To).name());
        return nullptr;
    }
    else
    {
        return reinterpret_cast<To*>(p);
    }
#else
    return from ? static_cast<To*>(from) : nullptr;
#endif
}
inline skr_guid_t SkrGUITypeInfo(const IObject* obj) SKR_NOEXCEPT
{
    return obj->zz_internal_guid();
}

inline bool SkrGUIBasedOn(const IObject* obj, skr_guid_t base_guid)
{
    void* p = obj->zz_internal_cast(base_guid);
    return p != nullptr;
}

} // namespace skr::gui

// type marco
#define SKR_GUI_OBJECT_ROOT(__T, __GUID)                                                                                                   \
    using zz_internal_castHelper = BaseCastHelper<__T>;                                                                                    \
    inline static constexpr skr_guid_t zz_internal_static_guid() SKR_NOEXCEPT                                                              \
    {                                                                                                                                      \
        constexpr auto guid = ::skr::gui::__help::make_guid_helper(u8##__GUID);                                                            \
        return guid;                                                                                                                       \
    }                                                                                                                                      \
    inline static constexpr void zz_internal_static_base_guid(const skr_guid_t*& p, size_t n) SKR_NOEXCEPT                                 \
    {                                                                                                                                      \
        p = nullptr;                                                                                                                       \
        n = 0;                                                                                                                             \
    }                                                                                                                                      \
    inline static void* zz_internal_cast(skr_guid_t id, const __T* self)                                                                   \
    {                                                                                                                                      \
        return id == zz_internal_static_guid() ? const_cast<__T*>(self) : nullptr;                                                         \
    }                                                                                                                                      \
    virtual skr_guid_t zz_internal_guid() const SKR_NOEXCEPT override { return zz_internal_static_guid(); }                                \
    virtual void zz_internal_base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT override { zz_internal_static_base_guid(p, n); } \
    virtual void* zz_internal_cast(skr_guid_t id) const SKR_NOEXCEPT override { return zz_internal_castHelper::cast(id, this); }

#define SKR_GUI_OBJECT(__T, __GUID, ...)                                                                                                   \
    using zz_internal_castHelper = BaseCastHelper<__T, __VA_ARGS__>;                                                                       \
    using zz_internal_guidHelper = BaseGUIDHelper<__VA_ARGS__>;                                                                            \
    inline static constexpr skr_guid_t zz_internal_static_guid()                                                                           \
    {                                                                                                                                      \
        constexpr auto guid = ::skr::gui::__help::make_guid_helper(u8##__GUID);                                                            \
        return guid;                                                                                                                       \
    }                                                                                                                                      \
    inline static constexpr void zz_internal_static_base_guid(const skr_guid_t*& p, size_t n)                                              \
    {                                                                                                                                      \
        p = zz_internal_guidHelper::base_guid;                                                                                             \
        n = zz_internal_guidHelper::count;                                                                                                 \
    }                                                                                                                                      \
    virtual skr_guid_t zz_internal_guid() const SKR_NOEXCEPT override { return zz_internal_static_guid(); }                                \
    virtual void zz_internal_base_guid(const skr_guid_t*& p, size_t n) const SKR_NOEXCEPT override { zz_internal_static_base_guid(p, n); } \
    virtual void* zz_internal_cast(skr_guid_t id) const SKR_NOEXCEPT override { return zz_internal_castHelper::cast(id, this); }

#define SKR_GUI_INTERFACE_ROOT(__T, __GUID) SKR_GUI_OBJECT_ROOT(__T, __GUID)
#define SKR_GUI_INTERFACE(__T, __GUID, ...) SKR_GUI_OBJECT(__T, __GUID, __VA_ARGS__)

// base marco
#define SKR_GUI_OBJECT_BASE : virtual public IObject
#define SKR_GUI_INTERFACE_BASE : virtual public IObject
#define SKR_GUI_OBJECT_BASE_WITH(...) : virtual public IObject, __VA_ARGS__
#define SKR_GUI_INTERFACE_BASE_WITH(...) : virtual public IObject, __VA_ARGS__

// type id
using SKR_GUI_TYPE_ID = ::skr_guid_t;
// TypeID SKR_GUI_TYPE_ID(Object*)
#define SKR_GUI_TYPE_ID_OF ::skr::gui::SkrGUITypeInfo
#define SKR_GUI_TYPE_ID_OF_STATIC(__TYPE) __TYPE::zz_internal_static_guid()
// To* SKR_GUI_CAST<To>(From*)
#define SKR_GUI_CAST ::skr::gui::SkrGUICast
#define SKR_GUI_CAST_FAST ::skr::gui::SkrGUICastFast
#define SKR_GUI_BASED_ON(__OBJ, __TYPE_ID) ::skr::gui::SkrGUIBasedOn(__OBJ, __TYPE_ID)