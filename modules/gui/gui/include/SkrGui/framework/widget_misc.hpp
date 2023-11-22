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

template <class TWidget>
struct StaticWidgetBuilder {
    const char*  file;
    const size_t line;
    bool         constructed = false;
    TWidget      widget;

    StaticWidgetBuilder(const char* file, const size_t line)
        : file(file)
        , line(line)
    {
    }

    static_assert(std::is_base_of_v<Widget, TWidget>, "bad type, must be widget");

    template <class F>
    inline TWidget* operator<<=(F&& f)
    {
        if (!constructed)
        {
            // TODO. 静态控件构造需要引入一种特殊的指针（或者向框架注册），因为其生命周期也是需要被框架控制的，不能作为简单的静态变量
            // TODO. 由于控件唯一性的影响，静态控件下的动态控件更新是异常的，所以应当阻止在静态控件下创建动态控件
            widget.pre_construct();
            f(widget);
            widget.post_construct();
            constructed = true;
        }
        return &widget;
    };
};
} // namespace skr::gui

#define SNewWidget(__TYPE) ::skr::gui::WidgetBuilder<__TYPE>{ __FILE__, __LINE__ } <<= [&](__TYPE & p)
#define SNewWidget_N(__TYPE, __NAME) ::skr::gui::WidgetBuilder<__TYPE>{ __FILE__, __LINE__ } <<= [&](__TYPE & __NAME)
#define SNewWidget_S(__TYPE)                                                            \
    [&]() -> ::skr::gui::StaticWidgetBuilder<__TYPE>& {                                 \
        static ::skr::gui::StaticWidgetBuilder<__TYPE> _instance{ __FILE__, __LINE__ }; \
        return _instance;                                                               \
    }() <<= [&](__TYPE & p)

#define SNewParam(__TYPE) ::skr::gui::ParamBuilder<__TYPE>{} <<= [&](__TYPE & p)
#define SNewParam_N(__TYPE, __NAME) ::skr::gui::ParamBuilder<__TYPE>{} <<= [&](__TYPE & __NAME)
