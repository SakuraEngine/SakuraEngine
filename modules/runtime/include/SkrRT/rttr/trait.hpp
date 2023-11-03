#pragma once

#ifdef __meta__
#define GENERATED_TRAIT_BODY(type) void* self; template<class T> type(T&& t) : self(&t) {}
#else
#define GENERATED_TRAIT_BODY(type) \
    void* self = nullptr; \
    const __VTABLE_##type* vtable = nullptr; \
    template<class T> \
    type(T& t) noexcept \
        : self(&t), vtable(&__VTABLE_##type##_HELPER<T>::vtable) {}
#endif