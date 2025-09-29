#include "SkrActors/instanced_mesh_actor.hpp"
#include "SkrRenderer/render_mesh.h"
#include "SkrScene/basic_components.hpp"
#include "SkrRenderer/gpu_scene.h"
#include "SkrRenderer/shared/gpu_scene.hpp"

namespace skr
{

InstancedMeshActor::InstancedMeshActor()
    : Actor()
{
}

InstancedMeshActor::~InstancedMeshActor()
{
}

void InstancedMeshActor::CreateDefaultComponents(ecs::ArchetypeBuilder& builder)
{
    Super::CreateDefaultComponents(builder);
    builder
        .add_component<gpu::Instance>()
        .add_component<GPUSceneInstance>()
        .add_component<MeshComponent>();
}

int32_t InstancedMeshActor::GetInstanceCount() const
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        return (int32_t)per_instance_data->size();
    }
    return 0;
}

int32_t InstancedMeshActor::AddInstance(const skr::TransformF& transform)
{
    return AddInstance(transform.to_matrix());
}

int32_t InstancedMeshActor::AddInstance(const skr::float4x4& matrix)
{
    int32_t index = -1;
    if (auto per_instance_data = FindComponent<gpu::Instance>())
    {
        index = (int32_t)per_instance_data->size();
        per_instance_data->push_back({ matrix });
    }
    return index;
}

Vector<int32_t> InstancedMeshActor::AddInstances(const Vector<skr::TransformF>& transforms, bool return_indices)
{
    Vector<skr::float4x4> matrices;
    matrices.reserve(transforms.size());
    for (const auto& transform : transforms)
    {
        matrices.push_back(transform.to_matrix());
    }
    return AddInstances(matrices, return_indices);
}

Vector<int32_t> InstancedMeshActor::AddInstances(const Vector<skr::float4x4>& matrices, bool return_indices)
{
    Vector<int32_t> indices;
    if (matrices.is_empty())
    {
        return indices;
    }

    if (auto per_instance_data = GetPerInstanceData())
    {
        const auto num_to_add = matrices.size();
        auto index_start = per_instance_data->size();
        per_instance_data->resize_default(index_start + num_to_add);
        if (return_indices)
        {
            indices.reserve(num_to_add);
        }

        // Fill in the new instances
        Span sub_instance_data{ per_instance_data->data() + index_start, num_to_add };
        for (auto& matrix : matrices)
        {
            sub_instance_data[index_start] = { matrix };
            if (return_indices)
            {
                indices.push_back(uint32_t(index_start));
            }
            index_start++;
        }
    }

    return indices;
}

bool InstancedMeshActor::UpdateInstance(int32_t index, const skr::TransformF& transform)
{
    return UpdateInstance(index, transform.to_matrix());
}

bool InstancedMeshActor::UpdateInstance(int32_t index, const skr::float4x4& matrix)
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        // Update a single instance
        if (index >= 0 && index < (int32_t)per_instance_data->size())
        {
            per_instance_data->data()[index].transform = matrix;
            return true;
        }
    }
    return false;
}

bool InstancedMeshActor::UpdateInstances(int32_t start, int32_t count, const skr::TransformF& transform)
{
    return UpdateInstances(start, count, transform.to_matrix());
}

bool InstancedMeshActor::UpdateInstances(int32_t start, int32_t count, const skr::float4x4& matrix)
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        // Update a range of instances
        if (start >= 0 && start + count <= (int32_t)per_instance_data->size())
        {
            for (int32_t i = 0; i < count; i++)
            {
                per_instance_data->data()[start + i].transform = matrix;
            }
            return true;
        }
    }
    return false;
}

bool InstancedMeshActor::UpdateInstances(int32_t start, const Vector<skr::TransformF>& transforms)
{
    Vector<skr::float4x4> matrices;
    matrices.reserve(transforms.size());
    for (const auto& transform : transforms)
    {
        matrices.push_back(transform.to_matrix());
    }

    return UpdateInstances(start, matrices);
}

bool InstancedMeshActor::UpdateInstances(int32_t start, const Vector<skr::float4x4>& matrices)
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        // Update a range of instances
        if (start >= 0 && start + matrices.size() <= (int32_t)per_instance_data->size())
        {
            for (int32_t i = 0; i < matrices.size(); i++)
            {
                per_instance_data->data()[start + i].transform = matrices[i];
            }
            return true;
        }
    }
    return false;
}

bool InstancedMeshActor::RemoveInstance(int32_t index)
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        // Swap and pop for efficient removal
        if (index >= 0 && index < (int32_t)per_instance_data->size())
        {
            std::swap(per_instance_data->data()[index], per_instance_data->back());
            per_instance_data->pop_back();
            return true;
        }
    }
    return false;
}

bool InstancedMeshActor::RemoveInstances(const Vector<int32_t>& indices)
{
    if (indices.is_empty())
    {
        return true;
    }

    if (auto per_instance_data = GetPerInstanceData())
    {
        const auto size = static_cast<int32_t>(per_instance_data->size());

        // Check if all indices are valid
        if (indices.contains_if([size](int32_t idx) { return idx < 0 || idx >= size; }))
        {
            return false;
        }

        // Create unique set of indices to remove
        Set<int32_t> indices_to_remove(indices.data(), indices.size());

        PerInstanceData temp_data;
        temp_data.reserve(per_instance_data->size() - indices_to_remove.size());

        // Copy over instances that are not in the removal set
        int32_t index = 0;
        for (auto& instance : *per_instance_data)
        {
            if (indices_to_remove.contains(index++))
            {
                continue;
            }
            temp_data.push_back(std::move(instance));
        }
        std::swap(*per_instance_data, temp_data);
    }
    return true;
}

void InstancedMeshActor::ClearInstances()
{
    if (auto per_instance_data = GetPerInstanceData())
    {
        per_instance_data->clear();
    }
}

SKR_FORCEINLINE InstancedMeshActor::PerInstanceData* InstancedMeshActor::GetPerInstanceData() const noexcept
{
    return FindComponent<gpu::Instance>();
}

} // namespace skr