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

// in physical pixel
struct MonitorFeature {
    Recti                bound;
    EMonitorFeatureType  type;
    EMonitorFeatureState state;
};

// in physical pixel
struct MonitorInfo {
    String name      = {};
    String device_id = {};

    Sizei native_size    = {};
    Sizei max_resolution = {};

    Recti display_area = {};
    Recti work_area    = {};

    bool is_primary = false;

    Array<MonitorFeature> features = {};
};

// in physical pixel
struct DisplayMetrics {
    Recti primary_display_area = {};
    Recti primary_work_area    = {};
    Recti virtual_display_area = {};

    EdgeInsetsf title_safe_padding  = {};
    EdgeInsetsf action_safe_padding = {};

    Array<MonitorInfo> monitors = {};
};

} // namespace skr::gui