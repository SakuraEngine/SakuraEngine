#pragma once
#include "platform/memory.h"

#define SKR_GUI_HIDE_CONSTRUCT(__PARAM) \
private:                                \
    using __PARAM;                      \
    using void construct(__PARAM)

namespace skr::gui
{
template <typename W>
inline W* NewWidget(typename W::Params params) SKR_NOEXCEPT
{
    auto result = SkrNew<W>();
    result->construct(std::move(params));
    return result;
}

template <typename P>
inline auto NewWidget(P params) SKR_NOEXCEPT->typename P::WidgetType
{
    auto result = SkrNew<P::WidgetType>();
    result->construct(std::move(params));
    return result;
}
} // namespace skr::gui
