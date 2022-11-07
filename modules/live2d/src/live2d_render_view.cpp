#include "skr_live2d/skr_live2d.h"
#include "platform/memory.h"
#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"
#include "live2d_helpers.hpp"
#include "skr_live2d/l2d_render_effect.h"

live2d_render_view_id skr_live2d_create_render_view()
{
    auto view = SkrNew<live2d_render_view_t>();
    skr_live2d_render_view_reset(view);
    return view;
}

void skr_live2d_render_view_reset(live2d_render_view_id view)
{
    view->clear_color = { 1.f, 1.f, 1.f, 0.f };
    new (&view->view_matrix) Csm::CubismViewMatrix();
    view->device_to_screen.LoadIdentity();
}

void skr_live2d_render_view_set_screen(live2d_render_view_id view, uint32_t width, uint32_t height)
{
    float ratio = static_cast<float>(width) / static_cast<float>(height);
    float left = -ratio;
    float right = ratio;
    float bottom = kLive2DViewLogicalLeft;
    float top = kLive2DViewLogicalRight;

    view->view_matrix.SetScreenRect(left, right, bottom, top);
    view->view_matrix.Scale(kLive2DViewScale, kLive2DViewScale);

    view->device_to_screen.LoadIdentity();
    if (width > height)
    {
        float screen_w = fabsf(right - left);
        view->device_to_screen.ScaleRelative(screen_w / width, -screen_w / width);
    }
    else
    {
        float screen_w = fabsf(top - bottom);
        view->device_to_screen.ScaleRelative(screen_w / height, -screen_w / height);
    }
    view->device_to_screen.TranslateRelative(width * -0.5f, height * -0.5f);

    // 表示範囲の設定
    view->view_matrix.SetMaxScale(kLive2DViewMaxScale); // 限界拡大率
    view->view_matrix.SetMinScale(kLive2DViewMinScale); // 限界縮小率

    // 表示できる最大範囲
    view->view_matrix.SetMaxScreenRect(
        kLive2DViewLogicalMaxLeft,
        kLive2DViewLogicalMaxRight,
        kLive2DViewLogicalMaxBottom,
        kLive2DViewLogicalMaxTop
    );
}

void skr_live2d_render_view_transform_screen(live2d_render_view_id view, float deviceX, float deviceY)
{
    view->device_to_screen.TransformX(deviceX);
    view->device_to_screen.TransformY(deviceY);
}

void skr_live2d_render_view_transform_view(live2d_render_view_id view, float deviceX, float deviceY)
{
    float screenX = view->device_to_screen.TransformX(deviceX); // 論理座標変換した座標を取得。
    view->view_matrix.InvertTransformX(screenX); // 拡大、縮小、移動後の値。

    float screenY = view->device_to_screen.TransformY(deviceY); // 論理座標変換した座標を取得。
    view->view_matrix.InvertTransformY(screenY); // 拡大、縮小、移動後の値。
}

void skr_live2d_free_render_view(live2d_render_view_id view)
{
    SkrDelete(view);
}