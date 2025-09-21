#pragma once
#include "SkrBase/types/impl/guid.hpp"
#include "SkrContainers/string.hpp"
#include "SkrCore/memory/rc.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrRuntime/ecs/component.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrScene/scene.h"
#include "SkrScene/actor.generated.h"

namespace skr
{
struct RootActor;

enum class [[sattr(
    guid = "a1ebd9b1-900c-44f4-b381-0dd48014718d" 
    serde = @enable
)]] EAttachRule
{
    Default = 0,
    KeepWorldTransform = 0x01
};

struct [[sattr(
    guid = "4cb20865-0d27-43ee-90b9-7b43ac4c067c";
    rttr = @enable;
    serde = @enable
)]] SKR_SCENE_API Actor
{
    friend class ActorManager;

public:
    SKR_GENERATE_BODY()
    SKR_RC_IMPL();

    Actor() SKR_NOEXCEPT {}
    virtual ~Actor() SKR_NOEXCEPT;
    static RCWeak<RootActor> GetRoot();
    void BindWorld(skr::ecs::ECSWorld* world) { this->world = world; }
    void CreateEntity();
    skr::ecs::Entity GetEntity() const;
    void AttachTo(RCWeak<Actor> parent, EAttachRule rule = EAttachRule::Default);
    void DetachFromParent();
    void DetachAllChildren();

    struct Spawner
    {
        using BuildF = std::function<void(skr::ecs::ArchetypeBuilder&)>;
        using RunF = std::function<void(skr::ecs::TaskContext&)>;
        Spawner(BuildF build_func, RunF func)
            : build_func(build_func)
            , f(func)
        {
        }

        void build(skr::ecs::ArchetypeBuilder& Builder)
        {
            build_func(Builder);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("Spawner");
            f(Context);
        }
        BuildF build_func;
        RunF f;
    };

    [[sattr(serde = @disable)]]
    skr::UPtr<Spawner> spawner;

    virtual void Initialize();                // Init Blank
    virtual void Initialize(skr_guid_t guid); // Init with GUID

protected:
    skr::String display_name; // for editor, profiler, and runtime dump
    skr::GUID guid;
    skr::GUID rttr_type_guid;

    EAttachRule attach_rule = EAttachRule::Default;
    bool bIsInitialized = false;

    [[sattr(serde = @disable)]]
    skr::InlineVector<skr::ecs::Entity, 1> scene_entities;
    [[sattr(serde = @disable)]]
    skr::Vector<skr::RC<Actor>> children;
    [[sattr(serde = @disable)]]
    skr::RC<Actor> _parent = nullptr;
    [[sattr(serde = @disable)]]
    skr::ecs::ECSWorld* world = nullptr; // Pointer to the ECS world for actor management

    skr::SerializeConstVector<skr_guid_t> children_serialized;
    skr_guid_t parent_serialized = skr_guid_t{};
    skr::SerializeConstVector<sugoi_entity_t> scene_entities_serialized;

public:
    void serialize() SKR_NOEXCEPT;
    void deserialize() SKR_NOEXCEPT;
    inline const skr::String GetDisplayName() const { return display_name; }
    inline void SetDisplayName(const skr::String& name) { display_name = name; }
    template <typename ComponentType>
    ComponentType* GetComponent() const
    {
        auto entity = GetEntity();
        if (entity != skr::ecs::Entity{ SUGOI_NULL_ENTITY })
        {
            return world->random_readwrite<ComponentType>().get(entity);
        }
        SKR_LOG_ERROR(u8"Actor {%s} has no valid entity to get Component", display_name.c_str());
        return nullptr;
    }

    skr::GUID GetGUID() const SKR_NOEXCEPT { return guid; }
    skr::GUID GetRTTRTypeGUID() const SKR_NOEXCEPT { return rttr_type_guid; }

    skr::Vector<skr::GUID> GetChildrenGUIDs() const SKR_NOEXCEPT;
};

// Actor that Carry ECS World Instance, and ActorRootComponent
struct [[sattr(
    guid = "01990a69-0bbf-7483-bbbf-36ab1580e83a";
    rttr = @enable
)]] SKR_SCENE_API RootActor : public Actor
{
public:
    RootActor() {}
    ~RootActor() SKR_NOEXCEPT override;
    skr::ecs::ECSWorld* GetWorld() const { return root_world.get(); }
    void bind_scheduler(skr::task::scheduler_t& scheduler) { root_world->bind_scheduler(scheduler); }

    skr::UPtr<skr::ecs::ECSWorld> root_world = nullptr;
    void Initialize() override;
    void Initialize(skr_guid_t guid) override;
    void InitWorld();
};

struct [[sattr(
    guid = "01987a21-a2b4-7488-924d-17639e937f87";
    rttr = @enable;
)]] SKR_SCENE_API MeshActor : public Actor
{
public:
    MeshActor() SKR_NOEXCEPT {}
    ~MeshActor() SKR_NOEXCEPT override;
    void Initialize() override;
    void Initialize(skr_guid_t guid) override;
};

struct [[sattr(
    guid = "01987a21-e796-76b6-89c4-fb550edf5610";
    rttr = @enable;
)]] SKR_SCENE_API SkelMeshActor : public MeshActor
{
public:
    SkelMeshActor() SKR_NOEXCEPT {}
    ~SkelMeshActor() SKR_NOEXCEPT override;
    void Initialize() override;
    void Initialize(skr_guid_t guid) override;
};

} // namespace skr