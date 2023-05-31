#pragma once
#include "type/type.h"
#include "platform/memory.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Widget, skr_gui_widget)

namespace skr::gui
{
template <typename W>
inline W* NewWidget(typename W::Params params)
{
    auto result = SkrNew<W>();
    result->construct(std::move(params));
    return result;
}

template <typename P>
inline auto NewWidget(P params) -> typename P::WidgetType
{
    auto result = SkrNew<P::WidgetType>();
    result->construct(std::move(params));
    return result;
}
}

