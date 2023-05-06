#pragma once
#include "SkrGui/framework/type_tree.hpp"

namespace skr
{
namespace gui
{
struct SKR_GUI_API Key : public SInterface
{
    SKR_GUI_BASE_TYPE(Key, "c53dd579-ec79-48fb-8b67-b30736774c67")

    virtual ~Key() = default;
    virtual bool operator==(const Key& other) const = 0;
};
}
} // namespace skr