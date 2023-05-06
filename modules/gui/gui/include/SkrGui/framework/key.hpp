#pragma once
#include "SkrGui/framework/type_tree.hpp"
#include "SkrGui/module.configure.h"
#include "containers/lite.hpp"
#include "platform/configure.h"
#include <stdint.h>

SKR_DECLARE_TYPE_ID_FWD(skr::gui, State, skr_gui_state)

namespace skr
{
namespace gui
{
// 三种 Key
// Unique 永远不相等
// KeepState 用来保持 State 的引用，方便 State 在控件树的各处流转
// Int/Float/Name 用来在某些控件（比如 ListView）下重用 State，避免重复创建
// IntStorage/FloatStorage/NameStorage 用于暂存一些页面信息，例如多个 Tab 中某一页内的 Scroll Pos
enum class EKeyType
{
    None,           // -
    Unique,         // -
    KeepState,      // State*

    Int,            // uint64_t
    Float,          // float
    Name,           // string
    
    IntStorage,     // int64_t
    FloatStorage,   // float
    NameStorage,    // name
};

struct SKR_GUI_API Key final
{
    static Key make_unique();
    static Key make_state(State* state);
    static Key make_int(uint64_t value);
    static Key make_float(float value);
    static Key make_name(const char* value);
    static Key make_state_storage(State* state);
    static Key make_int_storage(uint64_t value);
    static Key make_float_storage(float value);
    static Key make_name_storage(const char* value);

public:
    Key() SKR_NOEXCEPT;
    Key(const Key&) SKR_NOEXCEPT;
    Key(Key&&) SKR_NOEXCEPT;
    Key& operator=(const Key&) SKR_NOEXCEPT;
    Key& operator=(Key&&) SKR_NOEXCEPT;
    ~Key() SKR_NOEXCEPT;
    bool operator==(const Key& other) const SKR_NOEXCEPT;
    bool operator!=(const Key& other) const SKR_NOEXCEPT;

private:
    EKeyType _type;
    union
    {
        State*      _state;
        uint64_t    _int;
        float       _float;
        TextStorage _name;
    };
};
}
} // namespace skr