#pragma once

namespace skr
{
namespace io
{
struct IIORequest;

template <typename Component>
struct CID {
};

struct SKR_RUNTIME_API IORequestComponent {
    IORequestComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual ~IORequestComponent() SKR_NOEXCEPT;

protected:
    IIORequest* const request = nullptr;
};

template <class... Args>
class RequestComponents;

template <>
class RequestComponents<>
{
};

template <class T, class... Args>
class RequestComponents<T, Args...> : public RequestComponents<Args...>
{
    using Base = RequestComponents<Args...>;
    T Value_;

public:
    RequestComponents(T&& value, Args&&... args)
        : Value_(std::forward<T>(value))
        , Base(std::forward<Args>(args)...)
    {
    }
    T& Value()
    {
        return Value_;
    }
};

template <size_t k, class T, class... Args>
struct SelectComponent {
    using Type = typename SelectComponent<k - 1, Args...>::Type;
};

template <class T, class... Args>
struct SelectComponent<0, T, Args...> {
    using Type = T;
};

} // namespace io
} // namespace skr