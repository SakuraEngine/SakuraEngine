#pragma once
#include <cstdint>
#include "SkrGuid/guid.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/string.hpp"
#include "SkrRTTR/enum_value.hpp"
#include "SkrRTTR/enum_traits.hpp"
#include "SkrRTTR/export/export_data.hpp"
#include "SkrRTTR/export/rttr_backend.hpp"
#include "export/enum_builder.hpp"

// !!!! RTTR 不考虑动态类型建立(从脚本建立), 一切类型都是 CPP 静态注册的 loader !!!!
namespace skr::rttr
{
enum class ETypeCategory
{
    Invalid = 0, // uninitialized bad type

    Primitive,
    Record,
    Enum,
};

struct SKR_CORE_API Type final {
    // ctor & dtor
    Type();
    ~Type();

    // init
    void init(ETypeCategory type_category);

    // basic getter
    ETypeCategory      type_category() const;
    const skr::String& name() const;
    GUID               type_id() const;
    size_t             size() const;
    size_t             alignment() const;

    // data getter
    const PrimitiveData& primitive_data() const;
    PrimitiveData&       primitive_data();
    const RecordData&    record_data() const;
    RecordData&          record_data();
    const EnumData&      enum_data() const;
    EnumData&            enum_data();

    // TODO. check phase
    // void validate_export_data() const;

    // build optimize data
    //  1. fast base find table
    //  2. fast method/field table
    void build_optimize_data();

    // caster
    void* cast_to(GUID type_id, void* p) const;

    // TODO. signature extractor
    // TODO. template extractor (invoker 就不用了，分两步挺好的，妈的智障 CPP)
    // TODO. 直接 invoke 的情形比较通用, 要不先实现了

    // ? finder 一个 MethodInvoker 的情况很少用到，用到了再说

private:
    ETypeCategory _type_category = ETypeCategory::Invalid;
    union
    {
        PrimitiveData _primitive_data;
        RecordData    _record_data;
        EnumData      _enum_data;
    };
};

template <typename T>
void enum_type_loader_from_traits(Type* type)
{
    // init type
    type->init(ETypeCategory::Enum);
    auto& enum_data = type->enum_data();

    // get items and reserve
    auto items = EnumTraits<T>::items();
    enum_data.items.reserve(items.size());

    // build items
    EnumBuilder<T, RTTRBackend> builder(&enum_data);
    builder.basic_info();

    // build items
    for (const auto& item : items)
    {
        builder.item(item.name, item.value);
    }
}

} // namespace skr::rttr