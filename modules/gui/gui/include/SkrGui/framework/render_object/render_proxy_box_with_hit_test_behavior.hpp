#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_proxy_box_with_hit_test_behavior.generated.h"
#endif

namespace skr::gui
{
sreflect_struct("guid": "56dbbcf9-bfca-47c3-a75b-94c9a884255e")
SKR_GUI_API RenderProxyBoxWithHitTestBehavior : public RenderProxyBox {
    SKR_GENERATE_BODY()
    using Super = RenderProxyBox;

    // hit test
    bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;

    EHitTestBehavior hit_test_behavior = EHitTestBehavior::defer_to_child;
};
} // namespace skr::gui