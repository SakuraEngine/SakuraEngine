#pragma once
#include "SkrRuntime/ecs/world.hpp"
#include "SkrBase/atomic/atomic_mutex.hpp"

namespace skr::ecs
{

using SpinLock = skr::shared_atomic_mutex;
inline static constexpr auto kSugoiStagingBufferSize = 4096;

struct SKR_RUNTIME_API StagingWorld 
{
public:
    void initialize(uint32_t EntryCount = 65535) SKR_NOEXCEPT;
    void finalize() SKR_NOEXCEPT;
    void apply(World& World) SKR_NOEXCEPT;

    void remove_entity(uint32_t DeadEntity)
    {
        int32_t Index = skr_atomic_fetch_add_acquire(&DeadEntityCount, 1);
        DeadEntities[Index] = DeadEntity;
    }

    uint32_t add_entity(uint32_t Prefab = sugoi::kEntityNull, uint32_t Count = 1)
    {
        int32_t Index = skr_atomic_fetch_add_acquire(&SpawnEntityCount, 1);
        SpawnInfos[Index].Prefab = Prefab;
        skr_atomic_fetch_add_acquire(&TotalSpawnCount, Count);
        return sugoi::e_make_transient(Index);
    }

    template <typename T>
    void add_tag(sugoi_query_t* q)
    {
        using Type = typename std::remove_reference_t<T>;
        uint32_t Index = skr_atomic_fetch_add_acquire(&GlobalAddAlterCount, 1);
        GlobalAddAlters[Index].q = q;
        GlobalAddAlters[Index].type = sugoi_id_of<Type>();
    }

    template <class T>
    void remove_component(sugoi_query_t* q)
    {
        using Type = typename std::remove_reference_t<T>;
        uint32_t Index = skr_atomic_fetch_add_acquire(&GlobalRemoveAlterCount, 1);
        GlobalRemoveAlters[Index].q = q;
        GlobalRemoveAlters[Index].type = sugoi_id_of<Type>();
    }

    template <class T, class InitFunction, class OverrideFunction>
    void add_component(uint32_t Entity, InitFunction&& Init, OverrideFunction&& Override)
    {
        using Type = typename std::remove_reference_t<T>;
        using StorageType = typename ComponentStorage<Type>::Type;

        const uint32_t Index = SUGOI_ENTITY_ID(Entity);
        const auto TypeIndex = sugoi_id_of<Type>();
        uint32_t Row = Type2TableRow[TypeIndex];
        auto Size = sizeof(StorageType);
        bool IsTransientEntity = sugoi::e_transient(Entity);
        SKR_ASSERT(!IsTransientEntity || sugoi::e_id(Entity) < (uint32_t)SpawnEntityCount);
        auto& Entry = IsTransientEntity ? SpawnEntries[Index] : AlterEntries[Index];

        Entry.Lock.lock();
        SKR_DEFER({ Entry.Lock.unlock(); });

        uint32_t Column = Entry.Index;
        if (Column == -1)
        {
            Column = Entry.Index = skr_atomic_fetch_add_acquire(&AlterTableColumnCount, 1);
        }
        auto& Info = AlterInfos[Column];
        AlterEntities[Column] = Entity;
        Info.RemoveComponent[Row] = false;
        if (std::is_empty_v<Type>) // Is tag component
        {
            Info.AddComponent[Row] = true;
        }
        else
        {
            uint8_t* Data = AlterTable[Row].Data + Column * Size;
            if (AlterTable[Row].ContainsReference)
            {
                ContainReference = true;
                AlterInfos[Column].ContainsReference = true;
            }
            if (!Info.AddComponent[Row])
            {
                Info.AddComponent[Row] = true;
                Init(*(StorageType*)Data);
            }
            else
            {
                Override(*(StorageType*)Data);
            }
        }
    }

    template <class T>
    void add_component(uint32_t Entity, T&& Value)
    {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        const auto TypeIndex = sugoi_id_of<Type>();
        constexpr auto kArrayCount = sugoi_array_count<Type>;
        add_component<Type>(Entity, [&](auto& Comp) {
    		if constexpr (kArrayCount > 0)
    		{
                using TArr = sugoi::ArrayComponent<Type, sugoi_id_of<Type>::kArrayCount>;
				auto Arr = new (&Comp) TArr{};
				Arr->emplace_back(std::forward<T>(Value));
            } else {
				new (&Comp) Type{std::forward<T>(Value)};
			} 
        }, [&](auto& Comp) {
    		if constexpr (kArrayCount > 0)
    		{
                using TArr = sugoi::ArrayComponent<Type, sugoi_id_of<Type>::kArrayCount>;
                auto Arr = (TArr*)&Comp;
                Arr->emplace_back(std::forward<T>(Value));
			} else {
				Comp = (std::forward<T>(Value));
			} 
        });
    }

    template<class T>
    void remove_component(uint32_t Entity)
    {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        using StorageType = typename ComponentStorage<Type>::Type;

        uint32_t Index = SUGOI_ENTITY_ID(Entity);
        const auto TypeIndex = sugoi_id_of<Type>();
        uint32_t Row = Type2TableRow[TypeIndex];
		auto Size = sizeof(StorageType);
    	bool IsTransientEntity = sugoi::e_transient(Entity);
    	auto& Entry = IsTransientEntity ? SpawnEntries[Index] : AlterEntries[Index];

        Entry.Lock.lock();
        SKR_DEFER({ Entry.Lock.unlock(); });

        uint32_t Column = Entry.Index;
        if (Column == -1)
        {
            Column = Entry.Index = skr_atomic_fetch_add_acquire(&AlterTableColumnCount, 1);
        }
        auto& Info = AlterInfos[Column];
        AlterEntities[Column] = Entity;
        if (std::is_empty_v<Type>) // Is tag component
        {
            Info.AddComponent[Row] = false;
        }
        else
        {
        	if(Info.AddComponent[Row])
        	{
        		uint8_t* Data = AlterTable[Row].Data + Column * Size;
				((StorageType*)Data)->~StorageType();
        	}
        }
        Info.RemoveComponent[Row] = true;
    }

private:
    uint32_t DeadEntities[kSugoiStagingBufferSize];
    SAtomic32 DeadEntityCount;

    // Component modification tracking
    struct FAlterTableEntry {
        uint32_t Index = -1;
        SpinLock Lock;
    };
    skr::Vector<FAlterTableEntry> AlterEntries;
    FAlterTableEntry SpawnEntries[kSugoiStagingBufferSize];
    struct FSpawnInfo {
        uint32_t Prefab = sugoi::kEntityNull;
    };
    FSpawnInfo SpawnInfos[kSugoiStagingBufferSize];
    SAtomic32 SpawnEntityCount;
    SAtomic32 TotalSpawnCount;
    struct FAlterTableRow {
        sugoi_type_index_t type;
        bool IsBuffer;
        uint32_t Size;
        bool ContainsReference;
        uint8_t* Data;
    };
    skr::Vector<FAlterTableRow> AlterTable;
    struct FAlterInfo {
        bool ContainsReference;
        skr::Bitset<128> AddComponent;
        skr::Bitset<128> RemoveComponent;
    };
    uint32_t AlterEntities[kSugoiStagingBufferSize];
    FAlterInfo AlterInfos[kSugoiStagingBufferSize];
    SAtomic32 AlterTableColumnCount;
    skr::Map<sugoi_type_index_t, uint32_t> Type2TableRow;
    bool ContainReference = false;

    struct FGlobalAlter {
        sugoi_query_t* q;
        sugoi_type_index_t type;
    };
    FGlobalAlter GlobalAddAlters[kSugoiStagingBufferSize];
    SAtomic32 GlobalAddAlterCount;
    FGlobalAlter GlobalRemoveAlters[kSugoiStagingBufferSize];
    SAtomic32 GlobalRemoveAlterCount;
};

} // namespace skr::ecs