#include "SkrProfile/profile.h"
#include "SkrGraphics/raytracing.h"
#include "SkrCore/id_range_allocator.hpp"
#include "SkrRuntime/io/vram_io.hpp"
#include "SkrRuntime/resource/resource_system.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/shared/gpu_scene.hpp"
#include "SkrRenderer/render_mesh.h"

namespace skr
{

// 1.deserialize mesh resource
// 2.install indices/vertices to GPU
// 3?.update LOD information during runtime
struct SKR_RENDERER_API MeshFactoryImpl : public MeshFactory
{
    MeshFactoryImpl(const MeshFactory::Root& root)
        : root(root)
    {
        dstorage_root = skr::String::From(root.dstorage_root);
        this->root.dstorage_root = dstorage_root.c_str();

        auto cgpu_device = root.render_device->get_cgpu_device();
        desc_buffer.count = 512;
        CGPUDescriptorBufferDescriptor bdls_desc = { .count = desc_buffer.count };
        desc_buffer.descriptor_buffer = cgpu_create_descriptor_buffer(cgpu_device, &bdls_desc);

        if (auto TableManager = root.table_manager)
        {
            gpu::TableConfig table_builder(root.render_device->get_cgpu_device(), u8"Primitives");
            table_builder.with_instances(16 * 1024);
            gpu::GPUDatablock<gpu::Primitive>::SetupTableConfig(table_builder);
            mPrimitiveTable = TableManager->CreateTable(table_builder);
            mPrimitiveIdRangeAllocator.resize(mPrimitiveTable->GetInstanceCapacity());
        }
    }

    ~MeshFactoryImpl() noexcept
    {
        cgpu_free_descriptor_buffer(desc_buffer.descriptor_buffer);
        mPrimitiveTable.reset();
    }

    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(SResourceRecord* record) override;
    ESkrInstallStatus Install(SResourceRecord* record) override;
    bool Uninstall(SResourceRecord* record) override;
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override;

    enum class ECompressMethod : uint32_t
    {
        NONE,
        ZLIB,
        COUNT
    };

    struct InstallType
    {
        ECompressMethod compress_method;
    };

    struct BufferRequest
    {
        BufferRequest() SKR_NOEXCEPT = default;
        ~BufferRequest() SKR_NOEXCEPT = default;

        skr::Vector<std::string> absPaths;
        skr::Vector<skr_io_future_t> dFutures;
        skr::Vector<skr::io::VRAMIOBufferId> dBuffers;
    };

    struct UploadRequest
    {
        UploadRequest() SKR_NOEXCEPT = default;
        UploadRequest(MeshFactoryImpl* factory, MeshResource* mesh_resource) SKR_NOEXCEPT
            : factory(factory)
            , mesh_resource(mesh_resource)
        {
        }
        ~UploadRequest() SKR_NOEXCEPT = default;

        MeshFactoryImpl* factory = nullptr;
        MeshResource* mesh_resource = nullptr;
        skr::Vector<std::string> resource_uris;
        skr::Vector<skr_io_future_t> ram_futures;
        skr::Vector<skr::BlobId> blobs;
        skr::Vector<skr_io_future_t> vram_futures;
        skr::Vector<skr::io::VRAMIOBufferId> uBuffers;
    };

    ESkrInstallStatus InstallImpl(SResourceRecord* record);

    void IniitializeRenderMesh(RenderMesh* render_mesh, MeshResource* mesh_resource);
    void FreeRenderMesh(RenderMesh* render_mesh);
    const bool UseRayTracing = true;

    Root root;
    skr::String dstorage_root;
    skr::FlatHashMap<MeshResource*, InstallType> mInstallTypes;
    skr::FlatHashMap<MeshResource*, SP<BufferRequest>> mRequests;

    skr::RC<gpu::TableInstance> primitive_table() override
    {
        return mPrimitiveTable;
    }

    CGPUDescriptorBufferId descriptor_buffer() override
    {
        return desc_buffer.descriptor_buffer;
    }

    skr::RG::BufferHandle UpdateGPUTable(skr::RG::RenderGraph* graph) override
    {
        auto handle = mPrimitiveTable->UpdateTableBuffer(graph, mPrimitiveIdRangeAllocator.getMaxIds());
        mPrimitiveTable->DispatchSparseUpload(graph, {});
        return handle;
    }

    skr::IdRangeAllocator mPrimitiveIdRangeAllocator;
    skr::RC<gpu::TableInstance> mPrimitiveTable;
    struct
    {
        CGPUDescriptorBufferId descriptor_buffer = nullptr;
        uint32_t count = 0;
        uint32_t next = 0;
        skr::Vector<uint32_t> free_list;
    } desc_buffer;
};

MeshFactory* MeshFactory::Create(const Root& root)
{
    auto cgpu_device = root.render_device->get_cgpu_device();
    return SkrNew<MeshFactoryImpl>(root);
}

void MeshFactory::Destroy(MeshFactory* factory)
{
    SkrDelete(factory);
}

GUID MeshFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::type_id_of<MeshResource>();
    return resource_type;
}

ESkrInstallStatus MeshFactoryImpl::Install(SResourceRecord* record)
{
    auto mesh_resource = (MeshResource*)record->resource;
    if (!mesh_resource) return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
    for (auto& mat : mesh_resource->materials)
    {
        mat.install();
    }
    if (auto render_device = root.render_device)
    {
        if (mesh_resource->install_to_vram)
        {
            return InstallImpl(record);
        }
        else
        {
            // TODO: install to RAM only
            SKR_UNIMPLEMENTED_FUNCTION();
        }
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

ESkrInstallStatus MeshFactoryImpl::InstallImpl(SResourceRecord* record)
{
    const auto noCompression = true;
    auto vram_service = root.vram_service;
    auto mesh_resource = (MeshResource*)record->resource;
    auto guid = record->activeRequest->GetGuid();

    if (auto render_device = root.render_device)
    {
        [[maybe_unused]] auto dsqueue = render_device->get_file_dstorage_queue();
        auto batch = vram_service->open_batch(mesh_resource->bins.size());
        if (noCompression)
        {
            auto dRequest = SP<BufferRequest>::New();
            dRequest->absPaths.resize_default(mesh_resource->bins.size());
            dRequest->dFutures.resize_zeroed(mesh_resource->bins.size());
            dRequest->dBuffers.resize_zeroed(mesh_resource->bins.size());
            InstallType installType = { ECompressMethod::NONE };
            for (auto i = 0u; i < mesh_resource->bins.size(); i++)
            {
                auto binPath = skr::format(u8"{}.buffer{}", guid, i);
                // TODO: REFACTOR THIS WITH VFS PATH
                // auto fullBinPath = skr::fs::path(root.dstorage_root) / binPath.c_str();
                // auto&& thisPath = dRequest->absPaths[i];
                const auto& thisBin = mesh_resource->bins[i];
                auto&& thisFuture = dRequest->dFutures[i];
                auto&& thisDestination = dRequest->dBuffers[i];

                CGPUBufferUsages usages = CGPU_BUFFER_USAGE_SHADER_READWRITE;
                usages |= thisBin.used_with_index ? CGPU_BUFFER_USAGE_INDEX_BUFFER : 0;
                usages |= thisBin.used_with_vertex ? CGPU_BUFFER_USAGE_VERTEX_BUFFER : 0;

                CGPUBufferDescriptor bdesc = {};
                bdesc.usages = usages;
                bdesc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
                bdesc.size = thisBin.byte_length;
                bdesc.name = thisBin.used_with_index ? thisBin.used_with_vertex ? u8"IB | VB" : u8"IB" : u8"VB";

                auto request = vram_service->open_buffer_request();
                if (auto blob = mesh_resource->bins[i].blob)
                {
                    request->set_memory_src(blob->get_data(), blob->get_size());
                }
                else
                {
                    request->set_vfs(root.vfs);
                    request->set_path(binPath.c_str());
                }
                request->set_buffer(render_device->get_cgpu_device(), &bdesc);
                request->set_transfer_queue(render_device->get_cpy_queue());
                if (mesh_resource->install_to_ram)
                {
                    auto blob = request->pin_staging_buffer();
                    mesh_resource->bins[i].blob = blob;
                }
                auto result = batch->add_request(request, &thisFuture);
                thisDestination = result.cast_static<skr::io::IVRAMIOBuffer>();
            }
            mRequests.emplace(mesh_resource, dRequest);
            mInstallTypes.emplace(mesh_resource, installType);
        }
        else
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
        vram_service->request(batch);
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus MeshFactoryImpl::UpdateInstall(SResourceRecord* record)
{
    auto mesh_resource = (MeshResource*)record->resource;
    auto dRequest = mRequests.find(mesh_resource);
    if (dRequest != mRequests.end())
    {
        bool okay = true;
        for (auto&& dRequest : dRequest->second->dFutures)
        {
            okay &= dRequest.is_ready();
        }
        auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            auto render_mesh = mesh_resource->render_mesh = SkrNew<RenderMesh>();
            // TODO: remove these requests
            const auto N = dRequest->second->dBuffers.size();
            render_mesh->buffers.resize_default(N);
            for (auto i = 0u; i < N; i++)
            {
                auto pBuffer = dRequest->second->dBuffers[i]->get_buffer();
                render_mesh->buffers[i] = pBuffer;
            }
            IniitializeRenderMesh(render_mesh, mesh_resource);

            mRequests.erase(mesh_resource);
            mInstallTypes.erase(mesh_resource);
        }
        return status;
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

void MeshFactoryImpl::IniitializeRenderMesh(RenderMesh* render_mesh, MeshResource* mesh_resource)
{
    uint32_t ibv_c = 0;
    uint32_t vbv_c = 0;
    // 1. calculate the number of index buffer views and vertex buffer views
    for (uint32_t i = 0; i < mesh_resource->sections.size(); i++)
    {
        const auto& section = mesh_resource->sections[i];
        for (auto prim_idx : section.primitive_indices)
        {
            // for (uint32_t prim_idx = 0; prim_idx < mesh_resource->primitives.size(); prim_idx++)
            // {
            auto& prim = mesh_resource->primitives[prim_idx];
            vbv_c += (uint32_t)prim.vertex_buffers.size();
            ibv_c++;
        }
    }

    // 2. do early reserve
    render_mesh->mesh_resource = mesh_resource;
    render_mesh->index_buffer_views.reserve(ibv_c);
    render_mesh->vertex_buffer_views.reserve(vbv_c);

    // 3. fill sections
    uint32_t geometry_count = 0;
    for (uint32_t i = 0; i < mesh_resource->sections.size(); i++)
    {
        const auto& section = mesh_resource->sections[i];
        for (auto prim_idx : section.primitive_indices)
        {
            const auto& prim = mesh_resource->primitives[prim_idx];
            SKR_ASSERT(render_mesh->index_buffer_views.capacity() >= render_mesh->index_buffer_views.size());
            SKR_ASSERT(render_mesh->vertex_buffer_views.capacity() >= render_mesh->vertex_buffer_views.size());
            auto& draw_cmd = render_mesh->primitive_commands.add_default().ref();
            auto& mesh_ibv = render_mesh->index_buffer_views.add_default().ref();
            auto vbv_start = render_mesh->vertex_buffer_views.size();
            // 3.1 fill vbvs
            for (uint32_t j = 0; j < prim.vertex_buffers.size(); j++)
            {
                const auto& prim_vb = prim.vertex_buffers[j];
                auto& mesh_vbv = render_mesh->vertex_buffer_views.add_default().ref();
                const auto buffer_index = prim_vb.buffer_index;
                mesh_vbv.buffer = render_mesh->buffers[buffer_index];
                mesh_vbv.offset = prim_vb.offset;
                mesh_vbv.vertex_count = prim_vb.vertex_count;
                mesh_vbv.stride = prim_vb.stride;
                mesh_vbv.primitive_index = prim_idx;
                mesh_vbv.index_in_prim = j;
                // SKR_LOG_INFO(u8"Mesh VBV %d: buffer %p, offset %d, stride %d", j, mesh_vbv.buffer, mesh_vbv.offset, mesh_vbv.stride);
            }
            // 3.2 fill ibv
            const auto buffer_index = prim.index_buffer.buffer_index;
            mesh_ibv.buffer = render_mesh->buffers[buffer_index];
            mesh_ibv.offset = prim.index_buffer.index_offset;
            mesh_ibv.stride = prim.index_buffer.stride;
            mesh_ibv.index_count = prim.index_buffer.index_count;
            mesh_ibv.first_index = prim.index_buffer.first_index;
            mesh_ibv.primitive_index = prim_idx;

            draw_cmd.ibv = &mesh_ibv;
            draw_cmd.vbvs = { render_mesh->vertex_buffer_views.data() + vbv_start, prim.vertex_buffers.size() };
            draw_cmd.primitive_index = prim_idx;
            draw_cmd.material_index = prim.material_index;

            geometry_count += 1;
        }
    }

    // 4. construct blas
    if (UseRayTracing && (mesh_resource->primitives.size() > 0))
    {
        skr::InlineVector<CGPUAccelerationStructureGeometryDesc, 4> geoms;
        for (const auto& section : mesh_resource->sections)
        {
            const auto section_transform = skr::TransformF(skr::QuatF(section.rotation), section.translation, section.scale);
            const auto transform = section_transform.to_matrix();
            float transform34[12] = {
                transform.m00, transform.m10, transform.m20, transform.m30, // Row 0: X axis + X translation
                transform.m01, transform.m11, transform.m21, transform.m31, // Row 1: Y axis + Y translation
                transform.m02, transform.m12, transform.m22, transform.m32  // Row 2: Z axis + Z translation
            };

            for (const auto& prim_id : section.primitive_indices)
            {
                const auto& primitive = mesh_resource->primitives[prim_id];

                if (auto pos_vb = primitive.vertex_buffers.find_if(
                                                              [](auto prim) { return prim.attribute == EVertexAttribute::POSITION; }
                    ).ptr())
                {
                    CGPUAccelerationStructureGeometryDesc geom = {};
                    geom.flags = CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
                    geom.vertex_buffer = render_mesh->buffers[pos_vb->buffer_index];
                    geom.vertex_offset = pos_vb->offset;
                    geom.vertex_count = primitive.vertex_count;
                    geom.vertex_stride = pos_vb->stride;
                    geom.vertex_format = CGPU_FORMAT_R32G32B32_SFLOAT;
                    geom.index_buffer = render_mesh->buffers[primitive.index_buffer.buffer_index];
                    geom.index_offset = primitive.index_buffer.index_offset;
                    geom.index_count = primitive.index_buffer.index_count;
                    // D3D12 expects index_stride to be 2 (uint16) or 4 (uint32)
                    // primitive.index_buffer.stride should already be 2 or 4, but let's ensure it
                    geom.index_stride = (primitive.index_buffer.stride == 2) ? sizeof(uint16_t) : sizeof(uint32_t);
                    memcpy(geom.transform, transform34, sizeof(transform34));
                    geoms.add(geom);
                }
            }
        }

        CGPUAccelerationStructureDescriptor blas_desc = {};
        blas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        blas_desc.bottom.geometries = geoms.data();
        blas_desc.bottom.count = geoms.size();
        render_mesh->blas = cgpu_create_acceleration_structure(geoms[0].vertex_buffer->device, &blas_desc);
    }

    // 5. add to bindless array
    {
        for (auto buffer : render_mesh->buffers)
        {
            uint32_t vbv_id = 0;
            if (!desc_buffer.free_list.is_empty())
                vbv_id = desc_buffer.free_list.pop_back_get();
            else
                vbv_id = desc_buffer.next++;
            render_mesh->vbuffer_ids.add(vbv_id);

            auto device = buffer->device;
            CGPUBufferViewDescriptor vbv_desc = {
                .name = u8"VertexBuffer",
                .buffer = buffer,
                .view_usages = CGPU_BUFFER_VIEW_USAGE_SRV_RAW,
                .offset = 0,
                .size = (uint32_t)buffer->info->size
            };
            CGPUDescriptorBufferElement vbv_elem = {};
            vbv_elem.index = vbv_id;
            vbv_elem.resource_type = CGPU_RESOURCE_TYPE2_BUFFER;
            vbv_elem.buffer = vbv_desc;
            cgpu_update_descriptor_buffer(desc_buffer.descriptor_buffer, &vbv_elem, 1);
        }
        for (auto buffer : render_mesh->buffers)
        {
            uint32_t ibv_id = 0;
            if (!desc_buffer.free_list.is_empty())
                ibv_id = desc_buffer.free_list.pop_back_get();
            else
                ibv_id = desc_buffer.next++;
            render_mesh->ibuffer_ids.add(ibv_id);

            ECGPUFormat texel_format = CGPU_FORMAT_R16_UINT;
            if (render_mesh->index_buffer_views[0].stride == 1)
                texel_format = CGPU_FORMAT_R8_UINT;
            if (render_mesh->index_buffer_views[0].stride == 2)
                texel_format = CGPU_FORMAT_R16_UINT;
            if (render_mesh->index_buffer_views[0].stride == 4)
                texel_format = CGPU_FORMAT_R32_UINT;

            CGPUBufferViewDescriptor ibv_desc = {
                .name = u8"IndexBuffer",
                .buffer = buffer,
                .view_usages = CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL,
                .offset = 0,
                .size = (uint32_t)buffer->info->size,
                .texel = {
                    .format = texel_format }
            };
            CGPUDescriptorBufferElement ibv_elem = {};
            ibv_elem.index = ibv_id;
            ibv_elem.resource_type = CGPU_RESOURCE_TYPE2_BUFFER;
            ibv_elem.buffer = ibv_desc;
            cgpu_update_descriptor_buffer(desc_buffer.descriptor_buffer, &ibv_elem, 1);
        }
    }

    // 6. add to primitives array
    {
        auto id_range = mPrimitiveIdRangeAllocator.allocate(geometry_count);
        if (id_range.empty())
        {
            auto neededCount = geometry_count + mPrimitiveIdRangeAllocator.getMaxIds();
            mPrimitiveIdRangeAllocator.resize(neededCount * 1.2);
            id_range = mPrimitiveIdRangeAllocator.allocate(geometry_count);
        }
        render_mesh->primitive_table_id_start = id_range.start;

        uint32_t next_geom = 0;
        for (const auto& section : mesh_resource->sections)
        {
            for (const auto& prim_id : section.primitive_indices)
            {
                const auto& prim_info = mesh_resource->primitives[prim_id];
                bool has_material = prim_info.material_index < mesh_resource->materials.size();
                if (!has_material) { continue; }

                gpu::Primitive prim_data;
                prim_data.global_index = id_range.start + next_geom;
                prim_data.material_index = prim_info.material_index;
                for (const auto& vb : prim_info.vertex_buffers)
                {
                    if (vb.vertex_count == 0)
                        continue;

                    const auto buffer_id = mesh_resource->render_mesh->vbuffer_ids[vb.buffer_index];
                    if (vb.attribute == EVertexAttribute::POSITION)
                        prim_data.positions = gpu::Range<float3>(0, vb.vertex_count, vb.offset, buffer_id);
                    else if (vb.attribute == EVertexAttribute::TEXCOORD)
                        prim_data.uvs = gpu::Range<float2>(0, vb.vertex_count, vb.offset, buffer_id);
                    else if (vb.attribute == EVertexAttribute::NORMAL)
                        prim_data.normals = gpu::Range<float3>(0, vb.vertex_count, vb.offset, buffer_id);
                    else if (vb.attribute == EVertexAttribute::TANGENT)
                        prim_data.tangents = gpu::Range<float3>(0, vb.vertex_count, vb.offset, buffer_id);
                }

                {
                    const auto& ib = prim_info.index_buffer;
                    const auto buffer_id = mesh_resource->render_mesh->ibuffer_ids[ib.buffer_index];
                    auto mod = ib.index_offset % (ib.stride * 3);
                    prim_data.indices = gpu::Range<uint32_t>(ib.index_offset / ib.stride, ib.index_count, 0, buffer_id);
                }
                gpu::GPUDatablock<gpu::Primitive>::StoreInstance(*mPrimitiveTable, prim_data.global_index, prim_data);
                next_geom += 1;
            }
        }
    }
}

void MeshFactoryImpl::FreeRenderMesh(RenderMesh* render_mesh)
{
    for (auto id : render_mesh->vbuffer_ids)
    {
        desc_buffer.free_list.add(id);
    }

    for (auto id : render_mesh->ibuffer_ids)
    {
        desc_buffer.free_list.add(id);
    }

    if (UseRayTracing && render_mesh->blas)
    {
        cgpu_free_acceleration_structure(render_mesh->blas);
    }

    for (auto&& buffer : render_mesh->buffers)
    {
        cgpu_free_buffer(buffer);
    }

    SkrDelete(render_mesh);
}

bool MeshFactoryImpl::Unload(SResourceRecord* record)
{
    auto mesh_resource = (MeshResource*)record->resource;
    SkrDelete(mesh_resource);
    return true;
}

bool MeshFactoryImpl::Uninstall(SResourceRecord* record)
{
    auto mesh_resource = (MeshResource*)record->resource;
    if (mesh_resource->install_to_ram)
    {
        mesh_resource->bins.clear();
    }
    FreeRenderMesh(mesh_resource->render_mesh);
    mPrimitiveIdRangeAllocator.deallocate(mesh_resource->render_mesh->primitive_table_id_start);
    mesh_resource->render_mesh = nullptr;
    return true;
}

} // namespace skr