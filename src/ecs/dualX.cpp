#include "platform/configure.h"
#include "ecs/dualX.h"
#include "utils/make_zeroed.hpp"
#include "stack.hpp"
#include "callback.hpp"
#include "utils/hash.h"
#include <iostream>

namespace dual
{
    extern thread_local fixed_stack_t localStack;
    static dual_type_index_t hashed_set_type;
}

struct dualX_hash_set_query_exist_t {
    dual_type_index_t exist;
    size_t hash;
    dual_storage_t* storage;
};
struct dualX_hashed_set_t {
    // {f9324497-32de-41d4-b08e-423064eb7df0}
    static constexpr skr_guid_t _guid = {0xf9324497, 0x32de, 0x41d4, {0xb0, 0x8e, 0x42, 0x30, 0x64, 0xeb, 0x7d, 0xf0}};
    size_t hash;
};
dual_entity_t dualX_hashset_insert(
    dual_storage_t* storage, const dual_type_set_t* key_set,
    dual_entity_type_t* alloc_type, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    dual_entity_t temp = NULL_ENTITY;
    dual_chunk_view_t temp_view;
    // filter types
    auto filter_types = localStack.allocate<dual_type_index_t>(key_set->length + 1);
    {
        dual_type_index_t cursor = hashed_set_type;
        for(uint32_t i = 0, idx = 0; idx < key_set->length + 1; idx++)
        {
            if(key_set->data[i] < cursor)
                filter_types[idx] = key_set->data[i++];
            else
            {
                filter_types[idx] = cursor;
                cursor = key_set->data[i];
            }
        }
    }
    // allocate new
    auto types = localStack.allocate<dual_type_index_t>(alloc_type->type.length + 1);
    {
        dual_type_index_t cursor = hashed_set_type;
        for(uint32_t i = 0, idx = 0; idx < alloc_type->type.length + 1; idx++)
        {
            if(alloc_type->type.data[i] < cursor)
                types[idx] = alloc_type->type.data[i++];
            else
            {
                types[idx] = cursor;
                cursor = alloc_type->type.data[i];
            }
        }
    }
    std::cout << types[0] << types[1];
    auto actual_alloc_type = make_zeroed<dual_entity_type_t>();
    actual_alloc_type.type.data = types;
    actual_alloc_type.type.length = alloc_type->type.length + 1;
    actual_alloc_type.meta = alloc_type->meta;
    size_t this_hash = ~0;
    auto hash_callback = [&](dual_chunk_view_t* inView) {
        // do user initialize callback
        callback(u, inView);
        // do hash
        auto hashed = (dualX_hashed_set_t*)dualV_get_owned_rw(inView, hashed_set_type);
        hashed->hash = (size_t)storage;
        for(uint32_t i = 0; i < key_set->length; i++)
        {
            const auto dt = (dualX_hashed_set_t*)dualV_get_owned_rw(inView, key_set->data[i]);
            const auto dsize = dualT_get_desc(key_set->data[i])->size;
            hashed->hash = skr_hash(dt, dsize, hashed->hash);
        }
        this_hash = hashed->hash;
        temp = dualV_get_entities(inView)[0];
        temp_view = *inView;
    };
    dualS_allocate_type(storage, &actual_alloc_type, 1, DUAL_LAMBDA(hash_callback));
    // query exist
    auto filter = make_zeroed<dual_filter_t>();
    filter.all.data = filter_types;
    std::cout << filter_types[0] << filter_types[1] << " " << this_hash <<  std::endl;
    filter.all.length = key_set->length + 1;
    auto meta = make_zeroed<dual_meta_filter_t>();
    meta.all_meta = alloc_type->meta;
    dual_entity_t result = NULL_ENTITY;
    auto unique_callback = [&](dual_chunk_view_t* inView) {
        if (result != NULL_ENTITY) return;
        auto hashes = (dualX_hashed_set_t*)dualV_get_owned_rw(inView, hashed_set_type);
        auto ents = dualV_get_entities(inView);
        for(uint32_t i = 0; i < inView->count; i++)
        {
            // alreay exists and is not temp
            if(hashes[i].hash == this_hash && ents[i] != temp)
            {
                result = ents[i];
                std::cout << " == " << ents[i] << std::endl;
                return;
            }
        }
    };
    dualS_query(storage, &filter, &meta, DUAL_LAMBDA(unique_callback));
    if(result != NULL_ENTITY)
    {
        dualS_destroy(storage, &temp_view);
    }
    else 
        result = temp;
    return result;
}

extern "C" void dualX_register_types()
{
    {
        auto desc = make_zeroed<dual_type_description_t>();
        desc.name = "dualX_hashed_set";
        desc.size = sizeof(dualX_hashed_set_t);
        desc.guid = dualX_hashed_set_t::_guid;
        desc.alignment = alignof(dualX_hashed_set_t);
        dual::hashed_set_type = dualT_register_type(&desc);
    }
}