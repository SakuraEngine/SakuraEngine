#pragma once
#include "SkrGui/framework/render_object/render_window.hpp"
#include "SkrGui/framework/render_object/render_native_window.generated.h"

namespace skr::gui
{
struct INativeWindow;
struct [[sattr(guid = "1681d4be-cb32-4b65-9f07-9f143ebe1c6e"
)]] SKR_GUI_API RenderNativeWindow : public RenderWindow
{
    SKR_GENERATE_BODY(RenderNativeWindow)
    RenderNativeWindow(INativeWindow* native_window);

    NotNull<OffsetLayer*> update_layer(OffsetLayer* old_layer) override;

    void prepare_initial_frame() SKR_NOEXCEPT;
    inline void setup_owner(BuildOwner* owner) SKR_NOEXCEPT { _owner = owner; }

    bool hit_test(HitTestResult* result, Offsetf local_position);
};
} // namespace skr::gui