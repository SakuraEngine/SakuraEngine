#pragma once
#include "SkrRT/rttr/export/export_data.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T, typename Backend>
struct RecordBuilder {
    inline RecordBuilder(RecordData* data)
        : _data(data)
    {
    }

    // ctor
    template <typename... Args>
    inline RecordBuilder& ctor()
    {
        auto& ctor_data      = _data->ctor_data.emplace().ref();
        ctor_data.param_type = { TypeIdentifier::Make<Args>()... };
        ctor_data.invoke     = nullptr;
        return *this;
    }

    // base & interface
    template <typename Base>
    inline RecordBuilder& base()
    {
        _data->base_id = RTTRTraits<Base>::get_guid();
        return *this;
    }
    template <typename... Interfaces>
    inline RecordBuilder& implement()
    {
        _data->interface_ids.append({ RTTRTraits<Interfaces>::get_guid()... });
        return *this;
    }

    // method & field
    template <auto func>
    inline RecordBuilder& method(String name)
    {
        auto& method_data = _data->methods.emplace().ref();
        method_data.name  = std::move(name);
        method_data.fill_signature<T>(func);
        return *this;
    }
    template <typename Func, Func func>
    inline RecordBuilder& method(String name)
    {
        auto& method_data = _data->methods.emplace().ref();
        method_data.name  = std::move(name);
        method_data.fill_signature<T>(func);
        return *this;
    }
    template <auto field>
    inline RecordBuilder& field(String name)
    {
        auto& field_data = _data->fields.emplace().ref();
        field_data.name  = std::move(name);
        field_data.fill_signature(field);
        return *this;
    }

    // static method & field
    template <auto func>
    inline RecordBuilder& static_method(String name)
    {
        auto& method_data = _data->static_methods.emplace().ref();
        method_data.name  = std::move(name);
        method_data.fill_signature(func);
        return *this;
    }
    template <typename Func, Func func>
    inline RecordBuilder& static_method(String name)
    {
        auto& method_data = _data->static_methods.emplace().ref();
        method_data.name  = std::move(name);
        method_data.fill_signature(func);
        return *this;
    }
    template <auto Field>
    inline RecordBuilder& static_field(String name)
    {
        auto& field_data = _data->static_fields.emplace().ref();
        field_data.name  = std::move(name);
        field_data.fill_signature(Field);
        return *this;
    }

private:
    RecordData* _data;
};

}; // namespace skr::rttr