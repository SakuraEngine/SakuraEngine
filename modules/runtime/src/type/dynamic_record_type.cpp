#include "type/type.hpp"
#include "containers/vector.hpp"

namespace skr {
namespace type {

struct DynamicRecordType : public RecordType
{
    skr::string _name;
    skr::vector<skr_field_t> _fields;
    skr::vector<skr_method_t> _methods;
};

} // namespace type
} // namespace skr

skr_dynamic_record_type_id skr_create_record_type(const skr_guid_t* type_id, uint64_t size, uint64_t align, const skr_guid_t* parent)
{
    auto record = SkrNew<skr::type::DynamicRecordType>();
    record->size = size;
    record->guid = *type_id;
    record->align = align;
    record->type = SKR_TYPE_CATEGORY_OBJ;
    if (parent)
    {
        auto parent_type = skr_get_type(parent);
        if (parent_type->type == SKR_TYPE_CATEGORY_OBJ)
        {
            record->base = (skr::type::RecordType*)parent_type;
        }
    }
    skr::type::GetTypeRegistry()->register_type(*type_id, record);
    return record;
}

void skr_record_type_set_name(skr_dynamic_record_type_id type, const char* name)
{
    type->_name = name;
    auto pNameView = (skr::string_view*)&type->name;
    *pNameView = type->_name.c_str();
}

void skr_record_type_set_hasher(skr_dynamic_record_type_id type, size_t (*hasher)(const void* self, size_t base))
{
    type->nativeMethods.Hash = hasher;
}
