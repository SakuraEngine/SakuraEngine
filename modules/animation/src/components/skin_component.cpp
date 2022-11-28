#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"
#include "SkrAnim/ozz/geometry/skinning_job.h"
#include "SkrAnim/ozz/base/span.h"

void skr_initialize_skin_component(skr_skin_component_t* component, skr_skeleton_component_t* skelComponent)
{
    auto skin = component->skin.get_resolved();
    auto skeleton = skelComponent->skeleton.get_resolved();
    SKR_ASSERT(skin && skeleton);
    component->joint_remaps.resize(skeleton->skeleton.num_joints());
    for (size_t i = 0; i < skeleton->skeleton.num_joints(); ++i)
    {
        for (size_t j = 0; j < skin->blob.joint_remaps.size(); ++j)
        {
            if (std::strcmp(skeleton->skeleton.joint_names()[j], skin->blob.joint_remaps[i].data()) == 0)
            {
                component->joint_remaps[i] = j;
                break;
            }
        }
    }
}

void skr_initialize_anim_component(skr_anim_component_t* component, skr_mesh_resource_t* mesh, skr_skeleton_component_t* skeleton)
{
    component->primitive_buffers.resize(mesh->primitives.size());
    component->primitive_vbs.resize(mesh->primitives.size());
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        auto& prim = mesh->primitives[i];
        auto vertex_count = prim.index_buffer.index_count / 3;
        skr_vertex_buffer_entry_t *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;
        for (auto view : prim.vertex_buffers)
        {
            if (view.attribute == SKR_VERT_ATTRIB_JOINTS)
                joints_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_WEIGHTS)
                weights_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_POSITION)
                positions_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_NORMAL)
                normals_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_TANGENT)
                tangents_buffer = &view;
        }
        SKR_ASSERT(joints_buffer && weights_buffer);
        size_t buffer_size = 0, position_offset = 0, normal_offset = 0, tangent_offset = 0;
        position_offset = buffer_size;
        buffer_size += vertex_count * positions_buffer->stride;
        normal_offset = buffer_size;
        if (normals_buffer)
            buffer_size += vertex_count * normals_buffer->stride;
        tangent_offset = buffer_size;
        if (tangents_buffer)
            buffer_size += vertex_count * tangents_buffer->stride;
        component->buffer_size = buffer_size;
        component->position_offset = position_offset;
        component->normal_offset = normal_offset;
        component->tangent_offset = tangent_offset;
        component->primitive_buffers[i] = (uint8_t*)sakura_malloc_aligned(buffer_size, 16);
    }
}

void skr_cpu_skin(skr_skin_component_t* skin, skr_anim_component_t* anim, skr_mesh_resource_t* mesh)
{
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        ozz::geometry::SkinningJob job;
        auto& prim = mesh->primitives[i];
        auto vertex_count = prim.index_buffer.index_count / 3;
        skr_vertex_buffer_entry_t *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;
        for (auto view : prim.vertex_buffers)
        {
            if (view.attribute == SKR_VERT_ATTRIB_JOINTS)
                joints_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_WEIGHTS)
                weights_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_POSITION)
                positions_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_NORMAL)
                normals_buffer = &view;
            else if (view.attribute == SKR_VERT_ATTRIB_TANGENT)
                tangents_buffer = &view;
        }
        SKR_ASSERT(joints_buffer && weights_buffer);

        auto buffer_span = [&](skr_vertex_buffer_entry_t* buffer, auto t) {
            using T = typename decltype(t)::type;
            SKR_ASSERT(buffer->stride == sizeof(T));
            auto offset = mesh->bins[buffer->buffer_index].bin.bytes + buffer->offset;
            return ozz::span<const T>{ (T*)offset, vertex_count };
        };
        job.joint_matrices = { anim->skin_matrices.data(), anim->skin_matrices.size() };
        job.joint_weights = buffer_span(weights_buffer, skr::type_t<float>());
        job.joint_indices = { skin->joint_remaps.data(), skin->joint_remaps.size() };
        if (normals_buffer)
            job.in_normals = buffer_span(normals_buffer, skr::type_t<float>());
        if (tangents_buffer)
            job.in_tangents = buffer_span(tangents_buffer, skr::type_t<float>());
        job.in_positions = buffer_span(positions_buffer, skr::type_t<float>());
        auto buffer = anim->primitive_buffers[i];
        job.out_positions = { (float*)(buffer + anim->position_offset), vertex_count * 3 };
        if (normals_buffer)
            job.out_normals = { (float*)(buffer + anim->normal_offset), vertex_count * 3 };
        if (tangents_buffer)
            job.out_tangents = { (float*)(buffer + anim->tangent_offset), vertex_count * 3 };
        SKR_ASSERT(job.Run());
    }
}