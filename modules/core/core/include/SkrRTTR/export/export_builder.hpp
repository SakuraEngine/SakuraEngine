#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/export_helper.hpp"
#include "SkrRTTR/export/extern_methods.hpp"
#include <concepts>

// functions and methods
namespace skr::rttr
{
struct ParamBuilder {
    inline ParamBuilder(ParamData* data)
        : _data(data)
    {
    }

    // basic
    inline ParamBuilder& name(String name)
    {
        _data->name = std::move(name);
        return *this;
    }
    inline ParamBuilder& default_func(ParamData::MakeDefaultFunc func)
    {
        _data->make_default = func;
        return *this;
    }

    // flag & attributes
    inline ParamBuilder& flag(EParamFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline ParamBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    ParamData* _data;
};

struct FunctionBuilder {
    inline FunctionBuilder(FunctionData* data)
        : _data(data)
    {
    }

    // params
    inline ParamBuilder param_at(uint64_t index)
    {
        return ParamBuilder(&_data->param_data[index]);
    }

    // flag & attributes
    inline FunctionBuilder& flag(EFunctionFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline FunctionBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    FunctionData* _data;
};

struct MethodBuilder {
    inline MethodBuilder(MethodData* data)
        : _data(data)
    {
    }

    // params
    inline ParamBuilder param_at(uint64_t index)
    {
        return ParamBuilder(&_data->param_data[index]);
    }

    // flag & attributes
    inline MethodBuilder& flag(EMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline MethodBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    MethodData* _data;
};

struct StaticMethodBuilder {
    inline StaticMethodBuilder(StaticMethodData* data)
        : _data(data)
    {
    }

    // params
    inline ParamBuilder param_at(uint64_t index)
    {
        return ParamBuilder(&_data->param_data[index]);
    }

    // flag & attributes
    inline StaticMethodBuilder& flag(EStaticMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline StaticMethodBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    StaticMethodData* _data;
};

struct ExternMethodBuilder {
    inline ExternMethodBuilder(ExternMethodData* data)
        : _data(data)
    {
    }

    // params
    inline ParamBuilder param_at(uint64_t index)
    {
        return ParamBuilder(&_data->param_data[index]);
    }

    // flag & attributes
    inline ExternMethodBuilder& flag(EExternMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline ExternMethodBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    ExternMethodData* _data;
};

struct CtorBuilder {
    inline CtorBuilder(CtorData* data)
        : _data(data)
    {
    }

    // params
    inline ParamBuilder param_at(uint64_t index)
    {
        return ParamBuilder(&_data->param_data[index]);
    }

    // flag & attributes
    inline CtorBuilder& flag(ECtorFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline CtorBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    CtorData* _data;
};
} // namespace skr::rttr

// fields
namespace skr::rttr
{
struct FieldBuilder {
    inline FieldBuilder(FieldData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline FieldBuilder& flag(EFieldFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline FieldBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    FieldData* _data;
};

struct StaticFieldBuilder {
    inline StaticFieldBuilder(StaticFieldData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline StaticFieldBuilder& flag(EStaticFieldFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline StaticFieldBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    StaticFieldData* _data;
};
} // namespace skr::rttr

// record
namespace skr::rttr
{
template <typename T>
struct RecordBuilder {
    inline RecordBuilder(RecordData* data)
        : _data(data)
    {
    }

    // basic info
    // TODO. 在外部提供并抹除 T 信息
    inline RecordBuilder& basic_info()
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

        // TODO. 基础函数的导出也应当手动完成

        // fill default ctor
        if constexpr (std::is_default_constructible_v<T>)
        {
            ctor<>();
        }

        // fill copy ctor
        if constexpr (std::is_copy_constructible_v<T>)
        {
            ctor<const T&>();
        }

        // fill move ctor
        if constexpr (std::is_move_constructible_v<T>)
        {
            ctor<T&&>();
        }

        // fill assign operator
        if constexpr (std::is_copy_assignable_v<T>)
        {
            extern_method<+[](T& lhs, const T& rhs) { lhs.operator=(rhs); }>(CPPExternMethods::Assign);
        }

        // fill move assign operator
        if constexpr (std::is_move_assignable_v<T>)
        {
            extern_method<+[](T& lhs, T&& rhs) { lhs.operator=(std::move(rhs)); }>(CPPExternMethods::Assign);
        }

        // fill dtor
        if constexpr (std::is_destructible_v<T>)
        {
            _data->dtor_data.native_invoke = ExportHelper::export_dtor<T>();
        }

        return *this;
    }

    // ctor
    template <typename... Args>
    inline CtorBuilder ctor()
    {
        auto ctor_data = SkrNew<CtorData>();
        ctor_data->fill_signature<Args...>();
        ctor_data->native_invoke      = ExportHelper::export_ctor<T, Args...>();
        ctor_data->stack_proxy_invoke = ExportHelper::export_ctor_stack_proxy<T, Args...>();
        _data->ctor_data.add(ctor_data);
        return { ctor_data };
    }

    // bases
    template <typename... Bases>
    inline RecordBuilder& bases()
    {
        _data->bases_data.append({ BaseData::New<T, Bases>()... });
        return *this;
    }

    // method & static method & extern method
    template <auto func>
    inline MethodBuilder method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        auto method_data          = SkrNew<MethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke      = ExportHelper::export_method<func>();
        method_data->stack_proxy_invoke = ExportHelper::export_method_stack_proxy<func>();
        _data->methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline MethodBuilder method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        return method<func>(std::move(name), access_level);
    }
    template <auto func>
    inline StaticMethodBuilder static_method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        auto method_data          = SkrNew<StaticMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke      = ExportHelper::export_static_method<func>();
        method_data->stack_proxy_invoke = ExportHelper::export_static_method_stack_proxy<func>();
        _data->static_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline StaticMethodBuilder static_method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        return static_method<func>(std::move(name), access_level);
    }
    template <auto func>
    inline ExternMethodBuilder extern_method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        auto method_data          = SkrNew<ExternMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke      = ExportHelper::export_extern_method<func>();
        method_data->stack_proxy_invoke = ExportHelper::export_extern_method_stack_proxy<func>();
        _data->extern_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline StaticMethodBuilder extern_method(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        return extern_method<func>(std::move(name), access_level);
    }

    // field & static field
    template <auto _field>
    inline FieldBuilder field(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        auto field_data          = SkrNew<FieldData>();
        field_data->name         = std::move(name);
        field_data->access_level = access_level;
        field_data->fill_signature<_field>(_field);
        _data->fields.add(field_data);
        return { field_data };
    }
    template <auto _field>
    inline StaticFieldBuilder static_field(String name, EAccessLevel access_level = EAccessLevel::Public)
    {
        auto field_data          = SkrNew<StaticFieldData>();
        field_data->name         = std::move(name);
        field_data->access_level = access_level;
        field_data->address      = reinterpret_cast<void*>(_field);
        field_data->fill_signature(_field);
        _data->static_fields.add(field_data);
        return { field_data };
    }

    // flags & attributes
    inline RecordBuilder& flag(ERecordFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline RecordBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    RecordData* _data;
};
} // namespace skr::rttr

// enum
namespace skr::rttr
{
struct EnumItemBuilder {
    inline EnumItemBuilder(EnumItemData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline EnumItemBuilder& flag(EEnumItemFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline EnumItemBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    EnumItemData* _data;
};

template <typename T>
struct EnumBuilder {
    inline EnumBuilder(EnumData* data)
        : _data(data)
    {
    }

    // basic info
    inline EnumBuilder& basic_info()
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

        // fill underlying type id
        _data->underlying_type_id = RTTRTraits<std::underlying_type_t<T>>::get_guid();
        return *this;
    }

    // items
    inline EnumItemBuilder item(String name, T value)
    {
        auto& item_data = _data->items.emplace().ref();
        item_data.name  = std::move(name);
        item_data.value = static_cast<std::underlying_type_t<T>>(value);
        return { &item_data };
    }

    // flag & attributes
    inline EnumBuilder& flag(EEnumFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    template <std::derived_from<IAttribute> Attr>
    inline EnumBuilder& attribute(Attr attr)
    {
        IAttribute* copied_attr = new Attr(std::move(attr));
        _data->attributes.add(type_id_of<Attr>(), copied_attr);
        return *this;
    }

private:
    EnumData* _data;
};
} // namespace skr::rttr