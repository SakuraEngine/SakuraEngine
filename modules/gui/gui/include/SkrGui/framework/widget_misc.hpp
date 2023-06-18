#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

// builders
namespace skr::gui
{
template <class TWidget>
struct WidgetBuilder {
    const char*  file;
    const size_t line;

    static_assert(std::is_base_of_v<Widget, TWidget>, "bad type, must be widget");

    template <class F>
    TWidget* operator<<=(F&& f)
    {
        TWidget* widget = SKR_GUI_NEW<TWidget>();
        widget->pre_construct();
        f(*widget);
        widget->post_construct();
        return widget;
    };
};

template <class T>
struct SlotBuilder {
    const char*  file;
    const size_t line;
    T&           slot_list;

    template <class F>
    void operator<<=(F&& f)
    {
        auto& slot = slot_list.emplace_back();
        f(slot);
    }
};

template <class T>
struct ParamBuilder {
    template <class F>
    T operator<<=(F&& f)
    {
        T params;
        f(params);
        return std::move(params);
    }
};

} // namespace skr::gui

#define SNewWidget(__TYPE) ::skr::gui::WidgetBuilder<__TYPE>{ __FILE__, __LINE__ } <<= [&](__TYPE & p)
#define SNewSlot(__SLOT_PARAM) ::skr::gui::SlotBuilder<decltype(__SLOT_PARAM)>{ __FILE__, __LINE__, __SLOT_PARAM } <<= [&](decltype(__SLOT_PARAM[0])& p)
#define SNewParam(__TYPE) ::skr::gui::ParamBuilder<__TYPE>{} <<= [&](__TYPE & p)

#define SKR_GUI_HIDE_CONSTRUCT(__PARAM) \
private:                                \
    using __PARAM;                      \
    using void construct(__PARAM);      \
                                        \
public:
