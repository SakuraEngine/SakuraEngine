#pragma once
#include "SkrRuntime/sugoi/sugoi.h" // IWYU pragma: export
#include "SkrRuntime/sugoi/sugoi_meta.hpp" // IWYU pragma: export
#include "SkrRuntime/sugoi/array.hpp" // IWYU pragma: export
#include "SkrRuntime/sugoi/storage.hpp" // IWYU pragma: export
#include "SkrRuntime/sugoi/chunk.hpp" // IWYU pragma: export
#include "SkrRuntime/sugoi/archetype.hpp" // IWYU pragma: export
#include "SkrRuntime/sugoi/array.hpp" // IWYU pragma: export

namespace skr::ecs {

struct ECSWorld;
using TypeIndex = sugoi_type_index_t;

template <typename T, typename = void>
struct ComponentStorage
{
    using Type = T;
};

template <typename T>
struct ComponentStorage<T, std::enable_if_t<(sugoi_array_count<std::decay_t<T>> > 0)>>
{
    using Type = sugoi::ArrayComponent<std::decay_t<T>, sugoi_array_count<std::decay_t<T>>>;
};

struct Entity
{
public:
    explicit Entity(sugoi_entity_t InEntityId = sugoi::kEntityNull);
    bool operator==(Entity Other) const;
    bool operator==(sugoi_entity_t Other) const;
    explicit operator sugoi_entity_t() const;
    explicit operator bool() const;
    bool operator<(Entity Other) const;
    skr::String to_string() const;
    static skr_hash _skr_hash(const Entity& Handle);

private:
    sugoi_entity_t EntityId = sugoi::kEntityNull;
};
static_assert(sizeof(Entity) == sizeof(sugoi_entity_t), "Entity size must match sugoi_entity_t size");

template <typename T>
concept EntityConcept = std::is_same_v<T, Entity>;

struct ComponentViewBase
{
protected:
    ComponentViewBase(void* ptr = nullptr, uint32_t local_type = 0, uint32_t offset = 0)
        : _ptr(ptr), _local_type(local_type), _offset(offset) {}
    friend struct ECSWorld;
    friend struct TaskContext;
    void* _ptr = nullptr;
    uint32_t _local_type = 0;
    uint32_t _offset = 0;
};

template <typename T = void>
struct ComponentView : public ComponentViewBase
{
public:
    using Storage = typename ComponentStorage<T>::Type;
    Storage& operator[](int32_t Index)
    {
        return ((Storage*)_ptr)[Index + _offset];
    }

    explicit operator bool() const
    {
        return _ptr != nullptr;
    }
    
    const Storage* at(uint32_t Index) const
    {
        return _ptr ? ((Storage*)_ptr) + Index + _offset : nullptr;
    }

    ComponentView()
        : ComponentViewBase(nullptr, 0) {}

protected:
    friend struct ECSWorld;
    friend struct TaskContext;
    ComponentView(T* ptr, uint32_t local_type, uint32_t offset)
        : ComponentViewBase(ptr, local_type, offset) {}
};

template <>
struct ComponentView<const void> : public ComponentViewBase
{
public:
    explicit operator bool() const
    {
        return _ptr != nullptr;
    }

    const uint8_t* at(uint32_t i)
    {
        return _ptr ? (const uint8_t*)_ptr + (_offset + i) * _size : nullptr;
    }
    
    ComponentView()
        : ComponentViewBase(nullptr, 0) {}

protected:
    friend struct ECSWorld;
    friend struct TaskContext;
    ComponentView(const void* ptr, uint32_t local_type, uint32_t offset, uint32_t size)
        : ComponentViewBase((void*)ptr, local_type, offset), _size(size) {}
    uint32_t _size = 0;
};

template <>
struct ComponentView<void> : public ComponentViewBase
{
public:
    explicit operator bool() const
    {
        return _ptr != nullptr;
    }

    uint8_t* at(uint32_t i)
    {
        return (uint8_t*)_ptr + (_offset + i) * _size;
    }
    
    ComponentView()
        : ComponentViewBase(nullptr, 0) {}

protected:
    friend struct ECSWorld;
    friend struct TaskContext;
    ComponentView(void* ptr, uint32_t local_type, uint32_t offset, uint32_t size)
        : ComponentViewBase(ptr, local_type, offset), _size(size) {}
    uint32_t _size = 0;
};

struct SubQuery
{
public:
    bool is_valid() const;
    bool match(sugoi_entity_t Entity) const;
    uint32_t Count() const;
    SKR_RUNTIME_API Entity get_singleton() const;

private:
    sugoi_query_t* query;
};

struct ComponentAccessorBase
{
protected:
    template <class Storage>
    SKR_FORCEINLINE Storage* get(Entity entity)
    {
        sugoi_chunk_view_t view = World->entity_view((sugoi_entity_t)entity);
        if (view.chunk == nullptr)
            return nullptr;
        if(CachedPtr != nullptr && CachedView.chunk == view.chunk)
        {
            auto Offset = (int64_t)view.start - (int64_t)CachedView.start;
            return ((Storage*)CachedPtr) + Offset;
        }
        Storage* Result;
        if constexpr (std::is_const_v<Storage>)
            Result = (Storage*)sugoiV_get_owned_ro(&view, Type);
        else
            Result = (Storage*)sugoiV_get_owned_rw(&view, Type);
        CachedView = view;
        CachedPtr = (void*)Result;
        return Result;
    }

    template <class Storage>
    Storage& get_checked(Entity entity)
    {
        Storage* Result = get<Storage>(entity);
        SKR_ASSERT(Result);
        return *Result;
    }

    friend struct ECSWorld;
    sugoi_storage_t* World;
    TypeIndex Type;
    sugoi_chunk_view_t CachedView;
    void* CachedPtr = nullptr;
};

template <class T = void>
struct RandomComponentReader : public ComponentAccessorBase
{
    using Storage = typename ComponentStorage<T>::Type;
    const Storage& operator[](Entity entity)
    {
        return get_checked<Storage>(entity);
    }
    const Storage* get(Entity entity) { return ComponentAccessorBase::get<Storage>(entity); }
};

template <>
struct RandomComponentReader<void> : public ComponentAccessorBase
{
    template <typename T>
    const T* get(Entity entity) {  return ComponentAccessorBase::get<T>(entity); }
};

template <class T>
struct RandomComponentWriter : public ComponentAccessorBase
{
    using Storage = typename ComponentStorage<T>::Type;
    void write_at(Entity entity, const Storage& value)
    {
        auto& v = get_checked<Storage>(entity);
        v = value;
    }
};

template <class T>
struct RandomComponentReadWrite : public ComponentAccessorBase
{
    using Storage = typename ComponentStorage<T>::Type;
    Storage& operator[](Entity entity)
    {
        return get_checked<Storage>(entity);
    }

    void write_at(Entity entity, const Storage& value)
    {
        auto& v = get_checked<Storage>(entity);
        v = value;
    }
    
    Storage* get(Entity entity) { return ComponentAccessorBase::get<Storage>(entity); }
};

} // namespace skr::ecs

// inline implementations
namespace skr::ecs {

inline Entity::Entity(sugoi_entity_t InEntityId)
    : EntityId(InEntityId)
{

}

inline bool Entity::operator==(Entity Other) const
{
    return EntityId == Other.EntityId;
}

inline bool Entity::operator==(sugoi_entity_t Other) const
{
    return EntityId == Other;
}

inline Entity::operator sugoi_entity_t() const
{
    return EntityId;
}

inline Entity::operator bool() const
{
    return EntityId != sugoi::kEntityNull;
}

inline bool Entity::operator<(Entity Other) const
{
    return EntityId < Other.EntityId;
}

inline skr_hash Entity::_skr_hash(const Entity& Handle)
{
    return Handle.EntityId;
}

inline skr::String Entity::to_string() const
{
    return skr::format(u8"[{}/{}]", sugoi::e_id(EntityId), sugoi::e_version(EntityId));
}

inline bool SubQuery::is_valid() const
{
    return query != nullptr;
}

inline bool SubQuery::match(sugoi_entity_t Entity) const
{
    return sugoiQ_match_entity(query, Entity) != 0;
}

inline uint32_t SubQuery::Count() const
{
    return sugoiQ_get_count(query);
}

} // namespace skr::ecs
