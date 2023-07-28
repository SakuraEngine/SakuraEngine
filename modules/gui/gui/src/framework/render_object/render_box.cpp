#include "SkrGui/framework/render_object/render_box.hpp"
#include <algorithm>

namespace skr::gui
{
RenderBox::RenderBox() {}
RenderBox::~RenderBox() {}

// intrinsic size
float RenderBox::get_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_min_intrinsic_width(height);
}
float RenderBox::get_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_max_intrinsic_width(height);
}
float RenderBox::get_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_min_intrinsic_height(width);
}
float RenderBox::get_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_max_intrinsic_height(width);
}

// dry layout
Sizef RenderBox::get_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_dry_layout(constraints);
}

// intrinsic size
float RenderBox::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return 0.0f;
}

// dry layout
Sizef RenderBox::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    return Sizef::Zero();
}

void RenderBox::perform_resize() SKR_NOEXCEPT
{
    set_size(compute_dry_layout(constraints()));
    if (!size().is_finite()) { SKR_GUI_LOG_ERROR(u8"Box that [is_sized_by_parent() == true] must return finite size"); }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! old code !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// void RenderBox::before_draw(const DrawParams* params)
// {
//     RenderObject::before_draw(params);

//     if (auto canvas = draw_debug_rect ? params->canvas : nullptr)
//     {
//         debug_element->begin_frame(1.f);
//         {
//             debug_element->begin_path();
//             // actual region
//             debug_element->fill_color(128u, 0u, 0u, 128u);
//             debug_element->rect(pos.x, pos.y, size.width, size.height);
//             debug_element->fill();

//             // padding region
//             debug_element->begin_path();
//             debug_element->fill_color(128u, 0u, 0u, 64u);
//             const float Rate = 0.01f;
//             float       CanvasW, CanvasH;
//             canvas->get_size(&CanvasW, &CanvasH);
//             const auto DistX = Rate * std::min(CanvasW, CanvasH);
//             const auto DistY = Rate * std::min(CanvasW, CanvasH);
//             auto       X = pos.x - DistX;
//             const auto W = size.width + 2 * DistX;
//             auto       Y = pos.y - DistY;
//             const auto H = size.height + 2 * DistY;
//             debug_element->rect(X, Y, W, H);
//             debug_element->fill();
//         }
//         addElementToCanvas(params, debug_element);
//     }
// }

// bool RenderBox::hit_test(const Ray& point, HitTestRecord* record) const
// {
//     convert to local space
//     auto            origin = skr::math::load(point.origin);
//     auto            direction = skr::math::load(point.direction);
//     rtm::matrix4x4f matrix = skr::math::load(render_matrix);
//     auto            inv_matrix = rtm::matrix_inverse(matrix);
//     origin = rtm::matrix_mul_vector(origin, inv_matrix);
//     direction = rtm::matrix_mul_vector(direction, inv_matrix);
//     // check hit
//     auto min = skr::math::load(skr_float2_t{ pos.x, pos.y });
//     auto max = rtm::vector_add(skr::math::load(skr_float2_t{ pos.x, pos.y }), skr::math::load(skr_float2_t{ size.width, size.height }));
//     auto o = origin;
//     auto d = direction;
//     auto tmin = rtm::vector_div(rtm::vector_sub(min, o), d);
//     auto tmax = rtm::vector_div(rtm::vector_sub(max, o), d);
//     auto tmin_max = rtm::vector_max(tmin, tmax);
//     auto tmax_min = rtm::vector_min(tmin, tmax);
//     auto tmin_max_min = rtm::vector_min(tmin_max, tmax_min);
//     auto tmin_max_max = rtm::vector_max(tmin_max, tmax_min);
//     auto tmin_max_min_max = rtm::vector_max(tmin_max_min, tmin_max_max);
//     auto tmin_max_min_max_x = rtm::vector_get_x(tmin_max_min_max);
//     auto tmin_max_min_max_y = rtm::vector_get_y(tmin_max_min_max);
//     auto tmin_max_min_max_z = rtm::vector_get_z(tmin_max_min_max);
//     auto tmin_max_min_max_w = rtm::vector_get_w(tmin_max_min_max);
//     auto tmin_max_min_max_min = std::min<float>(tmin_max_min_max_x, std::min<float>(tmin_max_min_max_y, std::min<float>(tmin_max_min_max_z, tmin_max_min_max_w)));
//     auto tmin_max_min_max_max = std::max<float>(tmin_max_min_max_x, std::max<float>(tmin_max_min_max_y, std::max<float>(tmin_max_min_max_z, tmin_max_min_max_w)));
//     if (tmin_max_min_max_min > 0.f && tmin_max_min_max_min < tmin_max_min_max_max)
//     {
//         // record->hit = true;
//         // record->distance = tmin_max_min_max_min;
//         // record->hit_point = skr::math::store(rtm::vector_mul_add(d, rtm::vector_set(tmin_max_min_max_min, tmin_max_min_max_min, tmin_max_min_max_min, tmin_max_min_max_min), o));
//         // record->hit_normal = skr::math::store(rtm::vector_normalize(rtm::vector_mul_add(d
//         //     , rtm::vector_set(tmin_max_min_max_max, tmin_max_min_max_max, tmin_max_min_max_max, tmin_max_min_max_max)
//         //     , rtm::vector_mul_add(d, rtm::vector_set(tmin_max_min_max_min, tmin_max_min_max_min, tmin_max_min_max_min, tmin_max_min_max_min), o))));
//         return true;
//     }
//     return false;
// }

} // namespace skr::gui