#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/render_input_context.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct InputManager;

sreflect_struct("guid": "6f9efada-d7db-47f7-82de-6d6cf659be60")
SKR_GUI_API RenderInputContext : public RenderProxyBox {

    void set_manager(InputManager* manager) SKR_NOEXCEPT;

private:
    InputManager* _manager;
};
} // namespace gui sreflect
} // namespace skr sreflect