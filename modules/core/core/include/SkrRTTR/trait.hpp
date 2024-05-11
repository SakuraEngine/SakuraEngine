#pragma once

// TODO. rename to Proxy
// TODO. 通过反射导出 Proxy 对象
// TODO. Interface 可以 Codegen 一个 InterfaceProxy 实现，用于 Interface 与 Proxy 的交互
// TODO. Proxy 支持 RC 的内存管理
// TODO. 轻量的传递可以由 Proxy::View 来进行

#ifdef __meta__
    #define SKR_TRAIT_MIXIN(type) \
        void* self;               \
        template <class T>        \
        type(T&& t)               \
            : self(&t)            \
        {                         \
        }
#else
    #define SKR_TRAIT_MIXIN(type)                        \
        void*                      self   = nullptr;     \
        const __VTABLE_##type##_t* vtable = nullptr;     \
        template <class T>                               \
        type(T& t) noexcept                              \
            : self(&t)                                   \
            , vtable(&__VTABLE_##type##_impl<T>::vtable) \
        {                                                \
        }
#endif