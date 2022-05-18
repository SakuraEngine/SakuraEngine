#include <string_view>

#include "ecs/dual.h"
#include "pool.hpp"
#include "entity.hpp"
#include <utility>
#include "type.hpp"
#include "ecs/constants.hpp"
#include "type_registry.hpp"
#include "guid.hpp"
#include "sole.hpp"
#if __SSE2__
    #include <emmintrin.h>
    #define DUAL_MASK_ALIGN alignof(__m128i)
#else
    #define DUAL_MASK_ALIGN alignof(uint32_t)
#endif

namespace dual
{
type_registry_t::type_registry_t(pool_t& pool)
    : nameArena(pool)
{
    {
        type_description_t desc;
        desc.guid = skr::guid::make_guid("{B68B1CAB-98FF-4298-A22E-68B404034B1B}");
        desc.name = "disable";
        desc.size = 0;
        desc.elementSize = 0;
        desc.alignment = 0;
        desc.entityFieldsCount = 0;
        desc.flags = 0;
        descriptions.push_back(desc);
    }
    {
        type_description_t desc;
        desc.guid = skr::guid::make_guid("{C0471B12-5462-48BB-B8C4-9983036ECC6C}");
        desc.name = "dead";
        desc.size = 0;
        desc.elementSize = 0;
        desc.alignment = 0;
        desc.entityFieldsCount = 0;
        desc.flags = 0;
        descriptions.push_back(desc);
    }
    {
        type_description_t desc;
        desc.guid = skr::guid::make_guid("{54BD68D5-FD66-4DBE-85CF-70F535C27389}");
        desc.name = "link";
        desc.size = sizeof(dual_entity_t) * kLinkComponentSize;
        desc.elementSize = sizeof(dual_entity_t);
        desc.alignment = alignof(dual_entity_t);
        desc.entityFieldsCount = 1;
        entityFields.push_back(0);
        desc.entityFields = 0;
        desc.flags = 0;
        descriptions.push_back(desc);
    }
    {
        assert(descriptions.size() == kMaskComponent);
        type_description_t desc;
        desc.guid = skr::guid::make_guid("{B68B1CAB-98FF-4298-A22E-68B404034B1B}");
        desc.name = "mask";
        desc.size = sizeof(dual_mask_component_t);
        desc.elementSize = 0;
        desc.alignment = DUAL_MASK_ALIGN;
        desc.entityFieldsCount = 0;
        desc.flags = 0;
        descriptions.push_back(desc);
    }
    {
        assert(descriptions.size() == kGuidComponent);
        type_description_t desc;
        desc.guid = skr::guid::make_guid("{565FBE87-6309-4DF7-9B3F-C61B67B38BB3}");
        desc.name = "guid";
        desc.size = 0;
        desc.elementSize = 0;
        desc.alignment = 0;
        desc.entityFieldsCount = 0;
        desc.flags = 0;
        descriptions.push_back(desc);
    }
}

type_index_t type_registry_t::register_type(const type_description_t& inDesc)
{
    if (auto index = get_type(inDesc.guid); index != kInvalidTypeIndex)
        return index;
    type_description_t desc = inDesc;
    if (!desc.name)
    {
        return kInvalidSIndex;
    }
    else
    {
        if (name2type.count(desc.name))
            return kInvalidTypeIndex;
        auto len = std::strlen(desc.name);
        auto name = (char*)nameArena.allocate(len, 1);
        std::memcpy(name, desc.name, len + 1);
        desc.name = name;
    }
    if (desc.entityFields != 0)
    {
        intptr_t* efs = (intptr_t*)desc.entityFields;
        desc.entityFields = entityFields.size();
        for (uint32_t i = 0; i < desc.entityFieldsCount; ++i)
            entityFields.push_back(efs[i]);
    }
    bool tag = false;
    bool managed = false;
    bool pin = false;
    bool buffer = false;
    if (desc.size == 0)
        tag = true;
    if (desc.callback.copy != nullptr ||
        desc.callback.move != nullptr ||
        desc.callback.destructor != nullptr)
        managed = true;
    if (desc.elementSize != 0)
        buffer = true;
    pin = (desc.flags & DTF_PIN) != 0;
    type_index_t index{ (TIndex)descriptions.size(), pin, buffer, managed, tag };
    descriptions.push_back(desc);
    guid2type.insert(eastl::make_pair(desc.guid, index));
    name2type.insert(eastl::make_pair(desc.name, index));
    return index;
}

type_index_t type_registry_t::get_type(const guid_t& guid)
{
    auto i = guid2type.find(guid);
    if (i != guid2type.end())
        return i->second;
    return kInvalidTypeIndex;
}

type_index_t type_registry_t::get_type(eastl::string_view name)
{
    auto i = name2type.find(name);
    if (i != name2type.end())
        return i->second;
    return kInvalidTypeIndex;
}

guid_t type_registry_t::make_guid()
{
    if (guid_func)
    {
        dual_guid_t guid;
        guid_func(&guid);
        return guid;
    }
    {
        guid_t guid;
        auto uuid = sole::uuid4();
        memcpy(&guid, &uuid, sizeof(guid_t));
        return guid;
    }
}
} // namespace dual

extern "C" {

void dual_make_guid(skr_guid_t* guid)
{
    *guid = dual::type_registry_t::get().make_guid();
}

dual_type_index_t dualT_register_type(dual_type_description_t* description)
{
    return dual::type_registry_t::get().register_type(*description);
}

dual_type_index_t dualT_get_type(const dual_guid_t* guid)
{
    return dual::type_registry_t::get().get_type(*guid);
}

dual_type_index_t dualT_get_type_by_name(const char* name)
{
    return dual::type_registry_t::get().get_type(name);
}

const dual_type_description_t* dualT_get_desc(dual_type_index_t idx)
{
    return &dual::type_registry_t::get().descriptions[dual::type_index_t(idx).index()];
}

void dualT_set_guid_func(guid_func_t func)
{
    dual::type_registry_t::get().guid_func = func;
}
}