#pragma once
#include "SkrBase/types.h"
#include "SkrContainers/vector.hpp"
#include "SkrRTTR/irttr_basic.hpp"
#include "SkrRuntime/ecs/component.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include <SkrScene/basic_components.hpp>
#include "SkrScene/actor.generated.h"

//! 目前的 Actor 模型
//! - 与 UE 的 Actor 不同，Skr 的 Actor 是一种 Node 结构，逻辑的组合功能由 ECS 组件提供
//! - 因此 Skr 的 Actor 中，组件就是 ECS 的组件，Actor 本身被视为 ECS 的操作层
//! 
//! Actor 的生命周期
//! - 构造：我们通常不在构造中做任何事情，请尽量保证构造实现为空
//! - OnInitialize：用于完成一些 Actor 的创建工作
//! - CreateDefaultComponents：用于创建与 Actor 关联的 ECS 组件
//! - CreateDefaultEntity：用于对 ECS 中的 Entity 做一些基础的配置工作
//! - 反序列化（可选）：如果 Actor 来自于序列化数据，则会在此时进行反序列化
//! - BeginPlay：此时 Actor 将正式执行其逻辑，用于完成一些前置工作
//! - Tick（可选）：如果 Actor 需要每帧更新，则会在此时接收 Tick 信息
//! - EndPlay：用于进行一些立即的清理工作
//! - BeginDestroy：在销毁 Actor 之前，进行一些释放工作
//! 
//! Actor 的 Transform
//! - 目前没有提供 WorldTransform 相关的接口，这类操作通常会产生精度损失以及大量开销，如果需要这个功能后续再加
//! - 通常通过操作 GetLocalXXX / SetLocalXXX 来完成局部空间的操作
//! - GetWorldTransform 用于获取解算完的世界空间 Transform，它的值在 World 完成 TransformUpdate 时候才会更新
//! - 尽量不要使用 GetWorldTransform，因为更新是迟滞的

// fwd
namespace skr
{
namespace ecs
{
struct ECSWorld;
}
struct TransformSystem;
struct Actor;
struct World;
} // namespace skr

namespace skr
{
namespace concepts
{
template <typename T>
concept ActorSafeFindComponent =
    // hierarchy components, use Attach / Detach instead
    !std::is_same_v<T, ParentComponent> &&
    !std::is_same_v<T, ChildrenComponent> &&
    // transform components, use SetLocalXXX / GetLocalXXX instead
    !std::is_same_v<T, PositionComponent> &&
    !std::is_same_v<T, RotationComponent> &&
    !std::is_same_v<T, ScaleComponent> &&
    !std::is_same_v<T, SolvedTransformComponent>;
}

struct [[sattr(
    guid = "4cb20865-0d27-43ee-90b9-7b43ac4c067c";
    rttr = @enable;
    serde = @enable
)]] SKR_SCENE_API Actor : virtual skr::IRTTRBasic
{
    friend struct World;
    SKR_GENERATE_BODY()

    // ctor & dtor
    Actor();
    virtual ~Actor();

    // getters
    skr::GUID GetGUID() const;
    Actor* GetParent() const;
    const skr::Vector<Actor*>& GetChildren() const;
    skr::ecs::Entity GetDefaultEntity() const;

    // world
    ecs::ECSWorld* GetECSWorld() const;
    World* GetWorld() const;

    // transform management
    void SetLocalPosition(const Position& position);
    void SetLocalRotation(const Rotator& rotation);
    void SetLocalScale(const Scale& scale);
    Position GetLocalPosition() const;
    Rotator GetLocalRotation() const;
    Scale GetLocalScale() const;
    Transform GetWorldTransform() const;

    // compent management
    template <concepts::ActorSafeFindComponent T>
    auto FindComponent() const -> typename ecs::ComponentStorage<T>::Type*;
    template <typename T>
    auto FindComponentUnsafe() const -> typename ecs::ComponentStorage<T>::Type*;

    // hierarchy
    void Attach(Actor* parent);
    void Detach();

    // destroy
    void Destroy();

protected:
    //===========init call backs===========
    // 时机：构造
    // 用于完成一些 Actor 的创建工作，通常情况下不会使用
    virtual void OnInitialize();
    // 时机：OnInitialize 之后
    // 用于创建与 Actor 关联的 ECS 组件
    virtual void CreateDefaultComponents(ecs::ArchetypeBuilder& builder);
    // 时机：CreateDefaultComponents 之后
    // 用于对 ECS 中的 Entity 做一些基础的配置工作
    virtual void CreateDefaultEntity(ecs::TaskContext& ctx);
    // 时机：构造 -> 反序列化（可选）
    // 此时 Actor 将正式执行其逻辑，用于完成一些前置工作
    virtual void BeginPlay();
    // 时机：BeginPlay 之后, EndPlay 之后
    // 用于指示该 Actor 是否应该被添加到 Tick 列表中，由于绝大多数对象不具备 Tick 行为
    // 因此默认为 true 以节约性能，如果想接收 Tick 信息，请务必设为 false
    virtual bool IsNeverTick() const;
    // 时机：每帧
    // 用于接收每帧的 Tick 信息
    virtual void OnTick(double delta_time);
    // 时机：Destroy 被调用时
    // 用于进行一些清理工作
    virtual void EndPlay();
    // 时机：World 的释放时点
    // 用于进行一些最终的清理工作
    virtual void BeginDestroy();
    // TODO. OnAttach / OnDetach
    //===========init call backs===========

private:
    // called by world
    void _Initialize(skr::GUID guid, World* world);
    void _DestroyEntity();

private:
    // guid
    skr::GUID _guid;

    // hierarchy
    [[sattr(serde = @disable)]]
    Actor* _parent = nullptr;
    [[sattr(serde = @disable)]]
    skr::Vector<Actor*> _children = {};

    // world
    [[sattr(serde = @disable)]]
    skr::World* _world = nullptr;

    // ecs data
    [[sattr(serde = @disable)]]
    skr::ecs::Entity _default_entity = {};
};
} // namespace skr

// impl actor
namespace skr
{
// getters
inline skr::GUID Actor::GetGUID() const { return _guid; }
inline Actor* Actor::GetParent() const { return _parent; }
inline const skr::Vector<Actor*>& Actor::GetChildren() const { return _children; }
inline skr::ecs::Entity Actor::GetDefaultEntity() const { return _default_entity; }

// compent management
template <concepts::ActorSafeFindComponent T>
inline auto Actor::FindComponent() const -> typename ecs::ComponentStorage<T>::Type*
{
    return FindComponentUnsafe<T>();
}
template <typename T>
inline auto Actor::FindComponentUnsafe() const -> typename ecs::ComponentStorage<T>::Type*
{
    auto entity = GetDefaultEntity();
    if (entity != skr::ecs::Entity{ SUGOI_NULL_ENTITY })
    {
        return GetECSWorld()->random_readwrite<T>().get(entity);
    }
    return nullptr;
}
} // namespace skr