#include "SkrGui/framework/layer/layer.hpp"

namespace skr::gui
{

// lifecycle & tree
// ctor -> mount <-> unmount -> destroy
void Layer::mount(NotNull<Layer*> parent) SKR_NOEXCEPT
{
    // validate
    if (_parent != nullptr)
    {
        unmount();
        SKR_GUI_LOG_ERROR(u8"already mounted");
    }
    {
        Layer* node = parent;
        while (node->_parent)
        {
            node = node->_parent;
            if (node == this)
            {
                SKR_GUI_LOG_ERROR(u8"cycle in the tree");
                break;
            }
        }
    }

    // mount
    _parent = parent;
    if (parent->owner())
    {
        struct _RecursiveHelper {
            NotNull<BuildOwner*> owner;

            void operator()(NotNull<Layer*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
            }
        };
        _RecursiveHelper{ parent->owner() }(this);
    }
}
void Layer::unmount() SKR_NOEXCEPT
{
    // validate
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR(u8"already unmounted"); }

    // unmount
    _parent = nullptr;
    if (owner())
    {
        struct _RecursiveHelper {
            void operator()(NotNull<Layer*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
            }
        };
        this->visit_children(_RecursiveHelper{});
    }
}
void Layer::destroy() SKR_NOEXCEPT
{
}
void Layer::attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
    // validate
    if (_owner != nullptr) { SKR_GUI_LOG_ERROR(u8"already attached"); }

    // attach
    _owner = owner;
    _depth = _parent ? _parent->_depth + 1 : 0;
}
void Layer::detach() SKR_NOEXCEPT
{
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR(u8"already detached"); }
    _owner = nullptr;
    if (_parent != nullptr && _owner != _parent->_owner) { SKR_GUI_LOG_ERROR(u8"detach from owner but parent is still attached"); }
}

// dirty
void Layer::mark_needs_composite() SKR_NOEXCEPT
{
    if (!_needs_composite)
    {
        // TODO. schedule composite
        _needs_composite = true;
    }

    auto parent = _parent;
    while (parent)
    {
        parent->mark_needs_composite();
    }
}

} // namespace skr::gui