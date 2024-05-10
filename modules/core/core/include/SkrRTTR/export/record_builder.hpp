#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/cpp_extern_methods.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct CtorBuilder;
template <typename T, typename Backend>
struct MethodBuilder;
template <typename T, typename Backend>
struct StaticMethodBuilder;
template <typename T, typename Backend>
struct ExternMethodBuilder;

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
    template <auto field>
    RecordBuilder& static_field(String name);

    // extern method
    template <auto func>
    ExternMethodBuilder<T, Backend>& extern_method(String name);
    template <typename Func, Func func>
    StaticMethodBuilder<T, Backend>& extern_method(String name);

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
template <typename T, typename Backend>
struct ExternMethodBuilder : RecordBuilder<T, Backend> {
    inline ExternMethodBuilder& param(uint64_t index, String name, ParamModifier modifier = ParamModifier::In, ParamData::MakeDefaultFunc default_func = nullptr)
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
template <typename T, typename Backend>
inline RecordBuilder<T, Backend>::RecordBuilder(RecordData* data)
    : _data(data)
{
}

// basic info
template <typename T, typename Backend>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::name(String name)
{
    // split namespace
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

    return *this;
}
template <typename T, typename Backend>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::basic_info()
{
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
    _data->dtor_data.invoke = Backend::template export_dtor<T>();

    return *this;
}

// ctor
template <typename T, typename Backend>
template <typename... Args>
inline CtorBuilder<T, Backend>& RecordBuilder<T, Backend>::ctor()
{
    auto& ctor_data = _data->ctor_data.emplace().ref();
    ctor_data.fill_signature<Args...>();
    ctor_data.invoke = Backend::template export_ctor<T, Args...>();
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
    method_data.fill_signature(func);
    method_data.invoke = Backend::template export_method<func>();
    return *reinterpret_cast<MethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <typename Func, Func func>
inline MethodBuilder<T, Backend>& RecordBuilder<T, Backend>::method(String name)
{
    return method<func>(std::move(name));
}
template <typename T, typename Backend>
template <auto field>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::field(String name)
{
    auto& field_data = _data->fields.emplace().ref();
    field_data.name  = std::move(name);
    field_data.fill_signature<field>(field);
    field_data.setter = Backend::template export_field_setter<field>();
    field_data.getter = Backend::template export_field_getter<field>();
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
    method_data.invoke = Backend::template export_static_method<func>();
    return *reinterpret_cast<StaticMethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <typename Func, Func func>
inline StaticMethodBuilder<T, Backend>& RecordBuilder<T, Backend>::static_method(String name)
{
    return static_method<func>(std::move(name));
}
template <typename T, typename Backend>
template <auto field>
inline RecordBuilder<T, Backend>& RecordBuilder<T, Backend>::static_field(String name)
{
    auto& field_data   = _data->static_fields.emplace().ref();
    field_data.name    = std::move(name);
    field_data.address = reinterpret_cast<void*>(field);
    field_data.fill_signature(field);
    field_data.setter = Backend::template export_static_field_setter<field>();
    field_data.getter = Backend::template export_static_field_getter<field>();
    return *this;
}

// extern method
template <typename T, typename Backend>
template <auto func>
inline ExternMethodBuilder<T, Backend>& RecordBuilder<T, Backend>::extern_method(String name)
{
    auto& method_data = _data->extern_methods.emplace().ref();
    method_data.name  = std::move(name);
    method_data.fill_signature(func);
    method_data.invoke = Backend::template export_extern_method<func>();
    return *reinterpret_cast<ExternMethodBuilder<T, Backend>*>(this);
}
template <typename T, typename Backend>
template <typename Func, Func func>
inline StaticMethodBuilder<T, Backend>& RecordBuilder<T, Backend>::extern_method(String name)
{
    return extern_method<func>();
}
} // namespace skr::rttr