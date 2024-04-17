#pragma once
#include "SkrScript/export_data.hpp"

// TODO. RTTR 需要增加 interface 标记，interface mode 下会有不同的生成行为
// TODO. 这里可以联动 Trait，Trait 和 TS 亲和度非常高，TS 对象可以直接转化为 Trait 对象
//  Trait 名字不直观，Trait => Pattern 比较好
//  Trait 与导出器的联动方式与 RTTR 保持一致
namespace skr::script
{
template <typename T, typename Backend>
struct RecordBuilder {
    // ctor
    template <typename... Args>
    void ctor(T::T(Args...));

    // base & interface
    template <typename Base>
    void base(String name);
    template <typename Interface>
    void implement(String name);

    // method & field
    template <typename Ret, typename... Args>
    void method(String name, Ret (T::*method)(Args...));
    template <typename Field>
    void field(String name, Field T::*field);

    // static method & field
    template <typename Ret, typename... Args>
    void static_method(String name, Ret (*method)(Args...));
    template <typename Field>
    void static_field(String name, Field* field);
};

template <typename T, typename Backend>
struct InterfaceBuilder {
    // interface base
    template <typename Base>
    void base(String name);

    // method & field
    template <typename Ret, typename... Args>
    void method(String name, Ret (T::*method)(Args...));
};

// we DO NOT support global variable BY DESIGN
// because c++ global variable will cause a lot of trouble
// if you want to expose a global variable, please use a function warp it
// e.g. int& global_variable() { return global_variable; }

} // namespace skr::script