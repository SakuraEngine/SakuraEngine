#pragma once
#include "SkrGui/framework/element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, StatelessWidget, skr_gui_stateless_widget)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, StatefulWidget, skr_gui_stateful_widget)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, ProxyWidget, skr_gui_proxy_widget)

namespace skr {
namespace gui {

struct SKR_GUI_API ComponentElement : public Element
{

    Element* _child = nullptr;
};

struct SKR_GUI_API StatelessElement : public ComponentElement
{

};

struct SKR_GUI_API StatefulElement : public ComponentElement
{

};

struct SKR_GUI_API ProxyElement : public ComponentElement
{

};

} // namespace gui
} // namespace skr