#pragma once
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#ifndef __meta__
    #include "SkrCore/SkrRTTR/iobject.generated.h"
#endif

// iobject
namespace skr::rttr
{
// TODO. 暂时只有 GUI 在用，修改考虑 GUI 重构即可
// TODO. 需要对 core 开启 codegen 功能, 因为 core 内部的部分功能也需要依赖 codegen
sreflect_struct("guid": "3740620f-714d-4d78-b47e-095f256ba4a7")
SKR_CORE_API IObject {
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

// object
namespace skr::rttr
{
}

// rc concepts
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

// reference count for object
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

// reference count for interface object
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

// weak reference count temp object
struct WRCCounter {
    ::skr::rttr::IObject* object         = nullptr;
    uint32_t              weak_ref_count = 0;
};

// weak reference for object
template <concepts::RCObject T>
struct WRC {

private:
    WRCCounter* _counter;
};

// weak reference for interface object
template <concepts::NotRCObject T>
struct IWRC {

private:
    WRCCounter* _counter   = nullptr;
    T*          _interface = nullptr;
};

} // namespace skr
