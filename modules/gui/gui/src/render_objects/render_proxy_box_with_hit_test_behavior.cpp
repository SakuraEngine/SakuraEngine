#include "SkrGui/framework/render_object/render_proxy_box_with_hit_test_behavior.hpp"

namespace skr::gui
{
// hit test
bool RenderProxyBoxWithHitTestBehavior::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    if (size().contains(local_position))
    {
        if (child()->hit_test(result, local_position) || hit_test_behavior == EHitTestBehavior::opaque)
        {
            result->add(this);
            return true;
        }
        else if (hit_test_behavior == EHitTestBehavior::transparent)
        {
            result->add(this);
            return false;
        }
    }
    return false;
}
} // namespace skr::gui