#pragma once
#include "SkrRTTR/iobject.hpp"

// TODO. 统一化的 RC 内存管理，是否重载局部的 new delete 更好

namespace skr::concepts
{
// can be used to RC
template <typename T>
concept RCObject = std::derived_from<T, ::skr::rttr::IObject>;
template <typename T>
concept NotRCObject = !RCObject<T>;

// rc cast
template <typename To, typename From>
concept RCStaticCast = std::convertible_to<From, To> && RCObject<To>;
template <typename To, typename From>
concept IRCStaticCast = std::convertible_to<From, To> && NotRCObject<To>;

} // namespace skr::concepts

namespace skr
{
template <concepts::RCObject T>
struct RC;
template <concepts::NotRCObject T>
struct IRC;
template <concepts::RCObject T>
struct WRC;
template <concepts::NotRCObject T>
struct IWRC;

template <concepts::RCObject T>
struct RC {
    // factory
    template <typename... Args>
    static RC New(Args&&... args);
    template <typename... Args>
    static RC NewZeroed(Args&&... args);

    // ctor & dtor
    RC();
    RC(std::nullptr_t);
    RC(T* p);
    ~RC();

    // copy & move
    template <std::convertible_to<T> U>
    RC(const RC<U>& other);
    template <std::convertible_to<T> U>
    RC(RC<U>&& other);

    // assign & move assign
    template <std::convertible_to<T> U>
    RC& operator=(const RC<U>& other);
    template <std::convertible_to<T> U>
    RC& operator=(RC<U>&& other);
    RC& operator=(std::nullptr_t);
    RC& operator=(T* p);

    // release & reset
    void reset();
    void reset(T* p);

    // get
    T*       raw_ptr() const;
    uint32_t ref_count() const;

    // validate
    explicit operator bool() const;
    bool     is_null() const;
    bool     is_valid() const;

    // cast
    template <concepts::RCStaticCast<T> U>
    RC<U> static_cast_to() const;
    template <concepts::IRCStaticCast<T> U>
    IRC<U> static_cast_to() const;
    template <concepts::RCObject U>
    RC<U> dynamic_cast_to() const;
    template <concepts::NotRCObject U>
    IRC<U> dynamic_cast_to() const;

private:
    T* _object = nullptr;
};

template <concepts::NotRCObject T>
struct IRC {
    // ctor & dtor

    // copy & move

    // assign & move assign

    // release & reset

    // get

    // validate

    // cast

private:
    ::skr::rttr::IObject* _object    = nullptr;
    T*                    _interface = nullptr;
};

struct WRCCounter {
    ::skr::rttr::IObject* object         = nullptr;
    uint32_t              weak_ref_count = 0;
};
template <concepts::RCObject T>
struct WRC {

private:
    WRCCounter* _counter;
};

template <concepts::NotRCObject T>
struct IWRC {

private:
    WRCCounter* _counter   = nullptr;
    T*          _interface = nullptr;
};

} // namespace skr
