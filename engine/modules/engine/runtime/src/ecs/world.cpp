#include "SkrRuntime/ecs/world.hpp"

namespace skr::ecs
{

ArchetypeBuilder& ArchetypeBuilder::add_component(sugoi_type_index_t type)
{
    if (!types.contains(type))
    {
        types.add(type);
    }
    return *this;
}

ArchetypeBuilder& ArchetypeBuilder::add_meta_entity(Entity Entity)
{
    if (meta_entities.contains(Entity))
    {
        return *this;
    }
    meta_entities.add(Entity);
    return *this;
}

ArchetypeBuilder& ArchetypeBuilder::_access(sugoi_type_index_t type, intptr_t Field, EAccessMode FieldMode)
{
    fields.add(Field);
    field_types.add(type);
    field_modes.add(FieldMode);
    return *this;
}

void ArchetypeBuilder::commit() SKR_NOEXCEPT
{
    types.sort();
    meta_entities.sort();
}

void AccessBuilder::commit() SKR_NOEXCEPT
{
    all.sort([](auto a, auto b) { return a < b; });
    nones.sort([](auto a, auto b) { return a < b; });
}

sugoi_query_t* AccessBuilder::create_query(sugoi_storage_t* storage) SKR_NOEXCEPT
{
    sugoi_parameters_t parameters;
    parameters.types = types.data();
    parameters.accesses = ops.data();
    parameters.length = types.size();

    SKR_DECLARE_ZERO(sugoi_filter_t, filter);
    filter.all.data = all.data();
    filter.all.length = all.size();

    nones.sort([](auto a, auto b) { return a < b; });
    filter.none.data = nones.data();
    filter.none.length = nones.size();

    auto q = sugoiQ_create(storage, &filter, &parameters);
    if (q)
    {
        SKR_DECLARE_ZERO(sugoi_meta_filter_t, meta_filter);
        if (meta_entities.size())
        {
            meta_filter.all_meta.data = (const sugoi_entity_t*)meta_entities.data();
            meta_filter.all_meta.length = meta_entities.size();
        }
        /*
        if (none_meta.size())
        {
            meta_filter.none_meta.data = none_meta.data();
            meta_filter.none_meta.length = none_meta.size();
        }
        */
        sugoiQ_set_meta(q, &meta_filter);
    }
    return q;
}

AccessBuilder& AccessBuilder::_access(TypeIndex type, bool write, EAccessMode mode, intptr_t field)
{
    // fill: base type accessors
    if (write)
    {
        if (auto existed = writes.find_if([&](auto a) { return a.type == type; }))
        {
            existed.ref().mode |= mode;
        }
        else
        {
            writes.add({ type, mode });
        }
    }
    else
    {
        if (auto existed = reads.find_if([&](auto a) { return a.type == type; }))
        {
            existed.ref().mode |= mode;
        }
        else
        {
            reads.add({ type, mode });
        }
    }

    // fill: access builder args
    if ((field != kInvalidFieldPtr) && !fields.contains(field))
    {
        fields.add(field);
        field_types.add(type);
        field_modes.add(mode);
        fields_is_write.add(write);
    }
    return *this;
}

AccessBuilder& AccessBuilder::_add_has_filter(TypeIndex type, bool write)
{
    if (auto no = nones.find(type))
        SKR_ASSERT(0 && "Type already added as none!");

    if (auto existed = types.find(type))
    {
        ops[existed.index()].readonly &= !write;
    }
    else
    {
        types.add(type);
        all.add(type);
        ops.add({
            .phase = -1, .readonly = !write
            // seems these two attributes should be deprecated
            // .atomic = (mode == EAccessMode::Atomic),
            // .randomAccess = (mode == EAccessMode::Random) ? SOS_UNSEQ : SOS_SEQ
        });
    }
    return *this;
}

AccessBuilder& AccessBuilder::_add_none_filter(TypeIndex type)
{
    if (auto has = all.find(type))
        SKR_ASSERT(0 && "Type already added as all!");

    if (!types.find(type))
    {
        types.add(type);
        nones.add(type);
        ops.add({ .phase = -1, .readonly = true });
    }
    return *this;
}

static std::atomic_uint32_t kTaskSchedulerRefCount = 0;
static const auto kTSServiceDesc = ServiceThreadDesc{
    .name = u8"ECSTaskScheduler",
    .priority = SKR_THREAD_ABOVE_NORMAL,
};

ECSWorld::ECSWorld(skr::task::scheduler_t& scheduler) SKR_NOEXCEPT
{
    kTaskSchedulerRefCount += 1;
    TaskScheduler::Initialize(kTSServiceDesc, scheduler);
    storage = sugoiS_create();
}

ECSWorld::ECSWorld() SKR_NOEXCEPT
{
    storage = sugoiS_create();
}

void ECSWorld::bind_scheduler(skr::task::scheduler_t& scheduler) SKR_NOEXCEPT
{
    kTaskSchedulerRefCount += 1;
    TaskScheduler::Initialize(kTSServiceDesc, scheduler);
}

void ECSWorld::initialize() SKR_NOEXCEPT
{
    if (auto TS = TaskScheduler::Get())
    {
        TS->run();
    }
}

void ECSWorld::finalize() SKR_NOEXCEPT
{
    if (auto TS = TaskScheduler::Get())
    {
        kTaskSchedulerRefCount -= 1;
        if (kTaskSchedulerRefCount == 0)
        {
            TS->sync_all();
            TS->stop_and_exit();
            TaskScheduler::Finalize();
        }
    }

    sugoiS_release(storage);
    storage = nullptr;
}

uint32_t ECSWorld::GetComponentSize(skr::ecs::TypeIndex type) const
{
    auto desc = sugoiT_get_desc(type);
    return desc->size;
}

uint32_t ECSWorld::GetComponentAlign(skr::ecs::TypeIndex type) const
{
    auto desc = sugoiT_get_desc(type);
    return desc->alignment;
}

const char8_t* ECSWorld::GetComponentName(skr::ecs::TypeIndex type) const
{
    auto desc = sugoiT_get_desc(type);
    return desc->name;
}

sugoi_storage_t* ECSWorld::get_storage() SKR_NOEXCEPT
{
    return storage;
}

QueryBuilder::QueryBuilder(ECSWorld* world) SKR_NOEXCEPT
    : world(world)
{
}

QueryBuilderResult QueryBuilder::commit() SKR_NOEXCEPT
{
    sugoi_parameters_t parameters;
    parameters.types = types.data();
    parameters.accesses = ops.data();
    parameters.length = types.size();

    SKR_DECLARE_ZERO(sugoi_filter_t, filter);
    all.sort([](auto a, auto b) { return a < b; });
    filter.all.data = all.data();
    filter.all.length = all.size();

    none.sort([](auto a, auto b) { return a < b; });
    filter.none.data = none.data();
    filter.none.length = none.size();

    auto q = sugoiQ_create(world->get_storage(), &filter, &parameters);
    if (q)
    {
        SKR_DECLARE_ZERO(sugoi_meta_filter_t, meta_filter);
        if (all_meta.size())
        {
            meta_filter.all_meta.data = all_meta.data();
            meta_filter.all_meta.length = all_meta.size();
        }
        if (none_meta.size())
        {
            meta_filter.none_meta.data = none_meta.data();
            meta_filter.none_meta.length = none_meta.size();
        }
        sugoiQ_set_meta(q, &meta_filter);
    }
    return (EntityQuery*)q;
}

} // namespace skr::ecs

namespace skr
{
void Serialize<ecs::ECSWorld>::read(ArchiveRead& r, skr::ecs::ECSWorld& world)
{
    sugoiS_deserialize(world.storage, &r);
}
void Serialize<ecs::ECSWorld>::write(ArchiveWrite& w, const skr::ecs::ECSWorld& world)
{
    sugoiS_serialize(world.storage, &w);
}
} // namespace skr