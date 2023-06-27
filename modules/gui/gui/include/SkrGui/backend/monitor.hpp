#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
enum class EMonitorFeatureType
{
    Unknown,
    Fold,   // 柔性折叠屏（连续）的铰接部分
    Hinge,  // 刚性折叠屏（连续）的铰接部分
    Cutout, // 无法显示的部分（刘海等）
};
enum class EMonitorFeatureState
{
    Unknown,
    PostureFlat,       // 折叠屏完全展开
    PostureHalfOpened, // 折叠屏半开
};

struct MonitorFeature {
    Rect                 bound; // in physical unit
    EMonitorFeatureType  type;
    EMonitorFeatureState state;
};

struct MonitorInfo {
    String name;
    String id;

    int32_t width;  // in physical unit
    int32_t height; // in physical unit

    int32_t max_resolution_width;  // in physical unit
    int32_t max_resolution_height; // in physical unit

    Rect virtual_display_area; // in physical unit
    Rect virtual_work_area;    // in physical unit

    bool is_primary;

    Array<MonitorFeature> features;
};

struct DesktopInfo {
    int32_t primary_display_width;  // in physical unit
    int32_t primary_display_height; // in physical unit

    Rect primary_display_area; // in physical unit
    Rect virtual_display_area; // in physical unit

    // EdgeInset title_safe_padding;
    // EdgeInset action_safe_padding;

    Array<MonitorInfo> monitors;
};

} // namespace skr::gui