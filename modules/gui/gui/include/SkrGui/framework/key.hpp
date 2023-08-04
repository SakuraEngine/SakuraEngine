#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
// 三种 Key
// Unique 永远不相等
// KeepState 用来保持 State 的引用，方便 State 在控件树的各处流转
// Int/Float/Name 用来在某些控件（比如 ListView）下重用 State，避免重复创建
// IntStorage/FloatStorage/NameStorage 用于暂存一些页面信息，例如多个 Tab 中某一页内的 Scroll Pos
enum class EKeyType
{
    None,      // -
    Unique,    // -
    KeepState, // State* TODO. GlobalKey

    Int,   // int64_t
    Float, // float
    Name,  // string

    IntStorage,   // int64_t
    FloatStorage, // float
    NameStorage,  // string
};

struct SKR_GUI_API Key final {
    // create
    static Key unique() SKR_NOEXCEPT;
    static Key keep_state(State* state) SKR_NOEXCEPT;
    static Key value(int64_t v) SKR_NOEXCEPT;
    static Key value(float v) SKR_NOEXCEPT;
    static Key value(const String& v) SKR_NOEXCEPT;
    static Key storage(int64_t v) SKR_NOEXCEPT;
    static Key storage(float v) SKR_NOEXCEPT;
    static Key storage(const String& v) SKR_NOEXCEPT;

    // ctor & dtor & assign
    Key() SKR_NOEXCEPT;
    Key(const Key&) SKR_NOEXCEPT;
    Key(Key&&) SKR_NOEXCEPT;
    Key& operator=(const Key&) SKR_NOEXCEPT;
    Key& operator=(Key&&) SKR_NOEXCEPT;
    ~Key() SKR_NOEXCEPT;

    // compare
    bool operator==(const Key& other) const SKR_NOEXCEPT;
    bool operator!=(const Key& other) const SKR_NOEXCEPT;

    // type
    EKeyType type() const SKR_NOEXCEPT;
    bool     is_none() const SKR_NOEXCEPT;
    bool     is_unique() const SKR_NOEXCEPT;
    bool     is_keep_state() const SKR_NOEXCEPT;
    bool     is_value() const SKR_NOEXCEPT;
    bool     is_storage() const SKR_NOEXCEPT;

    // getter
    State*        get_state() const SKR_NOEXCEPT;
    int64_t       get_int() const SKR_NOEXCEPT;
    float         get_float() const SKR_NOEXCEPT;
    const String& get_name() const SKR_NOEXCEPT;
    bool          try_get_state(State*& out) const SKR_NOEXCEPT;
    bool          try_get_int(int64_t& out) const SKR_NOEXCEPT;
    bool          try_get_float(float& out) const SKR_NOEXCEPT;
    bool          try_get_name(String& out) const SKR_NOEXCEPT;

    // setter
    void clear() SKR_NOEXCEPT;
    void set_none() SKR_NOEXCEPT;
    void set_unique() SKR_NOEXCEPT;
    void set_keep_state(State* state) SKR_NOEXCEPT;
    void set_value(int64_t v) SKR_NOEXCEPT;
    void set_value(float v) SKR_NOEXCEPT;
    void set_value(const String& v) SKR_NOEXCEPT;
    void set_storage(int64_t v) SKR_NOEXCEPT;
    void set_storage(float v) SKR_NOEXCEPT;
    void set_storage(const String& v) SKR_NOEXCEPT;

private:
    EKeyType _type = EKeyType::None;
    union
    {
        State*  _state;
        int64_t _int;
        float   _float;
        String  _name;
    };
};

// create
inline Key Key::unique() SKR_NOEXCEPT
{
    Key k;
    k.set_unique();
    return k;
}
inline Key Key::keep_state(State* state) SKR_NOEXCEPT
{
    Key k;
    k.set_keep_state(state);
    return k;
}
inline Key Key::value(int64_t v) SKR_NOEXCEPT
{
    Key k;
    k.set_value(v);
    return k;
}
inline Key Key::value(float v) SKR_NOEXCEPT
{
    Key k;
    k.set_value(v);
    return k;
}
inline Key Key::value(const String& v) SKR_NOEXCEPT
{
    Key k;
    k.set_value(v);
    return k;
}
inline Key Key::storage(int64_t v) SKR_NOEXCEPT
{
    Key k;
    k.set_storage(v);
    return k;
}
inline Key Key::storage(float v) SKR_NOEXCEPT
{
    Key k;
    k.set_storage(v);
    return k;
}
inline Key Key::storage(const String& v) SKR_NOEXCEPT
{
    Key k;
    k.set_storage(v);
    return k;
}

// type
inline EKeyType Key::type() const SKR_NOEXCEPT
{
    return _type;
}
inline bool Key::is_none() const SKR_NOEXCEPT
{
    return _type == EKeyType::None;
}
inline bool Key::is_unique() const SKR_NOEXCEPT
{
    return _type == EKeyType::Unique;
}
inline bool Key::is_keep_state() const SKR_NOEXCEPT
{
    return _type == EKeyType::KeepState;
}
inline bool Key::is_value() const SKR_NOEXCEPT
{
    return _type == EKeyType::Int || _type == EKeyType::Float || _type == EKeyType::Name;
}
inline bool Key::is_storage() const SKR_NOEXCEPT
{
    return _type == EKeyType::IntStorage || _type == EKeyType::FloatStorage || _type == EKeyType::NameStorage;
}

// getter
inline State* Key::get_state() const SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(is_keep_state());
    return _state;
}
inline int64_t Key::get_int() const SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(_type == EKeyType::Int || _type == EKeyType::IntStorage);
    return _int;
}
inline float Key::get_float() const SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(_type == EKeyType::Float || _type == EKeyType::FloatStorage);
    return _float;
}
inline const String& Key::get_name() const SKR_NOEXCEPT
{
    SKR_GUI_ASSERT(_type == EKeyType::Name || _type == EKeyType::NameStorage);
    return _name;
}
inline bool Key::try_get_state(State*& out) const SKR_NOEXCEPT
{
    if (is_keep_state())
    {
        out = _state;
        return true;
    }
    return false;
}
inline bool Key::try_get_int(int64_t& out) const SKR_NOEXCEPT
{
    if (_type == EKeyType::Int || _type == EKeyType::IntStorage)
    {
        out = _int;
        return true;
    }
    return false;
}
inline bool Key::try_get_float(float& out) const SKR_NOEXCEPT
{
    if (_type == EKeyType::Float || _type == EKeyType::FloatStorage)
    {
        out = _float;
        return true;
    }
    return false;
}
inline bool Key::try_get_name(String& out) const SKR_NOEXCEPT
{
    if (_type == EKeyType::Name || _type == EKeyType::NameStorage)
    {
        out = _name;
        return true;
    }
    return false;
}

// setter
inline void Key::set_none() SKR_NOEXCEPT
{
    clear();
}
inline void Key::set_unique() SKR_NOEXCEPT
{
    clear();
    _type = EKeyType::Unique;
}
inline void Key::set_keep_state(State* state) SKR_NOEXCEPT
{
    clear();
    _type  = EKeyType::KeepState;
    _state = state;
}
inline void Key::set_value(int64_t v) SKR_NOEXCEPT
{
    clear();
    _type = EKeyType::Int;
    _int  = v;
}
inline void Key::set_value(float v) SKR_NOEXCEPT
{
    clear();
    _type  = EKeyType::Float;
    _float = v;
}
inline void Key::set_storage(int64_t v) SKR_NOEXCEPT
{
    clear();
    _type = EKeyType::IntStorage;
    _int  = v;
}
inline void Key::set_storage(float v) SKR_NOEXCEPT
{
    clear();
    _type  = EKeyType::FloatStorage;
    _float = v;
}
} // namespace skr::gui

// TODO. skr hash
namespace phmap
{
template <>
struct Hash<::skr::gui::Key> {
    size_t operator()(const ::skr::gui::Key& key) const SKR_NOEXCEPT
    {
        using namespace ::skr::gui;
        switch (key.type())
        {
            case EKeyType::None:
            case EKeyType::Unique:
                return 0;
            case EKeyType::KeepState:
                return Hash<State*>()(key.get_state());
            case EKeyType::Int:
            case EKeyType::IntStorage:
                return Hash<int64_t>()(key.get_int());
            case EKeyType::Float:
            case EKeyType::FloatStorage:
                return Hash<float>()(key.get_float());
            case EKeyType::Name:
            case EKeyType::NameStorage:
            {
                const auto& x = key.get_name();
                return ostr::hash_sequence_crc64(x.c_str(), x.size());
            }
            default:
            {
                SKR_UNREACHABLE_CODE();
                return UINT64_MAX;
            }
        }
    }
};
} // namespace phmap