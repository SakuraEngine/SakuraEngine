#pragma once

#ifdef __meta__
#define TRAIT_MIXIN(type) void* self; template<class T> type(T&& t) : self(&t) {}
#else
#define TRAIT_MIXIN(type) \
    void* self = nullptr; \
    const __VTABLE_##type##_t* vtable = nullptr; \
    template<class T> \
    type(T& t) noexcept \
        : self(&t), vtable(&__VTABLE_##type<T>) {}
#endif