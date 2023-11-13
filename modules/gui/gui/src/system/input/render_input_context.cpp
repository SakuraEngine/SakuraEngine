#include "SkrGui/system/input/render_input_context.hpp"
#include "SkrGui/system/input/input_manager.hpp"

namespace skr::gui
{
void RenderInputContext::set_manager(InputManager* manager) SKR_NOEXCEPT
{
    if (_manager)
    {
        _manager->unregister_context(this);
    }
    if (manager)
    {
        manager->register_context(this);
    }
    _manager = manager;
}

} // namespace skr::gui