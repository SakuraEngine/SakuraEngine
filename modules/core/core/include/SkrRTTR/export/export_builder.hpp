#pragma once
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/export_helper.hpp"
#include "SkrRTTR/export/extern_methods.hpp"
#include <concepts>

// functions and methods
namespace skr
{
struct RTTRParamBuilder {
    inline RTTRParamBuilder(RTTRParamData* data)
        : _data(data)
    {
    }

    // basic
    inline RTTRParamBuilder& name(String name)
    {
        _data->name = std::move(name);
        return *this;
    }
    inline RTTRParamBuilder& default_func(RTTRParamData::MakeDefaultFunc func)
    {
        _data->make_default = func;
        return *this;
    }

    // flag & attributes
    inline RTTRParamBuilder& flag(ERTTRParamFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRParamBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRParamData* _data;
};

struct RTTRFunctionBuilder {
    inline RTTRFunctionBuilder(RTTRFunctionData* data)
        : _data(data)
    {
    }

    // params
    inline RTTRParamBuilder param_at(uint64_t index)
    {
        return RTTRParamBuilder(_data->param_data[index]);
    }

    // flag & attributes
    inline RTTRFunctionBuilder& flag(ERTTRFunctionFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRFunctionBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRFunctionData* _data;
};

struct RTTRMethodBuilder {
    inline RTTRMethodBuilder(RTTRMethodData* data)
        : _data(data)
    {
    }

    // params
    inline RTTRParamBuilder param_at(uint64_t index)
    {
        return RTTRParamBuilder(_data->param_data[index]);
    }

    // flag & attributes
    inline RTTRMethodBuilder& flag(ERTTRMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRMethodBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRMethodData* _data;
};

struct RTTRStaticMethodBuilder {
    inline RTTRStaticMethodBuilder(RTTRStaticMethodData* data)
        : _data(data)
    {
    }

    // params
    inline RTTRParamBuilder param_at(uint64_t index)
    {
        return RTTRParamBuilder(_data->param_data[index]);
    }

    // flag & attributes
    inline RTTRStaticMethodBuilder& flag(ERTTRStaticMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRStaticMethodBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRStaticMethodData* _data;
};

struct RTTRExternMethodBuilder {
    inline RTTRExternMethodBuilder(RTTRExternMethodData* data)
        : _data(data)
    {
    }

    // params
    inline RTTRParamBuilder param_at(uint64_t index)
    {
        return RTTRParamBuilder(_data->param_data[index]);
    }

    // flag & attributes
    inline RTTRExternMethodBuilder& flag(ERTTRExternMethodFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRExternMethodBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRExternMethodData* _data;
};

struct RTTRCtorBuilder {
    inline RTTRCtorBuilder(RTTRCtorData* data)
        : _data(data)
    {
    }

    // params
    inline RTTRParamBuilder param_at(uint64_t index)
    {
        return RTTRParamBuilder(_data->param_data[index]);
    }

    // flag & attributes
    inline RTTRCtorBuilder& flag(ERTTRCtorFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRCtorBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRCtorData* _data;
};
} // namespace skr

// fields
namespace skr
{
struct RTTRFieldBuilder {
    inline RTTRFieldBuilder(RTTRFieldData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline RTTRFieldBuilder& flag(ERTTRFieldFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRFieldBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRFieldData* _data;
};

struct RTTRStaticFieldBuilder {
    inline RTTRStaticFieldBuilder(RTTRStaticFieldData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline RTTRStaticFieldBuilder& flag(ERTTRStaticFieldFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRStaticFieldBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRStaticFieldData* _data;
};
} // namespace skr

// record
namespace skr
{
template <typename T>
struct RTTRRecordBuilder {
    inline RTTRRecordBuilder(RTTRRecordData* data)
        : _data(data)
    {
    }

    // basic info
    inline RTTRRecordBuilder& basic_info()
    {
        // split namespace
        String             name = RTTRTraits<T>::get_name();
        Vector<StringView> splitted;
        auto               count = name.split(splitted, u8"::");

        // last part is name
        _data->name = splitted.at(splitted.size() - 1);

        // fill namespace
        if (count > 1)
        {
            _data->name_space.reserve(count - 1);
            for (auto i = 0; i < count - 1; ++i)
            {
                _data->name_space.push_back(splitted.at(i));
            }
        }

        // fill type id
        _data->type_id = RTTRTraits<T>::get_guid();

        // fill size & alignment
        _data->size      = sizeof(T);
        _data->alignment = alignof(T);

        // fill memory traits
        _data->memory_traits_data.Fill<T>();

        // fill default ctor
        if constexpr (std::is_default_constructible_v<T>)
        {
            ctor<>();
        }

        // fill dtor
        if constexpr (std::is_destructible_v<T>)
        {
            _data->dtor_data.native_invoke = RTTRExportHelper::export_dtor<T>();
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
            extern_method<+[](T& lhs, const T& rhs) -> void {
                lhs.operator=(rhs);
            }>(CPPExternMethods::Assign);
        }

        // fill move assign operator
        if constexpr (std::is_move_assignable_v<T>)
        {
            extern_method<+[](T& lhs, T&& rhs) -> void {
                lhs.operator=(std::move(rhs));
            }>(CPPExternMethods::Assign);
        }

        // fill compare operator
        if constexpr (skr::concepts::HasEq<const T&, const T&>)
        {
            extern_method<+[](const T& lhs, const T& rhs) -> bool {
                return lhs == rhs;
            }>(CPPExternMethods::Eq);
        }

        // fill hash
        if constexpr (skr::concepts::HasHasher<T>)
        {
            extern_method<+[](const T& object) -> uint64_t {
                return skr::Hash<T>{}(object);
            }>(SkrCoreExternMethods::Hash);
        }

        // fill swap
        if constexpr (skr::concepts::HasSwap<T>)
        {
            extern_method<+[](T& lhs, T& rhs) -> void {
                skr::Swap<T>::call(lhs, rhs);
            }>(SkrCoreExternMethods::Swap);
        }

        // fill bin serde
        if constexpr (skr::concepts::HasBinRead<T>)
        {
            extern_method<+[](void* object, void* reader) -> bool {
                return skr::bin_read<T>((SBinaryReader*)reader, *(T*)object);
            }>(SkrCoreExternMethods::ReadBin);
        }
        if constexpr (skr::concepts::HasBinWrite<T>)
        {
            extern_method<+[](void* object, void* writer) -> bool {
                return skr::bin_write<T>((SBinaryWriter*)writer, *(T*)object);
            }>(SkrCoreExternMethods::WriteBin);
        }

        // fill json serde
        if constexpr (skr::concepts::HasJsonRead<T>)
        {
            extern_method<+[](void* object, void* reader) -> bool {
                return skr::json_read<T>((skr::archive::JsonReader*)reader, *(T*)object);
            }>(SkrCoreExternMethods::ReadJson);
        }
        if constexpr (skr::concepts::HasJsonWrite<T>)
        {
            extern_method<+[](void* object, void* writer) -> bool {
                return skr::json_write<T>((skr::archive::JsonWriter*)writer, *(T*)object);
            }>(SkrCoreExternMethods::WriteJson);
        }

        return *this;
    }

    // ctor
    template <typename... Args>
    inline RTTRCtorBuilder ctor()
    {
        // find first
        // TODO. use extern mode flag will be better?
        {
            TypeSignatureTyped<void(Args...)> signature;

            auto result = _data->ctor_data.find_if([&](const RTTRCtorData* ctor_data) {
                return ctor_data->signature_equal(signature.view(), ETypeSignatureCompareFlag::Strict);
            });

            if (result)
            {
                return { result.ref() };
            }
        }

        // new ctor data
        auto ctor_data = SkrNew<RTTRCtorData>();
        ctor_data->fill_signature<Args...>();
        ctor_data->native_invoke        = RTTRExportHelper::export_ctor<T, Args...>();
        ctor_data->dynamic_stack_invoke = RTTRExportHelper::export_ctor_dynamic_stack<T, Args...>();
        _data->ctor_data.add(ctor_data);
        return { ctor_data };
    }

    // bases
    template <typename... Bases>
    inline RTTRRecordBuilder& bases()
    {
        _data->bases_data.append({ RTTRBaseData::New<T, Bases>()... });
        return *this;
    }

    // method & static method & extern method
    template <auto func>
    inline RTTRMethodBuilder method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto method_data          = SkrNew<RTTRMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke        = RTTRExportHelper::export_method<func>();
        method_data->dynamic_stack_invoke = RTTRExportHelper::export_method_dynamic_stack<func>();
        _data->methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline RTTRMethodBuilder method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        return method<func>(std::move(name), access_level);
    }
    template <auto func>
    inline RTTRStaticMethodBuilder static_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto method_data          = SkrNew<RTTRStaticMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke        = RTTRExportHelper::export_static_method<func>();
        method_data->dynamic_stack_invoke = RTTRExportHelper::export_static_method_dynamic_stack<func>();
        _data->static_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline RTTRStaticMethodBuilder static_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        return static_method<func>(std::move(name), access_level);
    }
    template <auto func>
    inline RTTRExternMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto method_data          = SkrNew<RTTRExternMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke        = RTTRExportHelper::export_extern_method<func>();
        method_data->dynamic_stack_invoke = RTTRExportHelper::export_extern_method_dynamic_stack<func>();
        _data->extern_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline RTTRStaticMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        return extern_method<func>(std::move(name), access_level);
    }

    // field & static field
    template <auto _field>
    inline RTTRFieldBuilder field(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto field_data          = SkrNew<RTTRFieldData>();
        field_data->name         = std::move(name);
        field_data->access_level = access_level;
        field_data->fill_signature<_field>(_field);
        _data->fields.add(field_data);
        return { field_data };
    }
    template <auto _field>
    inline RTTRStaticFieldBuilder static_field(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto field_data          = SkrNew<RTTRStaticFieldData>();
        field_data->name         = std::move(name);
        field_data->access_level = access_level;
        field_data->address      = reinterpret_cast<void*>(_field);
        field_data->fill_signature(_field);
        _data->static_fields.add(field_data);
        return { field_data };
    }

    // flags & attributes
    inline RTTRRecordBuilder& flag(ERTTRRecordFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTRRecordBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTRRecordData* _data;
};
} // namespace skr

// enum
namespace skr
{
struct RTTREnumItemBuilder {
    inline RTTREnumItemBuilder(RTTREnumItemData* data)
        : _data(data)
    {
    }

    // flag & attributes
    inline RTTREnumItemBuilder& flag(ERTTREnumItemFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTREnumItemBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

private:
    RTTREnumItemData* _data;
};

template <typename T>
struct RTTREnumBuilder {
    inline RTTREnumBuilder(RTTREnumData* data)
        : _data(data)
    {
    }

    // basic info
    inline RTTREnumBuilder& basic_info()
    {
        // split namespace
        String             name = RTTRTraits<T>::get_name();
        Vector<StringView> splitted;
        auto               count = name.split(splitted, u8"::");

        // last part is name
        _data->name = splitted.at(splitted.size() - 1);

        // fill namespace
        if (count > 1)
        {
            _data->name_space.reserve(count - 1);
            for (auto i = 0; i < count - 1; ++i)
            {
                _data->name_space.push_back(splitted.at(i));
            }
        }

        // fill type id
        _data->type_id = RTTRTraits<T>::get_guid();

        // fill size & alignment
        _data->size      = sizeof(T);
        _data->alignment = alignof(T);

        // fill memory traits
        _data->memory_traits_data.Fill<T>();

        // fill underlying type id
        _data->underlying_type_id = RTTRTraits<std::underlying_type_t<T>>::get_guid();
        return *this;
    }

    // items
    inline RTTREnumItemBuilder item(String name, T value)
    {
        auto* item_data  = SkrNew<RTTREnumItemData>();
        item_data->name  = std::move(name);
        item_data->value = static_cast<std::underlying_type_t<T>>(value);
        _data->items.add(item_data);
        return { item_data };
    }

    // flag & attributes
    inline RTTREnumBuilder& flag(ERTTREnumFlag flag)
    {
        _data->flag = flag_set(_data->flag, flag);
        return *this;
    }
    inline RTTREnumBuilder& attribute(Any attr)
    {
        _data->attrs.add(std::move(attr));
        return *this;
    }

    // extern methods
    template <auto func>
    inline RTTRExternMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto method_data          = SkrNew<RTTRExternMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke        = RTTRExportHelper::export_extern_method<func>();
        method_data->dynamic_stack_invoke = RTTRExportHelper::export_extern_method_dynamic_stack<func>();
        _data->extern_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline RTTRStaticMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        return extern_method<func>(std::move(name), access_level);
    }

private:
    RTTREnumData* _data;
};
} // namespace skr

// primitive builder
namespace skr
{
template <typename T>
struct RTTRPrimitiveBuilder {
    inline RTTRPrimitiveBuilder(RTTRPrimitiveData* data)
        : _data(data)
    {
    }

    // basic info
    inline RTTRPrimitiveBuilder& basic_info()
    {
        // fill basic data
        _data->name      = RTTRTraits<T>::get_name();
        _data->type_id   = RTTRTraits<T>::get_guid();
        _data->size      = sizeof(T);
        _data->alignment = alignof(T);

        // fill memory traits
        _data->memory_traits_data.Fill<T>();

        // fill hash
        // if constexpr (skr::concepts::HasHasher<T>)
        {
            extern_method<+[](const T& object) -> uint64_t {
                return skr::Hash<T>{}(object);
            }>(SkrCoreExternMethods::Hash);
        }

        // fill swap
        // if constexpr (skr::concepts::HasSwap<T>)
        {
            extern_method<+[](T& lhs, T& rhs) -> void {
                skr::Swap<T>::call(lhs, rhs);
            }>(SkrCoreExternMethods::Swap);
        }

        // fill bin serde
        // if constexpr (skr::concepts::HasBinRead<T>)
        {
            extern_method<+[](void* object, void* reader) -> bool {
                return skr::bin_read<T>((SBinaryReader*)reader, *(T*)object);
            }>(SkrCoreExternMethods::ReadBin);
        }
        // if constexpr (skr::concepts::HasBinWrite<T>)
        {
            extern_method<+[](void* object, void* writer) -> bool {
                return skr::bin_write<T>((SBinaryWriter*)writer, *(T*)object);
            }>(SkrCoreExternMethods::WriteBin);
        }

        // fill json serde
        // if constexpr (skr::concepts::HasJsonRead<T>)
        {
            extern_method<+[](void* object, void* reader) -> bool {
                return skr::json_read<T>((skr::archive::JsonReader*)reader, *(T*)object);
            }>(SkrCoreExternMethods::ReadJson);
        }
        // if constexpr (skr::concepts::HasJsonWrite<T>)
        {
            extern_method<+[](void* object, void* writer) -> bool {
                return skr::json_write<T>((skr::archive::JsonWriter*)writer, *(T*)object);
            }>(SkrCoreExternMethods::WriteJson);
        }

        return *this;
    }

    template <auto func>
    inline RTTRExternMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        auto method_data          = SkrNew<RTTRExternMethodData>();
        method_data->name         = std::move(name);
        method_data->access_level = access_level;
        method_data->fill_signature(func);
        method_data->native_invoke        = RTTRExportHelper::export_extern_method<func>();
        method_data->dynamic_stack_invoke = RTTRExportHelper::export_extern_method_dynamic_stack<func>();
        _data->extern_methods.add(method_data);
        return { method_data };
    }
    template <typename Func, Func func>
    inline RTTRStaticMethodBuilder extern_method(String name, ERTTRAccessLevel access_level = ERTTRAccessLevel::Public)
    {
        return extern_method<func>(std::move(name), access_level);
    }

private:
    RTTRPrimitiveData* _data;
};
} // namespace skr
