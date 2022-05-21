#pragma once
#include "dual_config.h"
#include "ecs/dual.h"
#include "platform/guid.h"

#if defined(__cplusplus)
extern "C" {
#endif
// objects
#define DUAL_DECLARE(name) typedef struct dual_##name dual_##name
DUAL_DECLARE(context_t);
DUAL_DECLARE(storage_t);
DUAL_DECLARE(group_t);
DUAL_DECLARE(chunk_t);
DUAL_DECLARE(query_t);
DUAL_DECLARE(storage_delta_t);
DUAL_DECLARE(counter_t);
#undef DUAL_DECLARE

// structs
typedef TIndex dual_type_index_t;

typedef skr_guid_t dual_guid_t;

/**
 * @brief guid generation function
 *
 */
typedef void (*guid_func_t)(dual_guid_t* guid);

typedef struct dual_serializer_v {
    void (*stream)(void* s, void* data, uint32_t bytes);
    void (*peek)(void* s, void* data, uint32_t bytes);
    int (*is_serialize)(void* s);
} dual_serializer_v;

typedef struct dual_mapper_t dual_mapper_t;

typedef struct dual_mapper_v {
    void (*map)(dual_mapper_t* mapper, dual_entity_t* ent);
} dual_mapper_v;

typedef struct dual_callback_v {
    void (*constructor)(dual_chunk_t* chunk, EIndex index, char* data);
    void (*copy)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src);
    void (*destructor)(dual_chunk_t* chunk, EIndex index, char* data);
    void (*move)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src);
    void (*serialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, const dual_serializer_v* v, void* s);
    void (*map)(dual_chunk_t* chunk, EIndex index, char* data, dual_mapper_t* p, dual_mapper_v* v);
} dual_callback_v;

enum type_flags
{
    DTF_PIN = 0x1,
};

/**
 * @brief describe basic infomation of a component type
 *
 */
typedef struct dual_type_description_t {
    dual_guid_t guid;
    const char* name;
    /**
     * a pinned component will not removed when destroying or copy when instantiating, and user should remove them manually
     * destroyed entity with pinned component will be marked by a dead component and will be erased when all pinned component is removed
     */
    int flags;
    /**
     * the storage size in chunk of this component, generally it is sizeof(T)
     * when this is a array component, it could be sizeof(T) * I + sizeof(dual_array_component_t) where I means the inline element count of the array
     * @see dual_array_component_t
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
    // lifetime callbacks of this component
    dual_callback_v callback;
} dual_type_description_t;

typedef struct dual_chunk_view_t {
    dual_chunk_t* chunk;
    EIndex start;
    EIndex count;
} dual_chunk_view_t;

/**
 * @brief set of component types
 * note. type set does not own the data, and all data should be in order
 */
typedef struct dual_type_set_t {
    const dual_type_index_t* data;
    SIndex length;
} dual_type_set_t;

/**
 * @brief set of meta types
 * meta type meaning entity as type
 * note. type set does not own the data, and all data should be in order
 */
typedef struct dual_entity_set_t {
    const dual_entity_t* data;
    SIndex length;
} dual_entity_set_t;

typedef struct dual_entity_type_t {
    dual_type_set_t type;
    dual_entity_set_t meta;
} dual_entity_type_t;

/**
 * @brief difference between two type
 *
 */
typedef struct dual_delta_type_t {
    dual_entity_type_t added;
    dual_entity_type_t removed;
} dual_delta_type_t;

/**
 * @brief entity filter
 *
 */
typedef struct dual_filter_t {
    // filter owned types
    dual_type_set_t all;
    dual_type_set_t any;
    dual_type_set_t none;
    // filter shared types
    dual_type_set_t all_shared;
    dual_type_set_t any_shared;
    dual_type_set_t none_shared;
} dual_filter_t;

enum dual_operation_scope
{
    DOS_SEQ,
    DOS_GROUP,
    DOS_GLOBAL,
};

/**
 * @brief describes the operation to a component
 *
 */
typedef struct dual_operation_t {
    int phase;    //-1 means any phase
    int readonly; // read or write
    int atomic;
    int randomAccess; // random access
} dual_operation_t;

/**
 * @brief describes the operation to components within one task
 *
 */
typedef struct dual_parameters_t {
    dual_type_index_t* types;
    dual_operation_t* accesses;
    TIndex length;
} dual_parameters_t;

// runtime type (context sensitive) filter
typedef struct dual_meta_filter_t {
    dual_entity_set_t all_meta;
    dual_entity_set_t any_meta;
    dual_entity_set_t none_meta;
    dual_type_set_t changed;
    uint64_t timestamp;
} dual_meta_filter_t;

// header data of a array component
typedef struct dual_array_component_t dual_array_component_t;

typedef uint32_t dual_mask_component_t;

// APIS
/**
 * @brief initialize context, user should store the context and pass it to library by implementing dual_get_context
 * @see dual_get_context
 * @return dual_context_t*
 */
RUNTIME_API dual_context_t* dual_initialize();
/**
 * @brief get context, used by library, implemented by user
 * @see dual_initialize
 * @return dual_context_t*
 */
RUNTIME_API dual_context_t* dual_get_context();
/**
 * @brief destroy context, shutdown library
 *
 */
RUNTIME_API void dual_shutdown();

RUNTIME_API void dual_make_guid(skr_guid_t* guid);

RUNTIME_API void* dualA_begin(dual_array_component_t* array);
RUNTIME_API void* dualA_end(dual_array_component_t* array);

typedef void (*dual_view_callback_t)(void* u, dual_chunk_view_t* view);
typedef void (*dual_group_callback_t)(void* u, dual_group_t* view);
typedef void (*dual_entity_callback_t)(void* u, dual_entity_t e);
typedef void (*dual_cast_callback_t)(void* u, dual_chunk_view_t* new_view, dual_chunk_view_t* old_view);

/**
 * @brief register a new component
 *
 * @param description
 * @return component type
 * @see dual_type_description_t
 */
RUNTIME_API dual_type_index_t dualT_register_type(dual_type_description_t* description);
/**
 * @brief get component type from guid
 *
 * @param guid
 * @return component type
 */
RUNTIME_API dual_type_index_t dualT_get_type(const dual_guid_t* guid);
/**
 * @brief get component type from name
 *
 * @param name
 * @return component type
 */
RUNTIME_API dual_type_index_t dualT_get_type_by_name(const char* name);
/**
 * @brief get description of component type
 *
 * @param idx
 * @return description
 */
RUNTIME_API const dual_type_description_t* dualT_get_desc(dual_type_index_t idx);
/**
 * @brief set guid generator function
 *
 * @param func
 */
RUNTIME_API void dualT_set_guid_func(guid_func_t func);

/**
 * @brief create a new storage
 *
 * @return new storage
 */
RUNTIME_API dual_storage_t* dualS_create();
/**
 * @brief release a storage
 * when storage is released, all entity and query within is deleted
 * @param storage
 */
RUNTIME_API void dualS_release(dual_storage_t* storage);
/**
 * @brief allocate entities
 * batch allocate numbers of entities with entity type
 * @param storage
 * @param type
 * @param count
 * @param callback optional callback after allocating chunk view
 */
RUNTIME_API void dualS_allocate_type(dual_storage_t* storage, const dual_entity_type_t* type, EIndex count, dual_view_callback_t callback, void* u);
/**
 * @brief allocate entities
 * batch allocate numbers of entities within a group
 * @param storage
 * @param group
 * @param count
 * @param callback optional callback after allocating chunk view
 */
RUNTIME_API void dualS_allocate_group(dual_storage_t* storage, dual_group_t* group, EIndex count, dual_view_callback_t callback, void* u);
/**
 * @brief instantiate entity
 * instantiate an entity n times
 * @param storage
 * @param prefab
 * @param count
 * @param callback optional callback after allocating chunk view
 */
RUNTIME_API void dualS_instantiate(dual_storage_t* storage, dual_entity_t prefab, EIndex count, dual_view_callback_t callback, void* u);
/**
 * @brief instantiate entities
 * instantiate entities n times, internal reference will be kept
 * @param storage
 * @param prefab
 * @param count
 * @param callback optional callback after allocating chunk view
 */
RUNTIME_API void dualS_instantiate_entities(dual_storage_t* storage, dual_entity_t* ents, EIndex n, EIndex count, dual_view_callback_t callback, void* u);
/**
 * @brief destroy entities in chunk view
 * destory all entities in target chunk view
 * @param storage
 * @param view
 */
RUNTIME_API void dualS_destroy(dual_storage_t* storage, const dual_chunk_view_t* view);
/**
 * @brief destory entities
 * destory all filtered entity
 * @param storage
 * @param ents
 * @param count
 */
RUNTIME_API void dualS_destroy_all(dual_storage_t* storage, const dual_meta_filter_t* meta);
/**
 * @brief change entities' type
 * change all entities' type in target chunk view by apply a delta type which will move entities to new group
 * there can be more than one chunk view after casting
 * @param storage
 * @param view
 * @param delta
 * @param callback optional callback before casting chunk view
 */
RUNTIME_API void dualS_cast_view_delta(dual_storage_t* storage, const dual_chunk_view_t* view, const dual_delta_type_t* delta, dual_cast_callback_t callback, void* u);
/**
 * @brief change entities' type
 * change all entities' type in target chunk view by move entities to new group
 * there can be more than one chunk view allocated
 * @param storage
 * @param view
 * @param group
 * @param callback optional callback before casting chunk view
 */
RUNTIME_API void dualS_cast_view_group(dual_storage_t* storage, const dual_chunk_view_t* view, const dual_group_t* group, dual_cast_callback_t callback, void* u);
/**
 * @brief get the chunk view of an entity
 * entity it self does not contain any data, get the chunk view of an entity to access it's data (all structural change apis and data access apis is based on chunk view)
 * @param storage
 * @param ent
 * @param view
 */
RUNTIME_API void dualS_access(dual_storage_t* storage, dual_entity_t ent, dual_chunk_view_t* view);
/**
 * @brief get the chunk view of entities
 * entity it self does not contain any data, get the chunk view of entities to access it's data (all structural change api and data access api is based on chunk view)
 * get the chunk view of entities will try batch continuous entities into single chunk view to get better performace
 * @param storage
 * @param ents
 * @param count
 * @param callback callback for each batched chunk view
 */
RUNTIME_API void dualS_batch(dual_storage_t* storage, const dual_entity_t* ents, EIndex count, dual_view_callback_t callback, void* u);
/**
 * @brief get all chunk view matching given filter
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
RUNTIME_API void dualS_query(dual_storage_t* storage, const dual_filter_t* filter, const dual_meta_filter_t* meta, dual_view_callback_t callback, void* u);
/**
 * @brief get all groups matching given filter
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
RUNTIME_API void dualS_query_groups(dual_storage_t* storage, const dual_filter_t* filter, const dual_meta_filter_t* meta, dual_group_callback_t callback, void* u);
/**
 * @brief merge two storage
 * after merge, the source storage will be empty
 * small chunks and empty chunks (fragment) will appear when merge storages which just move chunks(which could be small one or empty one) from source storage
 * every time we merge storage, we get more fragment
 * @see dualS_defragement
 * @param storage
 * @param source
 */
RUNTIME_API void dualS_merge(dual_storage_t* storage, dual_storage_t* source);
/**
 * @brief diff two storage
 *
 * @param storage
 * @param target
 */
RUNTIME_API dual_storage_delta_t* dualS_diff(dual_storage_t* storage, dual_storage_t* target);
/**
 * @brief serialize the storage
 *
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 * @see dual_serializer_v
 */
RUNTIME_API void dualS_serialize(dual_storage_t* storage, const dual_serializer_v* v, void* t);
/**
 * @brief deserialize the storage
 *
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 * @see dual_serializer_v
 */
RUNTIME_API void dualS_deserialize(dual_storage_t* storage, const dual_serializer_v* v, void* t);
/**
 * @brief test if given entity exist in storage
 * entity can be invalid(id not exist) or be dead(version not match)
 * @param storage
 * @param ent
 * @return bool
 */
RUNTIME_API int dualS_exist(dual_storage_t* storage, dual_entity_t ent);
/**
 * @brief test if given components is enabled on given ent
 * if there's no mask component on given ent, all components consider enabled
 * @param storage
 * @param ent
 * @param types
 */
RUNTIME_API int dualS_components_enabled(dual_storage_t* storage, dual_entity_t ent, const dual_type_set_t* types);
/**
 * @brief deserialize entity, if there's multiple entity, they will be deserialized together
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 */
RUNTIME_API dual_entity_t dualS_deserialize_entity(dual_storage_t* storage, const dual_serializer_v* v, void* t);
/**
 * @brief serialize entity
 * @param storage
 * @param ent
 * @param v
 * @param t
 */
RUNTIME_API void dualS_serialize_entity(dual_storage_t* storage, dual_entity_t ent, const dual_serializer_v* v, void* t);
/**
 * @brief serialize entities, internal reference will be kept
 * @param storage
 * @param ent
 * @param v
 * @param t
 */
RUNTIME_API void dualS_serialize_entities(dual_storage_t* storage, dual_entity_t* ents, EIndex n, const dual_serializer_v* v, void* t);
/**
 * @brief reset the storage
 * release all entities and all queries
 * @param storage
 */
RUNTIME_API void dualS_reset(dual_storage_t* storage);
/**
 * @brief validate all groups' meta type
 * invalid meta will be removed
 * @param storage
 */
RUNTIME_API void dualS_validate_meta(dual_storage_t* storage);
/**
 * @brief merge empty chunks
 *
 * @see dualS_merge
 * @param storage
 */
RUNTIME_API void dualS_defragement(dual_storage_t* storage);
/**
 * @brief pack entity id
 * when we destroy an entity, we don't "delete" it's id, we just left a hole awaiting reuse.
 * when we serialize the storage, we serialize those holes too. use this function to remap entities' id
 * @param storage
 */
RUNTIME_API void dualS_pack_entities(dual_storage_t* storage);
/**
 * @brief create a query which combine filter and parameters
 * query can be overloaded
 * @param storage
 * @param filter
 * @param params
 * @return created query
 */
RUNTIME_API dual_query_t* dualQ_create(dual_storage_t* storage, const dual_filter_t* filter, const dual_parameters_t* params);
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
 * @return dual_query_t*
 */
RUNTIME_API dual_query_t* dualQ_from_literal(dual_storage_t* storage, const char* desc);

RUNTIME_API const char* dualQ_get_error();
/**
 * @brief set meta filter for a query
 * note: query does not own this meta, user should care about meta's lifetime
 * @param query
 * @param meta pass nullptr to clear meta
 */
RUNTIME_API void dualQ_set_meta(dual_query_t* query, const dual_meta_filter_t* meta);
/**
 * @brief get filtered chunk view from query
 *
 * @param storage
 * @param query
 * @param callback callback for each filtered chunk view
 */
RUNTIME_API void dualQ_get_views(dual_query_t* query, dual_view_callback_t callback, void* u);
/**
 * @brief test if group contains components, whether owned or shared
 *
 * @param group
 * @param types
 */
RUNTIME_API int dualG_has_components(const dual_group_t* group, const dual_type_set_t* types);
/**
 * @brief test if group owns components, owned component is stored in this group
 *
 * @param group
 * @param types
 */
RUNTIME_API int dualG_own_components(const dual_group_t* group, const dual_type_set_t* types);
/**
 * @brief test if group shares components, shared component is owned by meta entities
 *
 * @param group
 * @param types
 */
RUNTIME_API int dualG_share_components(const dual_group_t* group, const dual_type_set_t* types);
/**
 * @brief get shared component data from group
 *
 * @param group
 * @param type
 * @return void const*
 */
RUNTIME_API const void* dualG_get_shared_ro(const dual_group_t* group, dual_type_index_t type);
/**
 * @brief get entity type from group
 *
 * @param group
 * @param type
 */
RUNTIME_API void dualG_get_type(const dual_group_t* group, dual_entity_type_t* type);
/**
 * @brief get component from chunk view readonly return null if component is not exist
 *
 * @param view
 * @param type
 * @return void const*
 */
RUNTIME_API const void* dualV_get_component_ro(const dual_chunk_view_t* view, dual_type_index_t type);
/**
 * @brief get component from chunk view readonly return null if component is not exist or not owned
 *
 * @param view
 * @param type
 * @return void const*
 */
RUNTIME_API const void* dualV_get_owned_ro(const dual_chunk_view_t* view, dual_type_index_t type);
/**
 * @brief get component from chunk view readwrite return null if component is not exist
 *
 * @param view
 * @param type
 * @return void*
 */
RUNTIME_API void* dualV_get_owned_rw(const dual_chunk_view_t* view, dual_type_index_t type);
/**
 * @brief get component from chunk view readonly return null if component is not exist or not owned
 *
 * @param view
 * @param type
 * @return void const*
 */
RUNTIME_API const void* dualV_get_owned_ro_local(const dual_chunk_view_t* view, dual_type_index_t localType);
/**
 * @brief get component from chunk view readwrite return null if component is not exist
 *
 * @param view
 * @param type
 * @return void*
 */
RUNTIME_API void* dualV_get_owned_rw_local(const dual_chunk_view_t* view, dual_type_index_t localType);
/**
 * @brief get entity list from chunk view
 *
 * @param view
 * @return dual_entity_t const*
 */
RUNTIME_API const dual_entity_t* dualV_get_entities(const dual_chunk_view_t* view);
/**
 * @brief enable components in chunk view, has no effect if there's no mask component in this group
 *
 * @param view
 * @param types
 */
RUNTIME_API void dualS_enable_components(const dual_chunk_view_t* view, const dual_type_set_t* types);
/**
 * @brief disable components in chunk view, has no effect if there's no mask component in this group
 *
 * @param view
 * @param types
 */
RUNTIME_API void dualS_disable_components(const dual_chunk_view_t* view, const dual_type_set_t* types);

/**
 * @brief set version of storage, useful when detecting changes
 *
 * @param storage
 * @param number
 */
RUNTIME_API void dualS_set_version(dual_storage_t* storage, uint64_t number);

/**
 * @brief get group of chunk
 *
 * @param chunk
 */
RUNTIME_API dual_group_t* dualC_get_group(const dual_chunk_t* chunk);
/**
 * @brief get count of chunk
 *
 * @param chunk
 */
RUNTIME_API uint32_t dualC_get_count(const dual_chunk_t* chunk);

/**
 * @brief register a resource to scheduler
 *
 */
RUNTIME_API dual_entity_t dualJ_add_resource();
/**
 * @brief remove a resource from scheduler
 *
 */
RUNTIME_API void dualJ_remove_resource(dual_entity_t id);
typedef uint32_t dual_thread_index_t;
typedef void (*dual_system_callback_t)(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex);
typedef void (*dual_system_init_callback_t)(void* u, EIndex entityCount);
typedef struct dual_resource_operation_t {
    dual_entity_t* resources;
    int* readonly;
    int* atomic;
    uint32_t count;
} dual_resource_operation_t;
/**
 * @brief schedule an ecs job with a query, filter runs in parallel, dependencies between ecs jobs are automatically resolved
 *
 * @param query
 * @param batchSize max entity count processed by a task, be aware of false sharing when batchSize is small
 * @param callback processor function, called multiple times in parallel
 * @param u
 * @param init initializer function, called at the beginning of job
 * @param resources
 * @return dual_job_t*
 */
RUNTIME_API void dualJ_schedule_ecs(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_init_callback_t init, dual_resource_operation_t* resources, dual_counter_t** counter);
typedef void (*dual_for_callback_t)(void* u, uint32_t index);
/**
 * @brief schedule multiple jobs
 *
 * @param count
 * @param callback
 * @param u
 * @return dual_job_t*
 */
RUNTIME_API void dualJ_schedule_for(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources, dual_counter_t** counter);
/**
 * @brief wait until counter equal to zero (when job is done)
 *
 * @param counter
 * @param pin stay on current thread
 */
RUNTIME_API void dualJ_wait_counter(dual_counter_t* counter, int pin);
/**
 * @brief release the counter
 *
 * @param counter
 */
RUNTIME_API void dualJ_release_counter(dual_counter_t* counter);
/**
 * @brief wait for all jobs are done
 *
 */
RUNTIME_API void dualJ_wait_all();
/**
 * @brief wait for all ecs jobs are done
 *
 */
RUNTIME_API void dualJ_wait_storage(dual_storage_t* storage);

typedef struct dual_scheduler_t dual_scheduler_t;
RUNTIME_API void dualJ_initialize(dual_scheduler_t* scheduler);
RUNTIME_API dual_scheduler_t* dualJ_get_scheduler();

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
template <class C>
struct dual_id_of {
    static dual_type_index_t get();
};
#endif