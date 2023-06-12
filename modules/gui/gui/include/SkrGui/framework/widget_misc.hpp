#pragma once
#include "SkrGui/fwd_config.hpp"

// fixed stack
namespace skr::gui
{
struct SKR_GUI_API FixedStack {
    void*  buffer;
    size_t size;
    size_t capacity;

    FixedStack(size_t capacity);
    ~FixedStack();

    void* allocate(size_t size);
    void  free(size_t size);

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
    size_t      top;
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

namespace __helper
{
// is detected
template <typename, template <typename...> class Op, typename... T>
struct is_detected_impl : std::false_type {
};
template <template <typename...> class Op, typename... T>
struct is_detected_impl<std::void_t<Op<T...>>, Op, T...> : std::true_type {
};
template <template <typename...> class Op, typename... T>
using is_detected = is_detected_impl<void, Op, T...>;
template <template <typename...> class Op, typename... T>
inline constexpr bool is_detected_v = is_detected<Op, T...>::value;

// checker
template <typename T, typename Params>
using has_construct = decltype(std::declval<T>().construct(std::declval<Params>()));
template <typename T, typename Params>
inline static constexpr bool has_construct_v = is_detected_v<has_construct, T, Params>;

template <typename T>
using has_default_params = typename T::Params;
template <typename T>
inline static constexpr bool has_default_params_v = is_detected_v<has_default_params, T>;

template <typename T>
inline static constexpr bool has_default_constructor_v = has_construct_v<T, typename T::Params>;

template <typename T>
using has_widget_type = typename T::WidgetType;
template <typename T>
inline static constexpr bool has_widget_type_v = is_detected_v<has_widget_type, T>;

template <class T>
inline static constexpr bool is_widget_v = std::is_base_of_v<Widget, T>;

template <class T>
inline static constexpr auto param_checker()
{
    if constexpr (__helper::is_widget_v<T>)
    {
        static_assert(__helper::has_default_params_v<T>, "Widget must have default params");
        static_assert(__helper::has_default_constructor_v<T>, "Widget must have default constructor");

        using ParamsType = typename T::Params;
        return ParamsType{};
    }
    else if constexpr (__helper::has_widget_type_v<T>)
    {
        static_assert(__helper::has_construct_v<typename T::WidgetType, T>, "Widget must have construct for params");

        using ParamsType = T;
        return ParamsType{};
    }
    else
    {
        return int{};
    }
}
template <class T>
using params_type_t = decltype(param_checker<T>());

template <class T>
using has_slot = typename T::Slot;
template <class T>
inline static constexpr bool has_slot_v = is_detected_v<has_slot, T>;

template <class T>
struct slot_type {
    using type = T;
};

template <class T>
struct slot_type<Span<T>> {
    using type = T;
};

template <class T>
using slot_type_t = typename slot_type<T>::type;

} // namespace __helper

template <class T>
struct WidgetBuilder {
    const char*  file;
    const size_t line;

    template <class F>
    auto operator<<=(F&& f)
    {
        FixedStackScope _(get_default_fixed_stack());

        if constexpr (__helper::is_widget_v<T>)
        {
            static_assert(__helper::has_default_params_v<T>, "Widget must have default params");
            static_assert(__helper::has_default_constructor_v<T>, "Widget must have default constructor");

            using WidgetType = T;
            using ParamsType = typename T::Params;

            ParamsType params;
            f(params);

            auto widget = SKR_GUI_NEW<WidgetType>();
            widget->construct(std::move(params));
            return widget;
        }
        else if constexpr (__helper::has_widget_type_v<T>)
        {
            static_assert(__helper::has_construct_v<typename T::WidgetType, T>, "Widget must have construct for params");

            using WidgetType = typename T::WidgetType;
            using ParamsType = T;

            ParamsType params;
            f(params);

            auto widget = SKR_GUI_NEW<WidgetType>();
            widget->construct(std::move(params));
            return widget;
        }
        else
        {
            static_assert(__helper::is_widget_v<T> || __helper::has_widget_type_v<T>, "bad type, must be widget or widget params");
        }
    };
};

template <class T>
struct SlotBuilder {
    const char*  file;
    const size_t line;
    T&           slot_list;

    template <class F>
    __helper::slot_type_t<T> operator<<=(F&& f)
    {
        static_assert(__helper::has_slot_v<T>, "widget must have slot");

        // TODO. more safe span
        using SlotType = __helper::slot_type_t<T>;
        static_assert(!std::is_same_v<T, __helper::slot_type_t<T>>, "bad slot type");
        SlotType* slot = get_default_fixed_stack().allocate<SlotType>();
        f(*slot);
        slot_list = { slot_list.data(), slot_list.size() + 1 };
        return std::move(slot);
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

#define SNewWidget(__TYPE) ::skr::gui::WidgetBuilder<__TYPE>{ __FILE__, __LINE__ } <<= [&](::skr::gui::__helper::params_type_t<__TYPE> & p)
#define SNewSlot(__SLOT_PARAM) ::skr::gui::SlotBuilder<decltype(__SLOT_PARAM)>{ __FILE__, __LINE__, __SLOT_PARAM } <<= [&](::skr::gui::__helper::slot_type_t<decltype(__SLOT_PARAM)> & p)
#define SNewParam(__TYPE) ::skr::gui::ParamBuilder<__TYPE>{} <<= [&](__TYPE & p)

#define SKR_GUI_HIDE_CONSTRUCT(__PARAM) \
private:                                \
    using __PARAM;                      \
    using void construct(__PARAM);      \
                                        \
public:
