#pragma once
#include "sugoi_types.h"
#if defined(__cplusplus)
    #include "SkrCore/log.h"
    #include "SkrTask/fib_task.hpp"
    #include "SkrRT/ecs/callback.hpp"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define SUGOI_COMPONENT_DISABLE 0x80000000
#define SUGOI_COMPONENT_DEAD 0x80000001
#define SUGOI_COMPONENT_LINK 0x20000002
#define SUGOI_COMPONENT_MASK 0x3
#define SUGOI_COMPONENT_GUID 0x4
#define SUGOI_COMPONENT_DIRTY 0x5

#define SUGOI_IS_TAG(c) ((c & 1 << 31) != 0)
#define SUGOI_IS_BUFFER(C) ((C & 1 << 29) != 0)

/**
 * @brief guid generation function
 *
 */
typedef void (*guid_func_t)(sugoi_guid_t* guid);
typedef struct sugoi_mapper_t {
    void (*map)(void* user, sugoi_entity_t* ent) SKR_IF_CPP(= nullptr);
    void* user SKR_IF_CPP(= nullptr);
} sugoi_mapper_t;

typedef struct SBinaryWriter SBinaryWriter;
typedef struct SBinaryReader SBinaryReader;
typedef struct sugoi_callback_v {
    void (*constructor)(sugoi_chunk_t* chunk, EIndex index, char* data) SKR_IF_CPP(= nullptr);
    void (*copy)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src) SKR_IF_CPP(= nullptr);
    void (*destructor)(sugoi_chunk_t* chunk, EIndex index, char* data) SKR_IF_CPP(= nullptr);
    void (*move)(sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, char* src) SKR_IF_CPP(= nullptr);
    void (*serialize)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer) SKR_IF_CPP(= nullptr);
    void (*deserialize)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* reader) SKR_IF_CPP(= nullptr);
    void (*serialize_text)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonWriter* writer) SKR_IF_CPP(= nullptr);
    void (*deserialize_text)(sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonReader* reader) SKR_IF_CPP(= nullptr);
    void (*map)(sugoi_chunk_t* chunk, EIndex index, char* data, sugoi_mapper_t* v) SKR_IF_CPP(= nullptr);
    int (*lua_push)(sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L) SKR_IF_CPP(= nullptr);
    void (*lua_check)(sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L, int idx) SKR_IF_CPP(= nullptr);
} sugoi_callback_v;

enum ESugoiTypeFlag SKR_IF_CPP( : uint32_t){
    SUGOI_TYPE_FLAG_PIN   = 0x1,
    SUGOI_TYPE_FLAG_CHUNK = 0x2,
};
typedef uint32_t SugoiTypeFlags;

enum ESugoiCallbackFlag SKR_IF_CPP( : uint32_t){
    SUGOI_CALLBACK_FLAG_CTOR = 0x1,
    SUGOI_CALLBACK_FLAG_DTOR = 0x2,
    SUGOI_CALLBACK_FLAG_COPY = 0x4,
    SUGOI_CALLBACK_FLAG_MOVE = 0x8,
    SUGOI_CALLBACK_FLAG_ALL  = SUGOI_CALLBACK_FLAG_CTOR | SUGOI_CALLBACK_FLAG_DTOR | SUGOI_CALLBACK_FLAG_COPY | SUGOI_CALLBACK_FLAG_MOVE,
};
typedef uint32_t SugoiCallbackFlags;

/**
 * @brief describe basic infomation of a component type
 *
 */
typedef struct sugoi_type_description_t {
    sugoi_guid_t   guid;
    const char8_t* name;
    const char8_t* guidStr;
    /**
     * a pinned component will not removed when destroying or copy when instantiating, and user should remove them manually
     * destroyed entity with pinned component will be marked by a dead component and will be erased when all pinned component is removed
     */
    SugoiTypeFlags flags;
    /**
     * the storage size in chunk of this component, generally it is sizeof(T)
     * when this is a array component, it could be sizeof(T) * I + sizeof(sugoi_array_comp_t) where I means the inline element count of the array
     * @see sugoi_array_comp_t
     */
    uint16_t size;
    /**
     * element size of this component, when this is a array component it would be equal to sizeof(T), otherwise it should be set to zero
     *
     */
    uint16_t elementSize;
    uint16_t alignment;
    // entity field is used to guarantee references between entities are keeping valid after operations like instantiate, merge world, deserialize etc.
    intptr_t entityFields;
    uint32_t entityFieldsCount;
    // resource field is used to track resource lifetime
    intptr_t resourceFields;
    uint32_t resourceFieldsCount;
    // lifetime callbacks of this component
    sugoi_callback_v callback;
} sugoi_type_description_t;

typedef struct sugoi_chunk_view_t {
    sugoi_chunk_t* chunk;
    EIndex         start;
    EIndex         count;
} sugoi_chunk_view_t;

/**
 * @brief set of component types
 * note. type set does not own the data, and all data should be in order
 */
typedef struct sugoi_type_set_t {
    const sugoi_type_index_t* data;
    SIndex                    length;
} sugoi_type_set_t;

/**
 * @brief set of meta types
 * meta type meaning entity as type
 * note. type set does not own the data, and all data should be in order
 */
typedef struct sugoi_entity_set_t {
    const sugoi_entity_t* data;
    SIndex                length;
} sugoi_entity_set_t;

typedef struct sugoi_entity_type_t {
    sugoi_type_set_t   type;
    sugoi_entity_set_t meta;
} sugoi_entity_type_t;

/**
 * @brief difference between two type
 *
 */
typedef struct sugoi_delta_type_t {
    sugoi_entity_type_t added;
    sugoi_entity_type_t removed;
} sugoi_delta_type_t;

/**
 * @brief entity filter
 *
 */
typedef struct sugoi_filter_t {
    // filter owned types
    sugoi_type_set_t all;
    sugoi_type_set_t none;
    // filter shared types
    sugoi_type_set_t all_shared;
    sugoi_type_set_t none_shared;
} sugoi_filter_t;

enum sugoi_operation_scope
{
    DOS_PAR,
    DOS_SEQ,
    DOS_UNSEQ,
};

/**
 * @brief describes the operation to a component
 *
 */
typedef struct sugoi_operation_t {
    int phase;    //-1 means any phase
    int readonly; // read or write
    int atomic;
    int randomAccess; // random access
} sugoi_operation_t;

/**
 * @brief describes the operation to components within one task
 *
 */
typedef struct sugoi_parameters_t {
    const sugoi_type_index_t* types;
    const sugoi_operation_t*  accesses;
    TIndex                    length;
} sugoi_parameters_t;

// runtime type (context sensitive) filter
typedef struct sugoi_meta_filter_t {
    sugoi_entity_set_t all_meta;
    sugoi_entity_set_t any_meta;
    sugoi_entity_set_t none_meta;
    sugoi_type_set_t   changed;
    uint64_t           timestamp;
} sugoi_meta_filter_t;

// header data of a array component
typedef struct sugoi_array_comp_t sugoi_array_comp_t;

typedef uint32_t sugoi_mask_comp_t;
typedef uint32_t sugoi_dirty_comp_t;

// APIS
/**
 * @brief initialize context, user should store the context and pass it to library by implementing sugoi_get_context
 * @see sugoi_get_context
 * @return sugoi_context_t*
 */
SKR_RUNTIME_API sugoi_context_t* sugoi_initialize();
/**
 * @brief get context, used by library, implemented by user
 * @see sugoi_initialize
 * @return sugoi_context_t*
 */
SKR_RUNTIME_API sugoi_context_t* sugoi_get_context();
/**
 * @brief destroy context, shutdown library
 *
 */
SKR_RUNTIME_API void sugoi_shutdown();

SKR_RUNTIME_API void sugoi_make_guid(skr_guid_t* guid);

SKR_RUNTIME_API void* sugoiA_begin(sugoi_array_comp_t* array);
SKR_RUNTIME_API void* sugoiA_end(sugoi_array_comp_t* array);

typedef void (*sugoi_view_callback_t)(void* u, sugoi_chunk_view_t* view);
typedef void (*sugoi_group_callback_t)(void* u, sugoi_group_t* view);
typedef void (*sugoi_entity_callback_t)(void* u, sugoi_entity_t e);
typedef void (*sugoi_cast_callback_t)(void* u, sugoi_chunk_view_t* new_view, sugoi_chunk_view_t* old_view);
typedef void (*sugoi_type_callback_t)(void* u, sugoi_type_index_t t);
typedef void (*sugoi_destroy_callback_t)(void* u, sugoi_chunk_view_t* view, sugoi_view_callback_t callback, void* u2);
typedef bool (*sugoi_custom_filter_callback_t)(void* u, sugoi_chunk_view_t* view);

/**
 * @brief register a new component
 *
 * @param description
 * @return component type
 * @see sugoi_type_description_t
 */
SKR_RUNTIME_API sugoi_type_index_t sugoiT_register_type(sugoi_type_description_t* description);
/**
 * @brief get component type from guid
 *
 * @param guid
 * @return component type
 */
SKR_RUNTIME_API sugoi_type_index_t sugoiT_get_type(const sugoi_guid_t* guid);
/**
 * @brief get component type from name
 *
 * @param name
 * @return component type
 */
SKR_RUNTIME_API sugoi_type_index_t sugoiT_get_type_by_name(const char8_t* name);
/**
 * @brief get description of component type
 *
 * @param idx
 * @return description
 */
SKR_RUNTIME_API const sugoi_type_description_t* sugoiT_get_desc(sugoi_type_index_t idx);
/**
 * @brief set guid generator function
 *
 * @param func
 */
SKR_RUNTIME_API void sugoiT_set_guid_func(guid_func_t func);
/**
 * @brief get all types registered to sugoi
 *
 * @param callback
 * @param u
 */
SKR_RUNTIME_API void sugoiT_get_types(sugoi_type_callback_t callback, void* u);
/**
 * @brief create a new storage
 *
 * @return new storage
 */
SKR_RUNTIME_API sugoi_storage_t* sugoiS_create();
/**
 * @brief release a storage
 * when storage is released, all entity and query within is deleted
 * @param storage
 */
SKR_RUNTIME_API void sugoiS_release(sugoi_storage_t* storage);
/**
 * @brief set userdata for storage
 *
 * @param storage
 * @param u
 * @return void
 */
SKR_RUNTIME_API void sugoiS_set_userdata(sugoi_storage_t* storage, void* u);
/**
 * @brief get userdata of storage
 *
 * @param storage
 * @return void*
 */
SKR_RUNTIME_API void* sugoiS_get_userdata(sugoi_storage_t* storage);
/**
 * @brief allocate entities
 * batch allocate numbers of entities with entity type
 * @param storage
 * @param type
 * @param count
 * @param callback optional callback after allocating chunk view
 */
SKR_RUNTIME_API void sugoiS_allocate_type(sugoi_storage_t* storage, const sugoi_entity_type_t* type, EIndex count, sugoi_view_callback_t callback, void* u);
/**
 * @brief allocate entities
 * batch allocate numbers of entities within a group
 * @param storage
 * @param group
 * @param count
 * @param callback optional callback after allocating chunk view
 */
SKR_RUNTIME_API void sugoiS_allocate_group(sugoi_storage_t* storage, sugoi_group_t* group, EIndex count, sugoi_view_callback_t callback, void* u);
/**
 * @brief instantiate entity
 * instantiate an entity n times
 * @param storage
 * @param prefab
 * @param count
 * @param callback optional callback after allocating chunk view
 */
SKR_RUNTIME_API void sugoiS_instantiate(sugoi_storage_t* storage, sugoi_entity_t prefab, EIndex count, sugoi_view_callback_t callback, void* u);
/**
 * @brief instantiate entity as specific type
 * instantiate an entity n times as specific type
 * @param storage
 * @param prefab
 * @param count
 * @param delta
 * @param callback optional callback after allocating chunk view
 */
SKR_RUNTIME_API void sugoiS_instantiate_delta(sugoi_storage_t* storage, sugoi_entity_t prefab, EIndex count, const sugoi_delta_type_t* delta, sugoi_view_callback_t callback, void* u);
/**
 * @brief instantiate entities
 * instantiate entities n times, internal reference will be kept
 * @param storage
 * @param prefab
 * @param count
 * @param callback optional callback after allocating chunk view
 */
SKR_RUNTIME_API void sugoiS_instantiate_entities(sugoi_storage_t* storage, sugoi_entity_t* ents, EIndex n, EIndex count, sugoi_view_callback_t callback, void* u);
/**
 * @brief destroy entities in chunk view
 * destory all entities in target chunk view
 * @param storage
 * @param view
 */
SKR_DEPRECATED("use other variants of sugoiS_destroy instead")
SKR_RUNTIME_API void sugoiS_destroy(sugoi_storage_t* storage, const sugoi_chunk_view_t* view);
/**
 * @brief destroy entities
 * destory given entities
 * @param storage
 * @param view
 */
SKR_RUNTIME_API void sugoiS_destroy_entities(sugoi_storage_t* storage, const sugoi_entity_t* ents, EIndex n);
/**
 * @brief destory entities
 * destory all entities matching given query
 * @param storage
 * @param ents
 * @param count
 */
SKR_RUNTIME_API void sugoiS_destroy_in_query(const sugoi_query_t* query);
/**
 * @brief destory entities
 * destory all entities matching given query and callback
 * @param storage
 * @param ents
 * @param count
 */
SKR_RUNTIME_API void sugoiS_destroy_in_query_if(const sugoi_query_t* query, sugoi_destroy_callback_t callback, void* u);
/**
 * @brief destory entities
 * destory all filtered entity
 * @param storage
 * @param ents
 * @param count
 */
SKR_RUNTIME_API void sugoiS_destroy_all(sugoi_storage_t* storage, const sugoi_meta_filter_t* meta);
/**
 * @brief change entities' type
 * change all entities' type in target chunk view by apply a delta type which will move entities to new group
 * there can be more than one chunk view after casting
 * @param storage
 * @param view
 * @param delta
 * @param callback optional callback before casting chunk view
 */
SKR_RUNTIME_API void sugoiS_cast_view_delta(sugoi_storage_t* storage, const sugoi_chunk_view_t* view, const sugoi_delta_type_t* delta, sugoi_cast_callback_t callback, void* u);
/**
 * @brief change entities' type
 * change all entities' type in target chunk view by move entities to new group
 * there can be more than one chunk view allocated
 * @param storage
 * @param view
 * @param group
 * @param callback optional callback before casting chunk view
 */
SKR_RUNTIME_API void sugoiS_cast_view_group(sugoi_storage_t* storage, const sugoi_chunk_view_t* view, const sugoi_group_t* group, sugoi_cast_callback_t callback, void* u);

/**
 * @brief change entities' type
 * move whole group to another group, the original group will be destoryed
 * @param group
 * @param type
 */
SKR_RUNTIME_API void sugoiS_cast_group_delta(sugoi_storage_t* storage, sugoi_group_t* group, const sugoi_delta_type_t* delta, sugoi_cast_callback_t callback, void* u);
/**
 * @brief get the chunk view of an entity
 * entity it self does not contain any data, get the chunk view of an entity to access it's data (all structural change apis and data access apis is based on chunk view)
 * @param storage
 * @param ent
 * @param view
 */
SKR_RUNTIME_API void sugoiS_access(sugoi_storage_t* storage, sugoi_entity_t ent, sugoi_chunk_view_t* view);
/**
 * @brief get the chunk view of entities
 * entity it self does not contain any data, get the chunk view of entities to access it's data (all structural change api and data access api is based on chunk view)
 * get the chunk view of entities will try batch continuous entities into single chunk view to get better performace
 * @param storage
 * @param ents
 * @param count
 * @param callback callback for each batched chunk view
 */
SKR_RUNTIME_API void sugoiS_batch(sugoi_storage_t* storage, const sugoi_entity_t* ents, EIndex count, sugoi_view_callback_t callback, void* u);
/**
 * @brief get all chunk view matching given filter
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
SKR_RUNTIME_API void sugoiS_query(sugoi_storage_t* storage, const sugoi_filter_t* filter, const sugoi_meta_filter_t* meta, sugoi_view_callback_t callback, void* u);
/**
 * @brief get all chunk view
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
SKR_RUNTIME_API void sugoiS_all(sugoi_storage_t* storage, bool includeDisabled, bool includeDead, sugoi_view_callback_t callback, void* u);
/**
 * @brief get entity count
 * @param storage
 * @return EIndex
 */
SKR_RUNTIME_API EIndex sugoiS_count(sugoi_storage_t* storage, bool includeDisabled, bool includeDead);
/**
 * @brief get all groups matching given filter
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
SKR_RUNTIME_API void sugoiS_query_groups(sugoi_storage_t* storage, const sugoi_filter_t* filter, const sugoi_meta_filter_t* meta, sugoi_group_callback_t callback, void* u);
/**
 * @brief merge two storage
 * after merge, the source storage will be empty
 * small chunks and empty chunks (fragment) will appear when merge storages which just move chunks(which could be small one or empty one) from source storage
 * every time we merge storage, we get more fragment
 * @see sugoiS_defragement
 * @param storage
 * @param source
 */
SKR_RUNTIME_API void sugoiS_merge(sugoi_storage_t* storage, sugoi_storage_t* source);
/**
 * @brief diff two storage
 *
 * @param storage
 * @param target
 */
SKR_RUNTIME_API sugoi_storage_delta_t* sugoiS_diff(sugoi_storage_t* storage, sugoi_storage_t* target);
/**
 * @brief serialize the storage
 *
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 * @see sugoi_serializer_v
 */
SKR_RUNTIME_API void sugoiS_serialize(sugoi_storage_t* storage, SBinaryWriter* v);
/**
 * @brief deserialize the storage
 *
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 * @see sugoi_serializer_v
 */
SKR_RUNTIME_API void sugoiS_deserialize(sugoi_storage_t* storage, SBinaryReader* v);
/**
 * @brief test if given entity exist in storage
 * entity can be invalid(id not exist) or be dead(version not match)
 * @param storage
 * @param ent
 * @return bool
 */
SKR_RUNTIME_API int sugoiS_exist(sugoi_storage_t* storage, sugoi_entity_t ent);
/**
 * @brief test if given components is enabled on given ent
 * if there's no mask component on given ent, all components consider enabled
 * @param storage
 * @param ent
 * @param types
 */
SKR_RUNTIME_API int sugoiS_components_enabled(sugoi_storage_t* storage, sugoi_entity_t ent, const sugoi_type_set_t* types);
/**
 * @brief deserialize entity, if there's multiple entity, they will be deserialized together
 * @param storage
 * @param v serializer callback
 */
SKR_RUNTIME_API sugoi_entity_t sugoiS_deserialize_entity(sugoi_storage_t* storage, SBinaryReader* v);
/**
 * @brief serialize entity
 * @param storage
 * @param ent
 * @param v
 */
SKR_RUNTIME_API void sugoiS_serialize_entity(sugoi_storage_t* storage, sugoi_entity_t ent, SBinaryWriter* v);
/**
 * @brief serialize entities, internal reference will be kept
 * @param storage
 * @param ent
 * @param v
 */
SKR_RUNTIME_API void sugoiS_serialize_entities(sugoi_storage_t* storage, sugoi_entity_t* ents, EIndex n, SBinaryWriter* v);
/**
 * @brief reset the storage
 * release all entities and all queries
 * @param storage
 */
SKR_RUNTIME_API void sugoiS_reset(sugoi_storage_t* storage);
/**
 * @brief validate all groups' meta type
 * invalid meta will be removed
 * @param storage
 */
SKR_RUNTIME_API void sugoiS_validate_meta(sugoi_storage_t* storage);
/**
 * @brief merge empty chunks
 *
 * @see sugoiS_merge
 * @param storage
 */
SKR_RUNTIME_API void sugoiS_defragement(sugoi_storage_t* storage);
/**
 * @brief pack entity id
 * when we destroy an entity, we don't "delete" it's id, we just left a hole awaiting reuse.
 * when we serialize the storage, we serialize those holes too. use this function to remap entities' id
 * @param storage
 */
SKR_RUNTIME_API void sugoiS_pack_entities(sugoi_storage_t* storage);
/**
 * @brief create a query which combine filter and parameters
 * query can be overloaded
 * @param storage
 * @param filter
 * @param params
 * @return created query
 */
SKR_RUNTIME_API sugoi_query_t* sugoiQ_create(sugoi_storage_t* storage, const sugoi_filter_t* filter, const sugoi_parameters_t* params);
/**
 * @brief create an alias for a component with unique phase to work with overloded query
 *
 * @param query
 * @param component
 * @param alias
 */
SKR_RUNTIME_API void sugoiQ_make_alias(sugoi_storage_t* storage, const char8_t* component, const char8_t* alias);
/**
 * @brief release a query
 *
 * @param query
 */
SKR_RUNTIME_API void sugoiQ_release(sugoi_query_t* query);
/**
 * @brief create a query from string
 * use dsl to descript filter and parameters info:
 * ? - optional
 * $ - shared
 * * - random access
 * | - any
 * ! - none
 * ' - stage
 * eg. [inout]fuck|masturbate', [in]?shit, [in]$sucker, *cocks, !bastard
 * @param storage
 * @param desc
 * @return sugoi_query_t*
 */
SKR_RUNTIME_API sugoi_query_t* sugoiQ_from_literal(sugoi_storage_t* storage, const char8_t* desc);

SKR_RUNTIME_API void sugoiQ_add_child(sugoi_query_t* query, sugoi_query_t* child);

SKR_RUNTIME_API const char8_t* sugoiQ_get_error();

SKR_RUNTIME_API void sugoiQ_sync(sugoi_query_t* query);

SKR_RUNTIME_API EIndex sugoiQ_get_count(sugoi_query_t* query);

SKR_RUNTIME_API void sugoiQ_get(sugoi_query_t* query, sugoi_filter_t* filter, sugoi_parameters_t* params);
/**
 * @brief set meta filter for a query
 * note: query does not own this meta, user should care about meta's lifetime
 * @param query
 * @param meta pass nullptr to clear meta
 */
SKR_RUNTIME_API void sugoiQ_set_meta(sugoi_query_t* query, const sugoi_meta_filter_t* meta);
/**
 * @brief set custom filter callback for a query
 * note: query does not own userdata
 * @param query
 * @param callback
 * @param u
 * @return SKR_RUNTIME_API
 */
SKR_RUNTIME_API void sugoiQ_set_custom_filter(sugoi_query_t* query, sugoi_custom_filter_callback_t callback, void* u);
/**
 * @brief get filtered chunk view from query
 *
 * @param storage
 * @param query
 * @param callback callback for each filtered chunk view
 */
SKR_RUNTIME_API void             sugoiQ_get_views(sugoi_query_t* query, sugoi_view_callback_t callback, void* u);
SKR_RUNTIME_API void             sugoiQ_get_groups(sugoi_query_t* query, sugoi_group_callback_t callback, void* u);
SKR_RUNTIME_API void             sugoiQ_get_views_group(sugoi_query_t* query, sugoi_group_t* group, sugoi_view_callback_t callback, void* u);
SKR_RUNTIME_API sugoi_storage_t* sugoiQ_get_storage(sugoi_query_t* query);

/**
 * @brief test if group contains components, whether owned or shared
 *
 * @param group
 * @param types
 */
SKR_RUNTIME_API int sugoiG_has_components(const sugoi_group_t* group, const sugoi_type_set_t* types);
/**
 * @brief test if group owns components, owned component is stored in this group
 *
 * @param group
 * @param types
 */
SKR_RUNTIME_API int sugoiG_own_components(const sugoi_group_t* group, const sugoi_type_set_t* types);
/**
 * @brief test if group shares components, shared component is owned by meta entities
 *
 * @param group
 * @param types
 */
SKR_RUNTIME_API int sugoiG_share_components(const sugoi_group_t* group, const sugoi_type_set_t* types);
/**
 * @brief get shared component data from group
 *
 * @param group
 * @param type
 * @return void const*
 */
SKR_RUNTIME_API const void* sugoiG_get_shared_ro(const sugoi_group_t* group, sugoi_type_index_t type);
/**
 * @brief get entity type from group
 *
 * @param group
 * @param type
 */
SKR_RUNTIME_API void sugoiG_get_type(const sugoi_group_t* group, sugoi_entity_type_t* type);
/**
 * @brief get type stable order, flag component will be ignored
 *
 * @param group
 * @param order
 * @return uint32_t
 */
SKR_RUNTIME_API uint32_t sugoiG_get_stable_order(const sugoi_group_t* group, sugoi_type_index_t localType);
/**
 * @brief get component from chunk view readonly return null if component is not exist
 *
 * @param view
 * @param type
 * @return void const*
 */
SKR_RUNTIME_API const void* sugoiV_get_component_ro(const sugoi_chunk_view_t* view, sugoi_type_index_t type);
/**
 * @brief get component from chunk view readonly return null if component is not exist or not owned
 *
 * @param view
 * @param type
 * @return void const*
 */
SKR_RUNTIME_API const void* sugoiV_get_owned_ro(const sugoi_chunk_view_t* view, sugoi_type_index_t type);
/**
 * @brief get component from chunk view readwrite return null if component is not exist
 *
 * @param view
 * @param type
 * @return void*
 */
SKR_RUNTIME_API void* sugoiV_get_owned_rw(const sugoi_chunk_view_t* view, sugoi_type_index_t type);
/**
 * @brief get component from chunk view readonly return null if component is not exist or not owned
 *
 * @param view
 * @param type
 * @return void const*
 */
SKR_RUNTIME_API const void* sugoiV_get_owned_ro_local(const sugoi_chunk_view_t* view, sugoi_type_index_t localType);
/**
 * @brief get component from chunk view readwrite return null if component is not exist
 *
 * @param view
 * @param type
 * @return void*
 */
SKR_RUNTIME_API void* sugoiV_get_owned_rw_local(const sugoi_chunk_view_t* view, sugoi_type_index_t localType);
/**
 * @brief get local type id of a component from chunk view
 *
 * @param view
 * @param type
 * @return sugoi_type_index_t
 */
SKR_RUNTIME_API sugoi_type_index_t sugoiV_get_local_type(const sugoi_chunk_view_t* view, sugoi_type_index_t type);
/**
 * @brief get component type id of a local type from chunk view
 *
 * @param view
 * @param type
 * @return sugoi_type_index_t
 */
SKR_RUNTIME_API sugoi_type_index_t sugoiV_get_component_type(const sugoi_chunk_view_t* view, sugoi_type_index_t type);
/**
 * @brief get entity list from chunk view
 *
 * @param view
 * @return sugoi_entity_t const*
 */
SKR_RUNTIME_API const sugoi_entity_t* sugoiV_get_entities(const sugoi_chunk_view_t* view);
/**
 * @brief copy data from
 *
 */
SKR_RUNTIME_API void sugoiV_copy(const sugoi_chunk_view_t* dst, const sugoi_chunk_view_t* src);
/**
 * @brief enable components in chunk view, has no effect if there's no mask component in this group
 *
 * @param view
 * @param types
 */
SKR_RUNTIME_API void sugoiS_enable_components(const sugoi_chunk_view_t* view, const sugoi_type_set_t* types);
/**
 * @brief disable components in chunk view, has no effect if there's no mask component in this group
 *
 * @param view
 * @param types
 */
SKR_RUNTIME_API void sugoiS_disable_components(const sugoi_chunk_view_t* view, const sugoi_type_set_t* types);

/**
 * @brief set version of storage, useful when detecting changes
 *
 * @param storage
 * @param number
 */
SKR_RUNTIME_API void sugoiS_set_version(sugoi_storage_t* storage, uint64_t number);

/**
 * @brief get group of chunk
 *
 * @param chunk
 */
SKR_RUNTIME_API sugoi_group_t* sugoiC_get_group(const sugoi_chunk_t* chunk);
/**
 * @brief get storage of chunk
 *
 * @param chunk
 */
SKR_RUNTIME_API sugoi_storage_t* sugoiC_get_storage(const sugoi_chunk_t* chunk);
/**
 * @brief get count of chunk
 *
 * @param chunk
 */
SKR_RUNTIME_API uint32_t sugoiC_get_count(const sugoi_chunk_t* chunk);

SKR_RUNTIME_API void sugoi_set_bit(uint32_t* mask, int32_t bit);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

/**
 * @brief register a resource to scheduler
 *
 */
SKR_RUNTIME_API sugoi_entity_t sugoiJ_add_resource();
/**
 * @brief remove a resource from scheduler
 *
 */
SKR_RUNTIME_API void sugoiJ_remove_resource(sugoi_entity_t id);
typedef uint32_t     sugoi_thread_index_t;
typedef void (*sugoi_system_callback_t)(void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex);
typedef void (*sugoi_system_lifetime_callback_t)(void* u, EIndex entityCount);
typedef struct sugoi_resource_operation_t {
    sugoi_entity_t* resources;
    int*            readonly;
    int*            atomic;
    uint32_t        count;
} sugoi_resource_operation_t;
/**
 * @brief schedule an ecs job with a query, filter runs in parallel, dependencies between ecs jobs are automatically resolved
 *
 * @param query
 * @param batchSize max entity count processed by a task, be aware of false sharing when batchSize is small
 * @param callback processor function, called multiple times in parallel
 * @param u
 * @param init initializer function, called at the beginning of job
 * @param resources
 * @param counter counter used to sync jobs, if *counter is null, a new counter will be created
 * @return false if job is skipped
 */
SKR_RUNTIME_API bool sugoiJ_schedule_ecs(sugoi_query_t* query, EIndex batchSize, sugoi_system_callback_t callback, void* u,
                                         sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources, skr::task::event_t* counter);

typedef void (*sugoi_schedule_callback_t)(void* u, sugoi_query_t* query);
/**
 * @brief schedule custom job and sync with ecs context
 *
 * @param query
 * @param counter
 * @param callback
 * @param u
 * @param resources
 */
SKR_RUNTIME_API void sugoiJ_schedule_custom(sugoi_query_t* query, sugoi_schedule_callback_t callback, void* u,
                                            sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources, skr::task::event_t* counter);
/**
 * @brief wait for all jobs are done
 *
 */
SKR_RUNTIME_API void sugoiJ_wait_all();
/**
 * @brief clear all expired entry handles
 *
 */
SKR_RUNTIME_API void sugoiJ_gc();
/**
 * @brief wait for all ecs jobs are done
 *
 */
SKR_RUNTIME_API void sugoiJ_wait_storage(sugoi_storage_t* storage);
/**
 * @brief enable job for storage
 *
 */
SKR_RUNTIME_API void sugoiJ_bind_storage(sugoi_storage_t* storage);
/**
 * @brief disable job for storage
 *
 */
SKR_RUNTIME_API void sugoiJ_unbind_storage(sugoi_storage_t* storage);

template <class C>
struct sugoi_id_of {
    static sugoi_type_index_t get()
    {
        static_assert(!sizeof(C), "sugoi_id_of<C> not implemented for this type, please include the appropriate generated header!");
    }
};

namespace sugoi
{
struct storage_scope_t {
    sugoi_storage_t* storage = nullptr;
    storage_scope_t(sugoi_storage_t* storage)
        : storage(storage)
    {
        sugoiJ_bind_storage(storage);
    }
    ~storage_scope_t()
    {
        sugoiJ_unbind_storage(storage);
    }
};

struct guid_comp_t {
    sugoi_guid_t value;
};

struct mask_comp_t {
    sugoi_mask_comp_t value;
};

struct dirty_comp_t {
    sugoi_dirty_comp_t value;
};

template <SugoiCallbackFlags flags = SUGOI_CALLBACK_FLAG_ALL, class C>
void managed_component(sugoi_type_description_t& desc, skr::type_t<C>)
{
    if constexpr ((flags & SUGOI_CALLBACK_FLAG_CTOR) != 0)
    {
        if constexpr (std::is_default_constructible_v<C>)
            desc.callback.constructor = +[](sugoi_chunk_t* chunk, EIndex index, char* data) {
                new (data) C();
            };
    }
    if constexpr ((flags & SUGOI_CALLBACK_FLAG_DTOR) != 0)
    {
        if constexpr (std::is_destructible_v<C>)
            desc.callback.destructor = +[](sugoi_chunk_t* chunk, EIndex index, char* data) {
                ((C*)data)->~C();
            };
    }
    if constexpr ((flags & SUGOI_CALLBACK_FLAG_COPY) != 0)
    {
        if constexpr (std::is_copy_constructible_v<C>)
            desc.callback.copy = +[](sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src) {
                new (dst) C(*(const C*)src);
            };
    }
    if constexpr ((flags & SUGOI_CALLBACK_FLAG_MOVE) != 0)
    {
        if constexpr (std::is_move_constructible_v<C>)
            desc.callback.move = +[](sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, char* src) {
                new (dst) C(std::move(*(C*)src));
            };
    }
}

template <class C>
void check_managed(const sugoi_type_description_t& desc, skr::type_t<C>)
{
    if constexpr (!std::is_trivially_constructible_v<C>)
    {
        if (desc.callback.constructor == nullptr)
        {
            SKR_LOG_WARN(u8"type %s is not trivially constructible but no contructor was provided.", desc.name);
        }
    }
    if constexpr (!std::is_trivially_destructible_v<C>)
    {
        if (desc.callback.destructor == nullptr)
        {
            SKR_LOG_WARN(u8"type %s is not trivially destructible but no destructor was provided.", desc.name);
        }
    }
    if constexpr (!std::is_trivially_copy_constructible_v<C>)
    {
        if (desc.callback.copy == nullptr)
        {
            SKR_LOG_WARN(u8"type %s is not trivially copy constructible but no copy constructor was provided.", desc.name);
        }
    }
    if constexpr (!std::is_trivially_move_constructible_v<C>)
    {
        if (desc.callback.move == nullptr)
        {
            SKR_LOG_WARN(u8"type %s is not trivially move constructible but no move constructor was provided.", desc.name);
        }
    }
}

template <class T>
auto get_component_ro(sugoi_chunk_view_t* view)
{
    static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "T must be a type declare!");
    return (std::add_const_t<std::decay_t<T>>*)sugoiV_get_component_ro(view, sugoi_id_of<T>::get());
}

template <class T>
T* get_owned_rw(sugoi_chunk_view_t* view)
{
    static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "T must be a type declare!");
    return (T*)sugoiV_get_owned_rw(view, sugoi_id_of<T>::get());
}

template <class T, class V>
V* get_owned_rw(sugoi_chunk_view_t* view)
{
    static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "T must be a type declare!");
    return (V*)sugoiV_get_owned_rw(view, sugoi_id_of<T>::get());
}

template <class T>
auto get_owned_ro(sugoi_chunk_view_t* view)
{
    static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "T must be a type declare!");
    return (std::add_const_t<std::decay_t<T>>*)sugoiV_get_owned_ro(view, sugoi_id_of<T>::get());
}

template <class T, class V>
auto get_owned_ro(sugoi_chunk_view_t* view)
{
    static_assert(!std::is_pointer_v<T> && !std::is_reference_v<T>, "T must be a type declare!");
    return (std::add_const_t<std::decay_t<V>>*)sugoiV_get_owned_ro(view, sugoi_id_of<T>::get());
}

struct task_context_t {
    sugoi_storage_t*    storage;
    sugoi_chunk_view_t* view;
    sugoi_type_index_t* localTypes;
    EIndex              entityIndex;
    sugoi_query_t*      query;
    const void*         paramPtrs[32];
    sugoi_parameters_t  params;
    task_context_t(sugoi_storage_t* storage, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex, sugoi_query_t* query)
        : storage(storage)
        , view(view)
        , localTypes(localTypes)
        , entityIndex(entityIndex)
        , query(query)
    {
        SKR_ASSERT(params.length < 32); // TODO: support more than 32 params
        sugoiQ_get(query, nullptr, &params);
        for (TIndex i = 0; i < params.length; ++i)
        {
            if (params.accesses[i].readonly)
                paramPtrs[i] = sugoiV_get_owned_ro_local(view, localTypes[i]);
            else
                paramPtrs[i] = sugoiV_get_owned_rw_local(view, localTypes[i]);
        }
    }

    auto count() { return view->count; }

    const sugoi_entity_t* get_entities() { return sugoiV_get_entities(view); }

    template <class T>
    void check_local_type(sugoi_type_index_t idx)
    {
        if (localTypes == nullptr)
            return;
        auto localType = localTypes[idx];
        if (localType == kInvalidTypeIndex)
            return;
        SKR_ASSERT(sugoiV_get_component_type(view, localTypes[idx]) == sugoi_id_of<T>::get());
    }

    void check_access(sugoi_type_index_t idx, bool readonly, bool random = false)
    {
        SKR_ASSERT(params.accesses[idx].readonly == static_cast<int>(readonly));
        SKR_ASSERT(params.accesses[idx].randomAccess >= static_cast<int>(random));
    }

    template <class T, bool noCheck = false>
    T* get_owned_rw(sugoi_type_index_t idx)
    {
        if constexpr (!noCheck)
        {
            check_local_type<T>(idx);
            check_access(idx, false);
        }
        return (T*)paramPtrs[idx];
    }

    template <class T, bool noCheck = false>
    T* get_owned_rw(sugoi_chunk_view_t* view, sugoi_type_index_t idx)
    {
        if constexpr (!noCheck)
        {
            check_local_type<T>(idx);
            check_access(idx, false, true);
        }
        return (T*)sugoiV_get_owned_rw(view, sugoi_id_of<T>::get());
    }

    template <class T, bool noCheck = false>
    const T* get_owned_ro(sugoi_type_index_t idx)
    {
        if constexpr (!noCheck)
        {
            check_local_type<T>(idx);
            check_access(idx, true);
        }
        return (const T*)paramPtrs[idx];
    }

    template <class T, bool noCheck = false>
    const T* get_owned_ro(sugoi_chunk_view_t* view, sugoi_type_index_t idx)
    {
        if constexpr (!noCheck)
        {
            check_local_type<T>(idx);
            check_access(idx, true, true);
        }
        return (const T*)sugoiV_get_owned_ro(view, sugoi_id_of<T>::get());
    }

    void set_dirty(dirty_comp_t& mask, sugoi_type_index_t idx)
    {
        check_access(idx, false);
        sugoi_set_bit(&mask.value, localTypes[idx]);
    }
};
struct query_t {
    sugoi_query_t* query = nullptr;
    ~query_t()
    {
        if (query)
            sugoiQ_release(query);
    }
    explicit operator bool() const
    {
        return query != nullptr;
    }
    sugoi_query_t*& operator*()
    {
        return query;
    }
    sugoi_query_t* const& operator*() const
    {
        return query;
    }
};

struct QWildcard {
    using TaskContext = task_context_t;
    QWildcard(sugoi_query_t* query)
        : query(query)
    {
    }
    sugoi_query_t* query = nullptr;
};

template <class T, class F>
bool schedule_task(T query, EIndex batchSize, F callback, skr::task::event_t* counter)
{
    static constexpr auto convertible_to_function_check = [](auto t) -> decltype(+t) { return +t; };
    using TaskContext                                   = typename T::TaskContext;
    if constexpr (std::is_invocable_v<decltype(convertible_to_function_check), F>)
    {
        static constexpr auto callbackType = +callback;
        auto                  trampoline   = +[](void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
            TaskContext ctx{ sugoiQ_get_storage(query), view, localTypes, entityIndex, query };
            callbackType(ctx);
        };
        return sugoiJ_schedule_ecs(query.query, batchSize, trampoline, nullptr, nullptr, nullptr, nullptr, counter);
    }
    else
    {
        struct payload {
            F callback;
        };
        auto trampoline = +[](void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
            payload*    p = (payload*)u;
            TaskContext ctx{ sugoiQ_get_storage(query), view, localTypes, entityIndex, query };
            p->callback(ctx);
        };
        payload* p        = SkrNew<payload>(std::move(callback));
        auto     teardown = +[](void* u, EIndex entityCount) {
            payload* p = (payload*)u;
            SkrDelete(p);
        };
        return sugoiJ_schedule_ecs(query.query, batchSize, trampoline, p, nullptr, teardown, nullptr, counter);
    }
}

template <class F>
auto schedule_task(sugoi_query_t* query, EIndex batchSize, F callback, skr::task::event_t* counter)
{
    return schedule_task(sugoi::QWildcard{ query }, batchSize, std::move(callback), counter);
}

template <class T, class F>
auto schesugoi_custom(T query, F callback, skr::task::event_t* counter)
{
    static constexpr auto convertible_to_function_check = [](auto t) -> decltype(+t) { return +t; };
    using TaskContext                                   = typename T::TaskContext;
    if constexpr (std::is_invocable_v<decltype(convertible_to_function_check), F>)
    {
        static constexpr auto callbackType = +callback;
        auto                  trampoline   = +[](void* u, sugoi_query_t* query) {
            TaskContext ctx{ sugoiQ_get_storage(query), nullptr, nullptr, 0, query };
            callbackType(ctx);
        };
        return sugoiJ_schedule_custom(query.query, trampoline, nullptr, nullptr, nullptr, nullptr, counter);
    }
    else
    {
        struct payload {
            F callback;
        };
        auto trampoline = +[](void* u, sugoi_query_t* query) {
            payload*    p = (payload*)u;
            TaskContext ctx{ sugoiQ_get_storage(query), nullptr, nullptr, 0, query };
            p->callback(ctx);
        };
        payload* p        = SkrNew<payload>(std::move(callback));
        auto     teardown = +[](void* u, EIndex entityCount) {
            payload* p = (payload*)u;
            SkrDelete(p);
        };
        return sugoiJ_schedule_custom(query.query, trampoline, p, nullptr, teardown, nullptr, counter);
    }
}

template <class F>
auto schesugoi_custom(sugoi_query_t* query, F callback, skr::task::event_t* counter)
{
    return schesugoi_custom<sugoi::QWildcard, F>(sugoi::QWildcard{ query }, std::move(callback), counter);
}

template <class T>
T* get_owned(sugoi_chunk_view_t* view)
{
    if constexpr (std::is_const_v<T>)
    {
        return get_owned_ro<std::remove_const_t<T>>(view);
    }
    else
    {
        return get_owned_rw<T>(view);
    }
}

template <class T1, class T2, class... T>
std::tuple<T1, T2, T*...> get_singleton(sugoi_query_t* query)
{
    std::tuple<T1, T2, T*...> result;
    bool                      singleton = true;
    auto                      callback  = [&](sugoi_chunk_view_t* view) {
        SKR_ASSERT(singleton);
        SKR_ASSERT(view->count == 1);
        result = std::make_tuple(get_owned<T1>(view), get_owned<T2>(view), get_owned<T>(view)...);
    };
    sugoiQ_get_views(query, SUGOI_LAMBDA(callback));
    return result;
}

template <class T>
T* get_singleton(sugoi_query_t* query)
{
    T*   result;
    bool singleton = true;
    auto callback  = [&](sugoi_chunk_view_t* view) {
        SKR_ASSERT(singleton);
        SKR_ASSERT(view->count == 1);
        result = get_owned<T>(view);
    };
    sugoiQ_get_views(query, SUGOI_LAMBDA(callback));
    return result;
}
} // namespace sugoi

template <>
struct SKR_RUNTIME_API sugoi_id_of<sugoi::dirty_comp_t> {
    static sugoi_type_index_t get();
};

template <>
struct SKR_RUNTIME_API sugoi_id_of<sugoi::mask_comp_t> {
    static sugoi_type_index_t get();
};

template <>
struct SKR_RUNTIME_API sugoi_id_of<sugoi::guid_comp_t> {
    static sugoi_type_index_t get();
};

    #define QUERY_CONBINE_GENERATED_NAME(file, type) QUERY_CONBINE_GENERATED_NAME_IMPL(file, type)
    #define QUERY_CONBINE_GENERATED_NAME_IMPL(file, type) GENERATED_QUERY_BODY_##file##_##type
    #ifdef __meta__
        #define GENERATED_QUERY_BODY(type) sugoi_query_t* query;
    #else
        #define GENERATED_QUERY_BODY(type) QUERY_CONBINE_GENERATED_NAME(SKR_FILE_ID, type)
    #endif

#endif