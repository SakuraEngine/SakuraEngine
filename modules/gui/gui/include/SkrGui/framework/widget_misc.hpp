#pragma once
#include "SkrGui/fwd_config.hpp"

// fixed stack
namespace skr::gui
{
struct SKR_GUI_API FixedStack {
    void* buffer;
    size_t size;
    size_t capacity;

    FixedStack(size_t capacity);
    ~FixedStack();

    void* allocate(size_t size);
    void free(size_t size);

    template <class T>
    T* allocate() { return (T*)allocate(sizeof(T)); }
    template <class T>
    T* allocate(size_t size) { return (T*)allocate(sizeof(T) * size); }
    template <class T>
    void free() { free(sizeof(T)); }
    template <class T>
    void free(size_t size) { free(sizeof(T) * size); }
};
SKR_GUI_API FixedStack& get_default_fixed_stack();

struct FixedStackScope {
    size_t top;
    FixedStack& stack;
    FixedStackScope(FixedStack& stack)
        : top(stack.size)
        , stack(stack)
    {
    }
    ~FixedStackScope()
    {
        stack.size = top;
    }
};

template <class T>
Span<T> make_arr(std::initializer_list<T> list)
{
    T* data = get_default_fixed_stack().allocate<T>(list.size());
    memcpy(data, list.begin(), list.size() * sizeof(T));
    return Span<T>{ data, list.size() };
}
} // namespace skr::gui

namespace skr::gui
{
struct Widget;

template <class T>
inline static constexpr bool is_widget_v = std::is_base_of_v<Widget, T>;

template <typename, class T>
struct is_widget_param : std::false_type {
};

template <class T>
struct is_widget_param<std::void_t<typename T::WidgetType>, T> : std::true_type {
};

template <class T>
inline static constexpr bool is_widget_param_v = is_widget_param<void, T>::value;

template <class T>
using widget_param_t = std::conditional_t<is_widget_param_v<T>, T, typename T::Params>;

template <class T>
struct WidgetBuilder {
    const char8_t* file;
    const size_t line;

    template <class F>
    T* operator<<=(F&& f)
    {
        FixedStackScope _(get_default_fixed_stack());

        typename T::Params params;
        f(params);

        auto widget = SKR_GUI_NEW<T>();
        widget->construct(std::move(params));
        return widget;
    };
};

template <class T>
struct WidgetBuilderParams {
    const char8_t* file;
    const size_t line;

    template <class F>
    class T::WidgetType* operator<<=(F&& f)
    {
        FixedStackScope _(get_default_fixed_stack());

        T params;
        f(params);

        auto widget = SKR_GUI_NEW<typename T::WidgetType>();
        widget->construct(std::move(params));
        return widget;
    };
};

template <typename W>
inline WidgetBuilder<W>& MakeBuilder(const char8_t* file, const size_t line) SKR_NOEXCEPT
{
    return WidgetBuilder<W>{ file, line };
}

template <typename W>
inline WidgetBuilderParams<W>& MakeBuilder(const char8_t* file, const size_t line) SKR_NOEXCEPT
{
    return WidgetBuilderParams<W>{ file, line };
}

} // namespace skr::gui

#define SNewWidget(type) MakeBuilder<type>(__FILE__, __LINE__) <<= [&](widget_param_t<type> & params)

#define SKR_GUI_HIDE_CONSTRUCT(__PARAM) \
private:                                \
    using __PARAM;                      \
    using void construct(__PARAM);      \
                                        \
public:
