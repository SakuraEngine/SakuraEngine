#pragma once
#include "SkrGui/framework/fwd_containers.hpp"
#include "type/type.h"
#include "SkrGui/framework/type_tree.hpp"
#include <assert.h>

namespace skr {
namespace gui {
struct Diagnosticable;

struct SKR_GUI_API TypeTreeNode
{
    virtual ~TypeTreeNode() SKR_NOEXCEPT;
    virtual void initialize(skr_guid_t id, skr_dynamic_record_type_id type) SKR_NOEXCEPT {};

    skr_dynamic_record_type_id get_type() SKR_NOEXCEPT { return type; }

protected:
    friend struct TypeTreeImpl;
    virtual void create_dynamic_type(skr_guid_t id, skr_dynamic_record_type_id parent_type, const char8_t* name) SKR_NOEXCEPT;

    uint64_t size = 0;
    uint64_t align = 0;
    skr_dynamic_record_type_id type = nullptr;
};

template<typename T>
struct TypeTreeNodeBase : public TypeTreeNode
{
    TypeTreeNodeBase()
    {
        if constexpr (T::hasBaseType())
        {
            _parent = TypeTreeNodeBase<typename T::BaseType>::Initialize();
        }
        TypeTreeNodeBase::Initialize();
    }

    inline static TypeTreeNode* Initialize()
    {
        if (!_this->type)
        {
            _this->create_dynamic_type(
                T::getStaticTypeId(), _parent ? _parent->get_type() : nullptr,
                _name);
        }
        return _this;
    }

    void create_dynamic_type(skr_guid_t id, skr_dynamic_record_type_id parent_type, const char8_t* name) SKR_NOEXCEPT final
    {
        size = sizeof(T);
        align = alignof(T);

        TypeTreeNode::create_dynamic_type(id, parent_type, name);
    }
    static const char8_t* _name;
    static TypeTreeNodeBase* _this;
    static TypeTreeNode* _parent;
};

struct SKR_GUI_API TypeTree
{
    virtual ~TypeTree() SKR_NOEXCEPT;
    
    virtual void register_type(skr_guid_t id, TypeTreeNode* node) SKR_NOEXCEPT = 0;
};

namespace literals
{
constexpr const size_t short_guid_form_length = 36; // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
constexpr const size_t long_guid_form_length = 38;  // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

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

constexpr skr_guid_t operator""_guid(const char8_t* str, size_t N)
{
    if (!(N == long_guid_form_length || N == short_guid_form_length))
    {
        assert(0 && "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");
    }
    if (N == long_guid_form_length && (str[0] != '{' || str[long_guid_form_length - 1] != '}'))
    {
        assert(0 && "Missing opening or closing brace");
    }   
    return make_guid_helper(str + (N == long_guid_form_length ? 1 : 0));
}
}

} }

namespace skr::type
{
    template<>
    struct type_id<struct skr::gui::Diagnosticable>
    {
        inline static SKR_CONSTEXPR const char8_t* str()
        {
            return u8"ae91b91e-e45b-4aa9-b574-bccd5e8119cb";
        }
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return skr::gui::literals::make_guid_helper(str());
        }
    };
}

// TYPE BODY MACROS

#define SKR_GUI_BASE_TYPE_BODY(__T) \
inline static SKR_CONSTEXPR bool hasBaseType() { return false; }

#define SKR_GUI_CHILD_TYPE_BODY(__T, __BASE) \
inline static SKR_CONSTEXPR skr_guid_t getBaseTypeId() \
{\
    return __BASE::getStaticTypeId();\
}\
inline static const skr_record_type_id getBaseType() \
{\
    return __BASE::getStaticType();\
}\
using BaseType = __BASE;\
inline static SKR_CONSTEXPR bool hasBaseType() { return true; }

#define SKR_GUI_TYPE_COMMON_BODY(__T, __GUID) \
static const skr_record_type_id getStaticType(); \
inline static SKR_CONSTEXPR const char8_t* getStaticTypeIdStr() \
{ \
    return __GUID; \
} \
inline static SKR_CONSTEXPR skr_guid_t getStaticTypeId() \
{ \
    return skr::gui::literals::make_guid_helper(getStaticTypeIdStr()); \
} \
skr_guid_t get_type() override { return getStaticTypeId(); }\

#define SKR_GUI_BASE_TYPE(__T, __GUID) \
SKR_GUI_TYPE_COMMON_BODY(__T, __GUID) \
SKR_GUI_BASE_TYPE_BODY(__T)\
virtual const skr_record_type_id getType() { return getStaticType(); } \

#define SKR_GUI_TYPE(__T, __BASE, __GUID) \
SKR_GUI_TYPE_COMMON_BODY(__T, __GUID) \
SKR_GUI_CHILD_TYPE_BODY(__T, __BASE) \
virtual const skr_record_type_id getType() override { return getStaticType(); } \


// TYPE IMPLEMENTATION MACROS

#define SKR_GUI_TYPE_IMPLMENTATION(__T) \
skr::gui::TypeTreeNodeBase<__T> __Type_##__T##_Instance__; \
template<> skr::gui::TypeTreeNodeBase<__T>* skr::gui::TypeTreeNodeBase<__T>::_this = &__Type_##__T##_Instance__; \
template<> skr::gui::TypeTreeNode* skr::gui::TypeTreeNodeBase<__T>::_parent = nullptr; \
template<> const char8_t* skr::gui::TypeTreeNodeBase<__T>::_name = OSTR_UTF8(#__T); \
const skr_record_type_id __T::getStaticType() \
{ \
    return (skr_record_type_id)__Type_##__T##_Instance__.get_type(); \
}