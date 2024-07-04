#pragma once
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
struct SKR_CORE_API IObject {
    virtual ~IObject() = default;

    //=> IObject API
    // TODO. 融合 RC 功能，去除虚继承，使用 IRC 模板来解决 Interface 无法回转 Cast 的问题
    // 之前使用虚继承的原因如下：
    //  1. interface 也需要继承 iobject 这样才能正确的向 iobject 回转
    //  2. IEmbeddedRC 与 IObject 是两个接口，对于需要应用 RC 的 Interface 和 Object 之间形成了冲突
    //  3. 一些复杂的 Interface 组合，即尽量细化的接口
    // 对应的解决方案：
    //  1. 增加一类模板接口，同时存储一个 IObject 指针和目标 Interface 指针用于满足存储需求
    //  2. 合并接口，因为大多数情况下需求都是重叠的，即便不愿意引入 codegen，也可以空置 iobject 类型的接口使用部分功能
    //  3. 这类复杂接口主要目有两类
    //      a. 高级语言中没有模板，需要通过 inteface 来约束接口
    //      b. 表达一些（有交集的）组合概念和公有特征
    //     解决方案为：
    //      a. 模板可以过滤掉一大部分接口约束的需求，典型案例为容器设计，通过 const + concept 能规避掉所有的接口抽象
    //      b. 公有特征通过 Proxy 抽取，尽量避免有交集的组合概念，多数情况下并不会发生冲突
    // 带来的局限:
    //  1. interface 无法进行 dynamic_cast，需要通过 IRC 进行，这就意味着纯 interface 指针无法进行回转
    virtual GUID  iobject_get_typeid() const   = 0;
    virtual void* iobject_get_head_ptr() const = 0;
    // virtual uint32_t embedded_rc_add_ref()         = 0;
    // virtual uint32_t embedded_rc_release_ref()     = 0;
    // virtual uint32_t embedded_rc_ref_count() const = 0;
    // virtual void     embedded_rc_delete()          = 0; // TODO. pooling 的释放方式可以由具体实现控制，但是默认的释放方式就固定了，只能使用 skr 的内存管理
    //=> IObject API

    //=> Helper API
    template <typename TO>
    inline TO* type_cast()
    {
        auto  from_type = get_type_from_guid(this->iobject_get_typeid());
        void* cast_p    = from_type->cast_to(::skr::rttr::type_id_of<TO>(), this->iobject_get_head_ptr());
        return reinterpret_cast<TO*>(cast_p);
    }
    template <typename TO>
    inline const TO* type_cast() const
    {
        return const_cast<IObject*>(this)->type_cast<TO>();
    }
    template <typename TO>
    inline const TO* type_cast_fast() const { return type_cast<TO>(); }
    template <typename TO>
    inline TO* type_cast_fast() { return type_cast<TO>(); }
    template <typename TO>
    inline bool type_is() const noexcept
    {
        return type_cast<TO>() != nullptr;
    }
    inline bool type_is(const GUID& guid) const
    {
        auto from_type = get_type_from_guid(this->iobject_get_typeid());
        return from_type->cast_to(guid, this->iobject_get_head_ptr());
    }
    //=> Helper API

    // disable default new/delete, please use SkrNewObj/SkrDeleteObj or RC<T> instead
    inline static void* operator new(std::size_t, void* p) { return p; }
    static void* operator new(size_t)   = delete;
    static void* operator new[](size_t) = delete;
};
} // namespace skr::rttr

SKR_RTTR_TYPE(IObject, "19246699-65f8-4c0b-a82e-7886a0cb315d")

#ifndef __meta__
    #define SKR_RTTR_GENERATE_BODY()                                                  \
        GUID iobject_get_typeid() const override                                      \
        {                                                                             \
            using namespace skr::rttr;                                                \
            using ThisType = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
            return type_id_of<ThisType>();                                            \
        }                                                                             \
        void* iobject_get_head_ptr() const override { return const_cast<void*>((const void*)this); }
#else
    #define SKR_RTTR_GENERATE_BODY()                                  \
        GUID  iobject_get_typeid() const override { return nullptr; } \
        void* iobject_get_head_ptr() const override { return nullptr; }
#endif