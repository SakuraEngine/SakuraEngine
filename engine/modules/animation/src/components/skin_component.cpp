
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRenderer/render_mesh.h"

#include "SkrAnim/components/skin_component.hpp"
#include "SkrAnim/components/skeleton_component.hpp"
#include "SkrAnim/ozz/geometry/skinning_job.h"
#include "SkrAnim/ozz/base/span.h"

#include "SkrProfile/profile.h"

using namespace skr;

skr::AnimComponent::~AnimComponent()
{
    for (auto vb : vbs)
    {
        if (vb) cgpu_free_buffer(vb);
    }
    buffers.clear();
}

void skr_init_skin_component(skr::SkinComponent* component, const skr::SkeletonResource* skeleton)
{
    auto skin = component->skin_resource.get_resolved();
    if (!skin)
        return;
    SKR_ASSERT(skeleton);
    component->joint_remaps.resize_zeroed(skin->joint_remaps.size());
    for (size_t i = 0; i < skin->joint_remaps.size(); ++i)
    {
        for (size_t j = 0; j < skeleton->skeleton.num_joints(); ++j)
        {
            auto remap = (const char*)skin->joint_remaps[i].data();
            if (strcmp(skeleton->skeleton.joint_names()[j], remap) == 0)
            {
                component->joint_remaps[i] = static_cast<uint32_t>(j);
                break;
            }
        }
    }
}

void skr_init_anim_component(skr::AnimComponent* anim, const MeshResource* mesh, skr::SkeletonResource* skeleton)
{
    anim->buffers.resize_zeroed(1); // CPU buffer
    anim->vbs.resize_zeroed(1);     // GPU buffer
    anim->primitives.resize_default(mesh->primitives.size());
    anim->joint_matrices.resize_zeroed(skeleton->skeleton.num_joints());

    for (size_t i = 0; i < skeleton->skeleton.num_joints(); ++i)
        anim->joint_matrices[i] = ozz::math::Float4x4::identity();
    size_t buffer_size = 0, position_offset = 0, normal_offset = 0, tangent_offset = 0;

    // sync view info
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        const auto& prim = mesh->primitives[i];
        auto vertex_count = prim.vertex_count;
        const VertexBufferEntry *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;
        for (auto& view : prim.vertex_buffers)
        {
            if (view.attribute == EVertexAttribute::JOINTS)
                joints_buffer = &view;
            else if (view.attribute == EVertexAttribute::WEIGHTS)
                weights_buffer = &view;
            else if (view.attribute == EVertexAttribute::POSITION)
                positions_buffer = &view;
            else if (view.attribute == EVertexAttribute::NORMAL)
                normals_buffer = &view;
            else if (view.attribute == EVertexAttribute::TANGENT)
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

        auto& primitive = anim->primitives[i];
        {
            primitive.position.buffer_index = 0;
            primitive.position.attribute = positions_buffer->attribute;
            primitive.position.attribute_index = positions_buffer->attribute_index;
            primitive.position.offset = static_cast<uint32_t>(position_offset);
            primitive.position.stride = positions_buffer->stride;
        }
        if (normals_buffer)
        {
            primitive.normal.buffer_index = 0;
            primitive.normal.attribute = normals_buffer->attribute;
            primitive.normal.attribute_index = normals_buffer->attribute_index;
            primitive.normal.offset = static_cast<uint32_t>(normal_offset);
            primitive.normal.stride = normals_buffer->stride;
        }
        if (tangents_buffer)
        {
            primitive.tangent.buffer_index = 0;
            primitive.tangent.attribute = tangents_buffer->attribute;
            primitive.tangent.attribute_index = tangents_buffer->attribute_index;
            primitive.tangent.offset = static_cast<uint32_t>(tangent_offset);
            primitive.tangent.stride = tangents_buffer->stride;
        }
    }

    auto blob = skr::IBlob::CreateAligned(nullptr, buffer_size, 16, false); // the cpu swap buffer for skinning
    anim->buffers[0] = blob.get();
}

void skr_init_anim_buffers(CGPUDeviceId device, skr::AnimComponent* anim, const MeshResource* mesh)
{
    auto mesh_resource = mesh;
    for (size_t j = 0u; j < anim->buffers.size(); j++)
    {
        const bool use_dynamic_buffer = anim->use_dynamic_buffer;

        if (!anim->vbs[j])
        {
            SkrZoneScopedN("CreateVB");

            auto vb_desc = make_zeroed<CGPUBufferDescriptor>();
            vb_desc.name = (const char8_t*)mesh_resource->name.c_str(); // TODO: buffer name
            vb_desc.usages = CGPU_BUFFER_USAGE_VERTEX_BUFFER;
            vb_desc.flags = anim->use_dynamic_buffer ? CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT : CGPU_BUFFER_FLAG_NONE;
            vb_desc.memory_usage = anim->use_dynamic_buffer ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY;
            vb_desc.prefer_on_device = true;
            vb_desc.size = anim->buffers[j]->get_size();
            SKR_ASSERT(vb_desc.size > 0);
            anim->vbs[j] = cgpu_create_buffer(device, &vb_desc);

            auto renderMesh = mesh_resource->render_mesh;

            anim->views.reserve(renderMesh->vertex_buffer_views.size());

            for (size_t k = 0; k < anim->primitives.size(); ++k)
            {
                auto& prim = anim->primitives[k];
                auto vbv_start = anim->views.size();

                for (size_t z = 0; z < renderMesh->primitive_commands[k].vbvs.size(); ++z)
                {
                    auto& vbv = renderMesh->primitive_commands[k].vbvs[z];
                    auto attr = mesh_resource->primitives[k].vertex_buffers[z].attribute;
                    // anim->views.add(vbv);

                    if (attr == EVertexAttribute::POSITION)
                    {
                        auto& view = anim->views.add_default().ref();
                        view.buffer = anim->vbs[j];
                        view.offset = prim.position.offset;
                        view.stride = prim.position.stride;
                    }
                    else if (attr == EVertexAttribute::NORMAL)
                    {
                        auto& view = anim->views.add_default().ref();
                        view.buffer = anim->vbs[j];
                        view.offset = prim.normal.offset;
                        view.stride = prim.normal.stride;
                    }
                    else if (attr == EVertexAttribute::TANGENT)
                    {
                        auto& view = anim->views.add_default().ref();
                        view.buffer = anim->vbs[j];
                        view.offset = prim.tangent.offset;
                        view.stride = prim.tangent.stride;
                    }
                    else
                        anim->views.add(vbv);
                }
                prim.views = skr::Span<skr_vertex_buffer_view_t>(anim->views.data() + vbv_start, renderMesh->primitive_commands[k].vbvs.size());
                // SKR_LOG_INFO(u8"Anim Primitive %d has %d vertex buffers", k, prim.views.size());
            }
        }
        const auto vertex_size = anim->buffers[j]->get_size();

        if (use_dynamic_buffer)
        {
            SkrZoneScopedN("CVVUpdateVB");

            void* vtx_dst = anim->vbs[j]->info->cpu_mapped_address;
            memcpy(vtx_dst, anim->buffers[j]->get_data(), vertex_size);
        }
    }
}

void skr_cpu_skin(skr::SkinComponent* skin, const skr::AnimComponent* anim, const MeshResource* mesh)
{
    auto skin_resource = skin->skin_resource.get_resolved();
    // SKR_LOG_INFO(u8"Skin %d mesh primitive(s)", mesh->primitives.size());
    for (size_t i = 0; i < mesh->primitives.size(); ++i)
    {
        ozz::geometry::SkinningJob job;
        auto& prim = mesh->primitives[i];
        auto vertex_count = prim.vertex_count;
        const VertexBufferEntry *joints_buffer = nullptr, *weights_buffer = nullptr, *positions_buffer = nullptr, *normals_buffer = nullptr, *tangents_buffer = nullptr;

        for (auto& view : prim.vertex_buffers)
        {
            if (view.attribute == EVertexAttribute::JOINTS)
                joints_buffer = &view;
            else if (view.attribute == EVertexAttribute::WEIGHTS)
                weights_buffer = &view;
            else if (view.attribute == EVertexAttribute::POSITION)
                positions_buffer = &view;
            else if (view.attribute == EVertexAttribute::NORMAL)
                normals_buffer = &view;
            else if (view.attribute == EVertexAttribute::TANGENT)
                tangents_buffer = &view;
        }
        SKR_ASSERT(joints_buffer && weights_buffer);

        auto buffer_span = [&](const VertexBufferEntry* buffer, auto t, uint32_t comps = 1) {
            using T = typename decltype(t)::type;
            SKR_ASSERT(buffer->stride == sizeof(T) * comps);
            auto bin = mesh->bins[buffer->buffer_index];
            auto offset = mesh->bins[buffer->buffer_index].blob->get_data() + buffer->offset;
            return ozz::span<const T>{ (T*)offset, vertex_count * comps };
        };

        skin->skin_matrices.resize_zeroed(anim->joint_matrices.size());

        for (size_t i = 0; i < skin->joint_remaps.size(); ++i)
        {
            auto inverse = skin_resource->inverse_bind_poses[i];
            skin->skin_matrices[i] = anim->joint_matrices[skin->joint_remaps[i]] * (ozz::math::Float4x4&)inverse;
        }

        auto& skprim = anim->primitives[i];
        {
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
            if (tangents_buffer && tangents_buffer->stride)
                job.in_tangents = buffer_span(tangents_buffer, skr::type_t<float>(), 4);
            job.in_tangents_stride = tangents_buffer->stride;
            job.in_positions = buffer_span(positions_buffer, skr::type_t<float>(), 3);
            job.in_positions_stride = positions_buffer->stride;

            job.out_positions = { (float*)(anim->buffers[skprim.position.buffer_index]->get_data() + skprim.position.offset), vertex_count * 3 };
            job.out_positions_stride = skprim.position.stride;

            if (normals_buffer)
                job.out_normals = { (float*)(anim->buffers[skprim.normal.buffer_index]->get_data() + skprim.normal.offset), vertex_count * 3 };
            job.out_normals_stride = skprim.normal.stride;
            if (tangents_buffer)
                job.out_tangents = { (float*)(anim->buffers[skprim.tangent.buffer_index]->get_data() + skprim.tangent.offset), vertex_count * 4 };

            job.out_tangents_stride = skprim.tangent.stride;
            auto result = job.Run();
            SKR_ASSERT(result);
        }

        // auto* skin_buf = (float*)(anim->buffers[0]->get_data() + skprim.position.offset);
        // auto* mesh_buf = buffer_span(positions_buffer, skr::type_t<float>(), 3).data();
        // memcpy(skin_buf, mesh_buf, 3 * vertex_count * sizeof(float));
    }
}
