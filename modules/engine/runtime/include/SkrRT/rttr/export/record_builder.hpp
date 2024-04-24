#pragma once
#include "SkrRT/rttr/export/export_data.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct CtorBuilder;
template <typename T, typename Backend>
struct MethodBuilder;
template <typename T, typename Backend>
struct StaticMethodBuilder;

template <typename T, typename Backend>
struct RecordBuilder {
    RecordBuilder(RecordData* data);

    // basic info
    RecordBuilder& name(String name);
    RecordBuilder& basic_info();

    // ctor
    template <typename... Args>
    CtorBuilder<T, Backend>& ctor();

    // base & interface
    template <typename... Bases>
    RecordBuilder& bases();

    // method & field
    template <auto func>
    MethodBuilder<T, Backend>& method(String name);
    template <typename Func, Func func>
    MethodBuilder<T, Backend>& method(String name);
    template <auto field>
    RecordBuilder& field(String name);

    // static method & field
    template <auto func>
    StaticMethodBuilder<T, Backend>& static_method(String name);
    template <typename Func, Func func>
    StaticMethodBuilder<T, Backend>& static_method(String name);
    template <auto Field>
    RecordBuilder& static_field(String name);

    // custom parameter

protected:
    RecordData* _data;
};
template <typename T, typename Backend>
struct CtorBuilder : RecordBuilder<T, Backend> {
    inline CtorBuilder& param(uint64_t index, String name, ParamModifier modifier = ParamModifier::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->ctor_data.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};
template <typename T, typename Backend>
struct MethodBuilder : RecordBuilder<T, Backend> {
    inline MethodBuilder& param(uint64_t index, String name, ParamModifier modifier = ParamModifier::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->methods.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};
template <typename T, typename Backend>
struct StaticMethodBuilder : RecordBuilder<T, Backend> {
    inline StaticMethodBuilder& param(uint64_t index, String name, ParamModifier modifier = ParamModifier::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->static_methods.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};

}; // namespace skr::rttr

namespace skr::rttr
{
template <typename T, typename Backend>
inline RecordBuilder<T, Backend>::RecordBuilder(RecordData* data)
    : _data(data)
{
}

// ctor
template <typename T, typename Backend>
template <typename... Args>
inline CtorBuilder<T, Backend>& RecordBuilder<T, Backend>::ctor()
{
    auto& ctor_data = _data->ctor_data.emplace().ref();
    ctor_data.fill_signature<Args...>();
    ctor_data.invoke = nullptr;
    return *reinterpret_cast<CtorBuilder<T, Backend>*>(this);
}

// base & interface
template <typename T, typename Backend>
template <typename... Bases>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::bases()
{
    _data->bases_data.append({ BaseData::Make<T, Bases>()... });
    return *this;
}

// method & field
template <typename T, typename Backend>
template <auto func>
inline MethodBuilder<T, Backend>& RecordBuilder<T, Backend>::method(String name)
{
    auto& method_data = _data->methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature<T>(func);
    return *reinterpret_cast<MethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <typename Func, Func func>
inline MethodBuilder<T, Backend>& RecordBuilder<T, Backend>::method(String name)
{
    auto& method_data = _data->methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature<T>(func);
    return *reinterpret_cast<MethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <auto field>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::field(String name)
{
    auto& field_data = _data->fields.emplace().ref();
    field_data.name  = std::move(name);
    field_data.fill_signature(field);
    return *this;
}

// static method & field
template <typename T, typename Backend>
template <auto func>
inline StaticMethodBuilder<T, Backend>& RecordBuilder<T, Backend>::static_method(String name)
{
    auto& method_data = _data->static_methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    return *reinterpret_cast<StaticMethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <typename Func, Func func>
inline StaticMethodBuilder<T, Backend>& RecordBuilder<T, Backend>::static_method(String name)
{
    auto& method_data = _data->static_methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    return *reinterpret_cast<StaticMethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <auto Field>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::static_field(String name)
{
    auto& field_data = _data->static_fields.emplace().ref();
    field_data.name  = std::move(name);
    field_data.fill_signature(Field);
    return *this;
}
} // namespace skr::rttr