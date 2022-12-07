#pragma once
#include "dual_types.h"
#include "utils/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DUAL_COMPONENT_DISABLE 0x80000000
#define DUAL_COMPONENT_DEAD 0x80000001
#define DUAL_COMPONENT_LINK 0x20000002
#define DUAL_COMPONENT_MASK 0x3
#define DUAL_COMPONENT_GUID 0x4
#define DUAL_COMPONENT_DIRTY 0x5

/**
 * @brief guid generation function
 *
 */
typedef void (*guid_func_t)(dual_guid_t* guid);
typedef struct dual_mapper_t {
    void (*map)(void* user, dual_entity_t* ent) SKR_IF_CPP(=nullptr);
    void* user SKR_IF_CPP(=nullptr);
} dual_mapper_t;

typedef struct skr_binary_writer_t skr_binary_writer_t;
typedef struct skr_binary_reader_t skr_binary_reader_t;
typedef struct dual_callback_v {
    void (*constructor)(dual_chunk_t* chunk, EIndex index, char* data) SKR_IF_CPP(=nullptr);
    void (*copy)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src) SKR_IF_CPP(=nullptr);
    void (*destructor)(dual_chunk_t* chunk, EIndex index, char* data) SKR_IF_CPP(=nullptr);
    void (*move)(dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src) SKR_IF_CPP(=nullptr);
    void (*serialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* writer) SKR_IF_CPP(=nullptr);
    void (*deserialize)(dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* reader) SKR_IF_CPP(=nullptr);
    void (*map)(dual_chunk_t* chunk, EIndex index, char* data, dual_mapper_t* v) SKR_IF_CPP(=nullptr);
} dual_callback_v;

enum dual_type_flags SKR_IF_CPP(: uint32_t)
{
    DTF_PIN = 0x1,
    DTF_CHUNK = 0x2,
};

enum dual_callback_flags SKR_IF_CPP(: uint32_t)
{
    DCF_CTOR = 0x1,
    DCF_DTOR = 0x2,
    DCF_COPY = 0x4,
    DCF_MOVE = 0x8,
    DCF_SERDE = 0x10,
    DCF_ALL = DCF_CTOR | DCF_DTOR | DCF_COPY | DCF_MOVE | DCF_SERDE,
};

/**
 * @brief describe basic infomation of a component type
 *
 */
typedef struct dual_type_description_t {
    dual_guid_t guid;
    const char* name;
    const char* guidStr;
    /**
     * a pinned component will not removed when destroying or copy when instantiating, and user should remove them manually
     * destroyed entity with pinned component will be marked by a dead component and will be erased when all pinned component is removed
     */
    uint32_t flags;
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
    // resource field is used to track resource lifetime
    intptr_t resourceFields;
    uint32_t resourceFieldsCount;
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
typedef uint32_t dual_dirty_component_t;

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
typedef void (*dual_type_callback_t)(void* u, dual_type_index_t t);

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
 * @brief get all types registered to dual
 * 
 * @param callback 
 * @param u
 */
RUNTIME_API void dualT_get_types(dual_type_callback_t callback, void* u);
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
 * @brief instantiate entity as specific type
 * instantiate an entity n times as specific type
 * @param storage
 * @param prefab
 * @param count
 * @param delta
 * @param callback optional callback after allocating chunk view
 */
RUNTIME_API void dualS_instantiate_delta(dual_storage_t* storage, dual_entity_t prefab, EIndex count, const dual_delta_type_t* delta, dual_view_callback_t callback, void* u);
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
 * @brief get all chunk view
 *
 * @param storage
 * @param filter
 * @param meta
 * @param callback callback for filtered chunk view
 */
RUNTIME_API void dualS_all(dual_storage_t* storage, bool includeDisabled, bool includeDead, dual_view_callback_t callback, void* u);
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
RUNTIME_API void dualS_serialize(dual_storage_t* storage, skr_binary_writer_t* v);
/**
 * @brief deserialize the storage
 *
 * @param storage
 * @param v serializer callback
 * @param t serializer state
 * @see dual_serializer_v
 */
RUNTIME_API void dualS_deserialize(dual_storage_t* storage, skr_binary_reader_t* v);
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
 */
RUNTIME_API dual_entity_t dualS_deserialize_entity(dual_storage_t* storage, skr_binary_reader_t* v);
/**
 * @brief serialize entity
 * @param storage
 * @param ent
 * @param v
 */
RUNTIME_API void dualS_serialize_entity(dual_storage_t* storage, dual_entity_t ent, skr_binary_writer_t* v);
/**
 * @brief serialize entities, internal reference will be kept
 * @param storage
 * @param ent
 * @param v
 */
RUNTIME_API void dualS_serialize_entities(dual_storage_t* storage, dual_entity_t* ents, EIndex n, skr_binary_writer_t* v);
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
 * @brief release a query
 * 
 * @param query 
 */
RUNTIME_API void dualQ_release(dual_query_t* query);
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

RUNTIME_API void dualQ_get(dual_query_t* query, dual_filter_t* filter, dual_parameters_t* params);
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
RUNTIME_API void dualQ_get_groups(dual_query_t* query, dual_group_callback_t callback, void* u);
RUNTIME_API void dualQ_get_views_group(dual_query_t* query, dual_group_t* group, dual_view_callback_t callback, void* u);

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
 * @brief copy data from 
 * 
 */
RUNTIME_API void dualV_copy(const dual_chunk_view_t* dst, const dual_chunk_view_t* src);
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
 * @brief get storage of chunk
 *
 * @param chunk
 */
RUNTIME_API dual_storage_t* dualC_get_storage(const dual_chunk_t* chunk);
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
typedef void (*dual_system_lifetime_callback_t)(void* u, EIndex entityCount);
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
 * @param counter counter used to sync jobs, if *counter is null, a new counter will be created
 */
RUNTIME_API void dualJ_schedule_ecs(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources, dual_counter_t** counter);

typedef void (*dual_schedule_callback_t)(void* u, dual_counter_t* counter);
/**
 * @brief schedule custom job and sync with ecs context
 *
 * @param query
 * @param counter
 * @param callback
 * @param u
 * @param resources
 */
RUNTIME_API void dualJ_schedule_custom(const dual_query_t* query, dual_counter_t* counter, dual_schedule_callback_t callback, void* u, dual_resource_operation_t* resources);
/**
 * @brief create a counter object
 * counter is used to sync jobs, counter need to be released manually
 * @return dual_counter_t*
 */
RUNTIME_API dual_counter_t* dualJ_create_counter();
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
 * @brief clear all expired entry handles
 * 
 */
RUNTIME_API void dualJ_gc();
/**
 * @brief wait for all ecs jobs are done
 *
 */
RUNTIME_API void dualJ_wait_storage(dual_storage_t* storage);
/**
 * @brief enable job for storage
 *
 */
RUNTIME_API void dualJ_bind_storage(dual_storage_t* storage);
/**
 * @brief disable job for storage
 *
 */
RUNTIME_API void dualJ_unbind_storage(dual_storage_t* storage);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
template <class C>
struct dual_id_of {
    static dual_type_index_t get()
    {
        static_assert(!sizeof(C), "dual_id_of<C> not implemented for this type, please include the appropriate generated header!");
    }
};
#include "binary/reader.h"
#include "binary/writer.h"
#include "type/type_helper.hpp"
namespace dual
{
    struct storage_scope_t
    {
        dual_storage_t* storage = nullptr;
        storage_scope_t(dual_storage_t* storage)
            : storage(storage)
        {
            dualJ_bind_storage(storage);
        }
        ~storage_scope_t()
        {
            dualJ_unbind_storage(storage);
        }
    };

    struct query_t
    {
        dual_query_t* query = nullptr;
        ~query_t()
        {
            if(query)
                dualQ_release(query);
        }
        explicit operator bool() const
        {
            return query != nullptr;
        }
        dual_query_t*& operator*()
        {
            return query;
        }
        dual_query_t*const & operator*() const
        {
            return query;
        }
    };

    struct counter_t
    {
        dual_counter_t* counter;
        ~counter_t()
        {
            if(counter)
                dualJ_release_counter(counter);
        }
        explicit operator bool() const
        {
            return counter != nullptr;
        }
        dual_counter_t*& operator*()
        {
            return counter;
        }
        dual_counter_t* const& operator*() const
        {
            return counter;
        }
    };
    
    template<uint32_t flags = DCF_ALL, class C>
    void managed_component(dual_type_description_t& desc, skr::type_t<C>)
    {
        if constexpr ((flags & DCF_CTOR) != 0) {
            if constexpr (std::is_default_constructible_v<C>)
                desc.callback.constructor = +[](dual_chunk_t* chunk, EIndex index, char* data) {
                    new (data) C();
                };
        }
        if constexpr ((flags & DCF_DTOR) != 0) {
            if constexpr (std::is_destructible_v<C>)
                desc.callback.destructor = +[](dual_chunk_t* chunk, EIndex index, char* data) {
                    ((C*)data)->~C();
                };
        }
        if constexpr ((flags & DCF_COPY) != 0) {
            if constexpr (std::is_copy_constructible_v<C>)
                desc.callback.copy = +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src) {
                    new (dst) C(*(const C*)src);
                };
        }
        if constexpr ((flags & DCF_MOVE) != 0) {
            if constexpr (std::is_move_constructible_v<C>)
                desc.callback.move = +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src) {
                    new (dst) C(std::move(*(C*)src));
                };
        }
        if constexpr ((flags & DCF_SERDE) != 0) {
            if constexpr(skr::is_complete_serde_v<skr::binary::WriteTrait<C>>)
                desc.callback.serialize = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* writer) {
                    skr::binary::Write<const C&>(writer, *(C*)data);
                };
            if constexpr(skr::is_complete_serde_v<skr::binary::ReadTrait<C>>)
                desc.callback.deserialize = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* reader) {
                    skr::binary::Read(reader, *(C*)data);
                };
        }
    }

    template<class T>
    T* get_component_ro(dual_chunk_view_t* view)
    {
        return (T*)dualV_get_component_ro(view, dual_id_of<T>::get());
    }

    template<class T>
    T* get_owned_rw(dual_chunk_view_t* view)
    {
        return (T*)dualV_get_owned_rw(view, dual_id_of<T>::get());
    }
    
    template<class T, class V>
    V* get_owned_rw(dual_chunk_view_t* view)
    {
        return (V*)dualV_get_owned_rw(view, dual_id_of<T>::get());
    }
}

#endif