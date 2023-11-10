#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_manager.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct InputContext;

// TODO. roadmap
// //  1. hit test
// //  2. pointer events
// //  3. pointer events bind
//  4. 实现 MouseRegion（同时补全 state 和 增量更新的机制）
//  5. focus (Pointer)
//  6. keyboard events
//  7. keyboard events bind
//  8. focus & navigation (keyboard)         // ! 可 TODO

// InputManager 管理 Context 提供全局 hit test 支持与事件分发
// TODO. 其只是作为 InputContext 的一个中心，其实这个功能是否可以放在 BuildOwner 内部？
sreflect_struct("guid": "5c9d7e26-c2a1-4785-8832-bda746906801")
SKR_GUI_API InputManager {

    // hit test
    bool hit_test(HitTestResult* result, Offsetf global_position);

    // register
    void register_context(NotNull<InputContext*> context);
    void unregister_context(NotNull<InputContext*> context);

private:
    Array<InputContext*> _contexts;
};
} // namespace gui sreflect
} // namespace skr sreflect