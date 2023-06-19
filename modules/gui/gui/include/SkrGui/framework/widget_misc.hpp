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
    inline TWidget* operator<<=(F&& f)
    {
        TWidget* widget = SKR_GUI_NEW<TWidget>();
        widget->pre_construct();
        f(*widget);
        widget->post_construct();
        return widget;
    };
};

template <class TChildList, class TWidget>
struct ChildBuilder {
    const char*  file;
    const size_t line;
    TChildList&  child_list;

    static_assert(std::is_base_of_v<Widget, TWidget>, "bad type, must be widget");

    template <class F>
    void operator<<=(F&& f)
    {
        TWidget* widget = SKR_GUI_NEW<TWidget>();
        widget->pre_construct();
        f(*widget);
        widget->post_construct();

        child_list.emplace_back(widget);
    }
};

template <class T>
struct ParamBuilder {
    template <class F>
    inline T operator<<=(F&& f)
    {
        T params;
        f(params);
        return std::move(params);
    }
};

} // namespace skr::gui

#define SNewWidget(__TYPE) ::skr::gui::WidgetBuilder<__TYPE>{ __FILE__, __LINE__ } <<= [&](__TYPE & p)
#define SNewChild(__SLOT_PARAM, __CHILD_TYPE) ::skr::gui::ChildBuilder<decltype(__SLOT_PARAM), __CHILD_TYPE>{ __FILE__, __LINE__, __SLOT_PARAM } <<= [&](__CHILD_TYPE & p)
#define SNewParam(__TYPE) ::skr::gui::ParamBuilder<__TYPE>{} <<= [&](__TYPE & p)