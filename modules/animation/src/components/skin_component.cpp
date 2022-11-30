#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"
#include "SkrAnim/ozz/geometry/skinning_job.h"
#include "SkrAnim/ozz/base/span.h"

skr_render_anim_comp_t::~skr_render_anim_comp_t()
{
    for(auto vb : vbs)
    {
       if (vb) cgpu_free_buffer(vb);
    }
    for(auto buffer : buffers)
    {
        sakura_free_aligned(buffer.bytes, 16);
    }
}

void skr_init_skin_component(skr_render_skin_comp_t* component, skr_skeleton_resource_t* skeleton)
{
    auto skin = component->skin_resource.get_resolved();
    if(!skin)
        return;
    SKR_ASSERT(skeleton);
    component->joint_remaps.resize(skin->blob.joint_remaps.size());
    for (size_t i = 0; i < skin->blob.joint_remaps.size(); ++i)
    {
        for (size_t j = 0; j < skeleton->skeleton.num_joints(); ++j)
        {
            if (std::strcmp(skeleton->skeleton.joint_names()[j], skin->blob.joint_remaps[i].data()) == 0)
            {
                component->joint_remaps[i] = static_cast<uint32_t>(j);
                break;
            }
        }
    }
}

void skr_init_anim_component(skr_render_anim_comp_t* component, skr_mesh_resource_t* mesh, skr_skeleton_resource_t* skeleton)
{
    component->buffers.resize(1);
    component->vbs.resize(1);
    component->primitives.resize(mesh->primitives.size());
    component->joint_matrices.resize(skeleton->skeleton.num_joints());
    for (size_t i = 0; i < skeleton->skeleton.num_joints(); ++i)
        component->joint_matrices[i] = ozz::math::Float4x4::identity();
    size_t buffer_size = 0, position_offset = 0, normal_offset = 0, tangent_offset = 0;
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        auto& prim = mesh->primitives[i];
        auto vertex_count = prim.vertex_count;
        skr_vertex_buffer_entry_t *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;
        for (auto& view : prim.vertex_buffers)
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
        position_offset = buffer_size;
        buffer_size += vertex_count * positions_buffer->stride;
        normal_offset = buffer_size;
        if (normals_buffer)
            buffer_size += vertex_count * normals_buffer->stride;
        tangent_offset = buffer_size;
        if (tangents_buffer)
            buffer_size += vertex_count * tangents_buffer->stride;
        auto& primitive = component->primitives[i];
        {
            primitive.position.buffer_index = 0;
            primitive.position.attribute = positions_buffer->attribute;
            primitive.position.attribute_index = positions_buffer->attribute_index;
            primitive.position.offset = static_cast<uint32_t>(position_offset);
            primitive.position.stride = positions_buffer->stride;
        }
        if(normals_buffer)
        {
            primitive.normal.buffer_index = 0;
            primitive.normal.attribute = normals_buffer->attribute;
            primitive.normal.attribute_index = normals_buffer->attribute_index;
            primitive.normal.offset = static_cast<uint32_t>(normal_offset);
            primitive.normal.stride = normals_buffer->stride;
        }
        if(tangents_buffer)
        {
            primitive.tangent.buffer_index = 0;
            primitive.tangent.attribute = tangents_buffer->attribute;
            primitive.tangent.attribute_index = tangents_buffer->attribute_index;
            primitive.tangent.offset = static_cast<uint32_t>(tangent_offset);
            primitive.tangent.stride = tangents_buffer->stride;
        }
    }
    component->buffers[0].bytes = (uint8_t*)sakura_malloc_aligned(buffer_size, 16);
    component->buffers[0].size = buffer_size;
}

void skr_cpu_skin(skr_render_skin_comp_t* skin, skr_render_anim_comp_t* anim, skr_mesh_resource_t* mesh)
{
    auto skin_resource = skin->skin_resource.get_resolved();
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        ozz::geometry::SkinningJob job;
        auto& prim = mesh->primitives[i];
        auto vertex_count = prim.vertex_count;
        skr_vertex_buffer_entry_t *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;
        for (auto& view : prim.vertex_buffers)
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

        auto buffer_span = [&](skr_vertex_buffer_entry_t* buffer, auto t, uint32_t comps = 1) {
            using T = typename decltype(t)::type;
            SKR_ASSERT(buffer->stride == sizeof(T) * comps);
            auto offset = mesh->bins[buffer->buffer_index].bin.bytes + buffer->offset;
            return ozz::span<const T>{ (T*)offset, vertex_count * comps };
        };
        skin->skin_matrices.resize(anim->joint_matrices.size());
        for(size_t i = 0; i < skin->joint_remaps.size(); ++i)
        {
            auto inverse = skin_resource->blob.inverse_bind_poses[i];
            skin->skin_matrices[i] = anim->joint_matrices[skin->joint_remaps[i]] * (ozz::math::Float4x4&)inverse;
        }
        job.joint_matrices = { skin->skin_matrices.data(), skin->skin_matrices.size() };
        job.influences_count = 4;
        job.vertex_count = vertex_count;
        job.joint_weights = buffer_span(weights_buffer, skr::type_t<float>(), 4);
        job.joint_weights_stride = weights_buffer->stride;
        job.joint_indices = buffer_span(joints_buffer, skr::type_t<uint16_t>(), 4);
        job.joint_indices_stride = joints_buffer->stride;
        if (normals_buffer)
            job.in_normals = buffer_span(normals_buffer, skr::type_t<float>(), 3);
        job.in_normals_stride = normals_buffer->stride;
        if (tangents_buffer)
            job.in_tangents = buffer_span(tangents_buffer, skr::type_t<float>(), 4);
        job.in_tangents_stride = tangents_buffer->stride;
        job.in_positions = buffer_span(positions_buffer, skr::type_t<float>(), 3);
        job.in_positions_stride = positions_buffer->stride;
        auto& skprim = anim->primitives[i];
        job.out_positions = { (float*)(anim->buffers[skprim.position.buffer_index].bytes + skprim.position.offset), vertex_count * 3 };
        job.out_positions_stride = skprim.position.stride;
        if (normals_buffer)
            job.out_normals = { (float*)(anim->buffers[skprim.normal.buffer_index].bytes + skprim.normal.offset), vertex_count * 3 };
        job.out_normals_stride = skprim.normal.stride;
        if (tangents_buffer)
            job.out_tangents = { (float*)(anim->buffers[skprim.tangent.buffer_index].bytes + skprim.tangent.offset), vertex_count * 4 };
        job.out_tangents_stride = skprim.tangent.stride;
        auto result = job.Run();
        SKR_ASSERT(result);
    }
}