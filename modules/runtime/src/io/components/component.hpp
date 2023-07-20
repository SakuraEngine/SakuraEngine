#pragma once
#include "SkrRT/misc/types.h"

namespace skr {
namespace io {
struct IIORequest;

template <typename Component>
struct IORequestComponentTID { };

struct IORequestComponent
{
    IORequestComponent(IIORequest* const request) SKR_NOEXCEPT 
        : request(request) 
    {

    }
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT = 0;
protected:
    IIORequest* const request = nullptr;
};

template <class... Args>
class RequestComponents;

template <>
class RequestComponents<> {};

template <class T, class... Args>
class RequestComponents<T, Args...>: public RequestComponents<Args...> {
    using Base = RequestComponents<Args...>;
    T Value_;

public:
    RequestComponents(T&& value, Args&&... args)
        : Value_(std::forward<T>(value))
        , Base(std::forward<Args>(args)...)
    {
    }
    T& Value() {
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