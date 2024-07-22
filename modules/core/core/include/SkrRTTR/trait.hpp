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

// TODO. vtable 类型在 GENERATE_BODY 中直接生成
// TODO. vtable 的模板型萃取器也在 GENERATE_BODY 中生成生命，定义可以后置
// TODO. 萃取器的实现尽量使用 concept 来实现，实际萃取器中只是进行使用
// TODO. 是否考虑类似于 RTTR 的实现，将函数指针置于模板参数中，进行萃取

namespace skr
{
template <typename T>
struct ProxyObjectTraits {
    // TODO. validator (use concept?)
    // method -> t->*m()
    // static -> m(t)
    // field(getter/setter only) -> t->f
    //
    // 分为两部分，存在性验证和签名验证，签名验证可以被抽象，存在性验证则必须用 Validator/Concept 来实现
};
} // namespace skr