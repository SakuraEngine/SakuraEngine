#pragma once
#include "SkrScene/actor.hpp"
#include "SkrActors/instanced_mesh_actor.generated.h"

namespace skr
{
namespace gpu
{
struct Instance;
}

struct [[sattr(
    guid = "f669eed4-a3d6-4a70-b332-cc9f66487f51";
    rttr = @enable;
)]] SKR_ACTORS_API InstancedMeshActor : public Actor
{
    SKR_GENERATE_BODY(InstancedMeshActor)

    InstancedMeshActor();
    ~InstancedMeshActor() override;
    void CreateDefaultComponents(ecs::ArchetypeBuilder& builder) override;

    /** Get the number of instances in this actor. */
    int32_t GetInstanceCount() const;

    /**
     * @brief  Adds instance(s) to this actor.
     *         Returns the index of the newly added instance.
     * @note   The transform is in local space relative to the parent actor.
     */
    virtual int32_t AddInstance(const skr::TransformF& transform);
    virtual int32_t AddInstance(const skr::float4x4& matrix);
    virtual Vector<int32_t> AddInstances(const Vector<skr::TransformF>& transforms, bool return_indices = false);
    virtual Vector<int32_t> AddInstances(const Vector<skr::float4x4>& matrices, bool return_indices = false);

    /**
     * @brief  Updates instance(s) in this actor.
     *         Returns true if the instance was updated successfully.
     * @note   The transform is in local space relative to the parent actor.
     */
    virtual bool UpdateInstance(int32_t index, const skr::TransformF& transform);
    virtual bool UpdateInstance(int32_t index, const skr::float4x4& matrix);
    virtual bool UpdateInstances(int32_t start, int32_t count, const skr::TransformF& transform);
    virtual bool UpdateInstances(int32_t start, int32_t count, const skr::float4x4& matrix);
    virtual bool UpdateInstances(int32_t start, const Vector<skr::TransformF>& transforms);
    virtual bool UpdateInstances(int32_t start, const Vector<skr::float4x4>& matrices);

    /**
     * @brief  Removes instance(s) from this actor.
     *         Returns true if the instance was removed successfully.
     */
    virtual bool RemoveInstance(int32_t index);
    virtual bool RemoveInstances(const Vector<int32_t>& indices);
    virtual void ClearInstances();

private:
    using PerInstanceData = sugoi::ArrayComponent<gpu::Instance, 1>;
    PerInstanceData* GetPerInstanceData() const noexcept;
    struct InstanceCmdBuffer;
    InstanceCmdBuffer* _cmd_buffer = nullptr;
};

} // namespace skr