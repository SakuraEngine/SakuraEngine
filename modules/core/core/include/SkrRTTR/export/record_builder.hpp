#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/extern_methods.hpp"
#include "SkrRTTR/export/export_helper.hpp"

namespace skr::rttr
{
template <typename T>
struct CtorBuilder;
template <typename T>
struct MethodBuilder;
template <typename T>
struct StaticMethodBuilder;
template <typename T>
struct ExternMethodBuilder;

template <typename T>
struct RecordBuilder {
    RecordBuilder(RecordData* data);

    // basic info
    RecordBuilder& basic_info();

    // ctor
    template <typename... Args>
    CtorBuilder<T>& ctor();

    // base & interface
    template <typename... Bases>
    RecordBuilder& bases();

    // method & field
    template <auto func>
    MethodBuilder<T>& method(String name);
    template <typename Func, Func func>
    MethodBuilder<T>& method(String name);
    template <auto field>
    RecordBuilder& field(String name);

    // static method & field
    template <auto func>
    StaticMethodBuilder<T>& static_method(String name);
    template <typename Func, Func func>
    StaticMethodBuilder<T>& static_method(String name);
    template <auto field>
    RecordBuilder& static_field(String name);

    // extern method
    template <auto func>
    ExternMethodBuilder<T>& extern_method(String name);
    template <typename Func, Func func>
    StaticMethodBuilder<T>& extern_method(String name);

protected:
    RecordData* _data;
};
template <typename T>
struct CtorBuilder : RecordBuilder<T> {
    inline CtorBuilder& param(uint64_t index, String name, ParamFlag modifier = ParamFlag::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->ctor_data.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};
template <typename T>
struct MethodBuilder : RecordBuilder<T> {
    inline MethodBuilder& param(uint64_t index, String name, ParamFlag modifier = ParamFlag::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->methods.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};
template <typename T>
struct StaticMethodBuilder : RecordBuilder<T> {
    inline StaticMethodBuilder& param(uint64_t index, String name, ParamFlag modifier = ParamFlag::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->static_methods.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};
template <typename T>
struct ExternMethodBuilder : RecordBuilder<T> {
    inline ExternMethodBuilder& param(uint64_t index, String name, ParamFlag modifier = ParamFlag::In, ParamData::MakeDefaultFunc default_func = nullptr)
    {
        auto& param_data        = this->_data->extern_methods.back().param_data[index];
        param_data.name         = std::move(name);
        param_data.modifier     = modifier;
        param_data.make_default = default_func;
        return *this;
    }
};

} // namespace skr::rttr

namespace skr::rttr
{
template <typename T>
inline RecordBuilder<T>::RecordBuilder(RecordData* data)
    : _data(data)
{
}

// basic info
template <typename T>
inline RecordBuilder<T>& RecordBuilder<T>::basic_info()
{
    // split namespace
    String              name = RTTRTraits<T>::get_name();
    sequence<text_view> splitted;
    auto                count = name.split(u8"::", splitted);

    // last part is name
    _data->name = splitted.access_at(splitted.size() - 1);

    // fill namespace
    if (count > 1)
    {
        _data->name_space.reserve(count - 1);
        for (auto i = 0; i < count - 1; ++i)
        {
            _data->name_space.push_back(splitted.access_at(i));
        }
    }

    // fill type id
    _data->type_id = RTTRTraits<T>::get_guid();

    // fill size & alignment
    _data->size      = sizeof(T);
    _data->alignment = alignof(T);

    // TODO. fill default ctor
    // std::is_default_constructible_v<T>

    // TODO. fill copy ctor
    // std::is_copy_constructible_v<T>

    // TODO. fill move ctor
    // std::is_move_constructible_v<T>

    // TODO. fill assign operator
    // std::is_copy_assignable_v<T>

    // TODO. fill move assign operator
    // std::is_move_assignable_v<T>

    // TODO. fill dtor
    // std::is_destructible_v<T>
    _data->dtor_data.native_invoke = ExportHelper::export_dtor<T>();

    return *this;
}

// ctor
template <typename T>
template <typename... Args>
inline CtorBuilder<T>& RecordBuilder<T>::ctor()
{
    auto& ctor_data = _data->ctor_data.emplace().ref();
    ctor_data.fill_signature<Args...>();
    ctor_data.native_invoke = ExportHelper::export_ctor<T, Args...>();
    return *reinterpret_cast<CtorBuilder<T>*>(this);
}

// base & interface
template <typename T>
template <typename... Bases>
inline RecordBuilder<T>& RecordBuilder<T>::bases()
{
    _data->bases_data.append({ BaseData::Make<T, Bases>()... });
    return *this;
}

// method & field
template <typename T>
template <auto func>
inline MethodBuilder<T>& RecordBuilder<T>::method(String name)
{
    auto& method_data = _data->methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    method_data.native_invoke = ExportHelper::export_method<func>();
    return *reinterpret_cast<MethodBuilder<T>*>(this);
}
template <typename T>
template <typename Func, Func func>
inline MethodBuilder<T>& RecordBuilder<T>::method(String name)
{
    return method<func>(std::move(name));
}
template <typename T>
template <auto field>
inline RecordBuilder<T>& RecordBuilder<T>::field(String name)
{
    auto& field_data = _data->fields.emplace().ref();
    field_data.name  = std::move(name);
    field_data.fill_signature<field>(field);
    return *this;
}

// static method & field
template <typename T>
template <auto func>
inline StaticMethodBuilder<T>& RecordBuilder<T>::static_method(String name)
{
    auto& method_data = _data->static_methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    method_data.native_invoke = ExportHelper::export_static_method<func>();
    return *reinterpret_cast<StaticMethodBuilder<T>*>(this);
}
template <typename T>
template <typename Func, Func func>
inline StaticMethodBuilder<T>& RecordBuilder<T>::static_method(String name)
{
    return static_method<func>(std::move(name));
}
template <typename T>
template <auto field>
inline RecordBuilder<T>& RecordBuilder<T>::static_field(String name)
{
    auto& field_data   = _data->static_fields.emplace().ref();
    field_data.name    = std::move(name);
    field_data.address = reinterpret_cast<void*>(field);
    field_data.fill_signature(field);
    return *this;
}

// extern method
template <typename T>
template <auto func>
inline ExternMethodBuilder<T>& RecordBuilder<T>::extern_method(String name)
{
    auto& method_data = _data->extern_methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    method_data.native_invoke = ExportHelper::export_extern_method<func>();
    return *reinterpret_cast<ExternMethodBuilder<T>*>(this);
}
template <typename T>
template <typename Func, Func func>
inline StaticMethodBuilder<T>& RecordBuilder<T>::extern_method(String name)
{
    return extern_method<func>();
}
} // namespace skr::rttr