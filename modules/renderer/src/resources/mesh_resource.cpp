#include "SkrRenderer/resources/mesh_resource.h"
#include "containers/sptr.hpp"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/guid.hpp"
#include "cgpu/cgpux.hpp"
#include "utils/io.h"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "platform/thread.h"
#include <platform/filesystem.hpp>
#include "containers/hashmap.hpp"
#include "SkrRenderer/render_mesh.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "SkrRenderer/render_device.h"
#include "utils/log.h"
#include "cgpu/io.h"

#include "tracy/Tracy.hpp"

skr_mesh_resource_t::~skr_mesh_resource_t() SKR_NOEXCEPT
{

}

static struct SkrMeshResourceUtil
{
    struct RegisteredVertexLayout : public CGPUVertexLayout
    {
        RegisteredVertexLayout(const CGPUVertexLayout& layout, skr_vertex_layout_id id, const char* name)
            : CGPUVertexLayout(layout), id(id), name(name)
        {
            hash = cgpux::hash<CGPUVertexLayout>()(layout);
        }
        skr_vertex_layout_id id;
        skr::string name;
        uint64_t hash;
    };

    using VertexLayoutIdMap = skr::flat_hash_map<skr_vertex_layout_id, skr::SPtr<RegisteredVertexLayout>, skr::guid::hash>;
    using VertexLayoutHashMap = skr::flat_hash_map<uint64_t, RegisteredVertexLayout*>;
    
    SkrMeshResourceUtil()
    {
        skr_init_mutex_recursive(&vertex_layouts_mutex_);
    }

    ~SkrMeshResourceUtil()
    {
        skr_destroy_mutex(&vertex_layouts_mutex_);
    }

    inline static skr_vertex_layout_id AddVertexLayout(skr_vertex_layout_id id, const char* name, const CGPUVertexLayout& layout)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto pLayout = skr::SPtr<RegisteredVertexLayout>::Create(layout, id, name);
        if (id_map.find(id) == id_map.end())
        {
            id_map.emplace(id, pLayout);
            hash_map.emplace((uint64_t)pLayout->hash, pLayout.get());
        }
        else
        {
            // id repeated
            SKR_UNREACHABLE_CODE();
        }
        return id;
    }

    inline static CGPUVertexLayout* HasVertexLayout(const CGPUVertexLayout& layout, skr_vertex_layout_id* outGuid = nullptr)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto hash = cgpux::hash<CGPUVertexLayout>()(layout);
        auto iter = hash_map.find(hash);
        if (iter == hash_map.end())
        {
            return nullptr;
        }
        if (outGuid) *outGuid = iter->second->id;
        return iter->second;
    }

    inline static const char* GetVertexLayoutName(CGPUVertexLayout* pLayout)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto hash = cgpux::hash<CGPUVertexLayout>()(*pLayout);
        auto iter = hash_map.find(hash);
        if (iter == hash_map.end())
        {
            return nullptr;
        }
        return iter->second->name.c_str();
    }

    inline static skr_vertex_layout_id GetVertexLayoutId(CGPUVertexLayout* pLayout)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto hash = cgpux::hash<CGPUVertexLayout>()(*pLayout);
        auto iter = hash_map.find(hash);
        if (iter == hash_map.end())
        {
            return {};
        }
        return iter->second->id;
    }

    inline static const char* GetVertexLayout(skr_vertex_layout_id id, CGPUVertexLayout* layout = nullptr)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto iter = id_map.find(id);
        if (iter == id_map.end()) return nullptr;
        if (layout) *layout = *iter->second;
        return iter->second->name.c_str();
    }

    static VertexLayoutIdMap id_map;
    static VertexLayoutHashMap hash_map;
    static SMutex vertex_layouts_mutex_;
} mesh_resource_util;
SkrMeshResourceUtil::VertexLayoutIdMap SkrMeshResourceUtil::id_map;
SkrMeshResourceUtil::VertexLayoutHashMap SkrMeshResourceUtil::hash_map;
SMutex SkrMeshResourceUtil::vertex_layouts_mutex_;

void skr_mesh_resource_free(skr_mesh_resource_id mesh_resource)
{
    SkrDelete(mesh_resource);
}

void skr_mesh_resource_register_vertex_layout(skr_vertex_layout_id id, const char* name, const struct CGPUVertexLayout* in_vertex_layout)
{
    if (auto layout = mesh_resource_util.GetVertexLayout(id))
    {
        return; // existed
    }
    else if (auto layout = mesh_resource_util.HasVertexLayout(*in_vertex_layout))
    {
        return; // existed
    }
    else // do register
    {
        auto result = mesh_resource_util.AddVertexLayout(id, name, *in_vertex_layout);
        SKR_ASSERT(result == id);
    }
}

const char* skr_mesh_resource_query_vertex_layout(skr_vertex_layout_id id, struct CGPUVertexLayout* out_vertex_layout)
{
    if (auto name = mesh_resource_util.GetVertexLayout(id, out_vertex_layout))
    {
        return name;
    }
    else
    {
        return nullptr;
    }
}

namespace skr
{
namespace resource
{
// 1.deserialize mesh resource
// 2.install indices/vertices to GPU
// 3?.update LOD information during runtime
struct SKR_RENDERER_API SMeshFactoryImpl : public SMeshFactory
{
    SMeshFactoryImpl(const SMeshFactory::Root& root)
        : root(root)
    {

    }

    ~SMeshFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;

    enum class EInstallMethod : uint32_t
    {
        DSTORAGE,
        UPLOAD,
        COUNT
    };

    enum class ECompressMethod : uint32_t
    {
        NONE,
        ZLIB,
        COUNT
    };

    struct InstallType
    {
        EInstallMethod install_method;
        ECompressMethod compress_method;
    };

    struct DStorageRequest
    {
        DStorageRequest() SKR_NOEXCEPT = default;
        ~DStorageRequest() SKR_NOEXCEPT = default;

        eastl::vector<std::string> absPaths;
        eastl::vector<skr_async_request_t> dRequests;
        eastl::vector<skr_async_vbuffer_destination_t> dDestinations;
    };

    struct UploadRequest
    {
        UploadRequest() SKR_NOEXCEPT = default;
        UploadRequest(SMeshFactoryImpl* factory,skr_mesh_resource_id mesh_resource) SKR_NOEXCEPT
            : factory(factory), mesh_resource(mesh_resource)
        {

        }
        ~UploadRequest() SKR_NOEXCEPT = default;
        
        SMeshFactoryImpl* factory = nullptr;
        skr_mesh_resource_id mesh_resource = nullptr;
        eastl::vector<std::string> resource_uris;
        eastl::vector<skr_async_request_t> ram_requests;
        eastl::vector<skr_async_ram_destination_t> ram_destinations;
        eastl::vector<skr_async_request_t> vram_requests;
        eastl::vector<skr_async_vbuffer_destination_t> buffer_destinations;
    };

    ESkrInstallStatus InstallWithDStorage(skr_resource_record_t* record);
    ESkrInstallStatus InstallWithUpload(skr_resource_record_t* record);

    Root root;
    skr::flat_hash_map<skr_mesh_resource_id, InstallType> mInstallTypes;
    skr::flat_hash_map<skr_mesh_resource_id, SPtr<UploadRequest>> mUploadRequests;
    skr::flat_hash_map<skr_mesh_resource_id, SPtr<DStorageRequest>> mDStorageRequests;
};

SMeshFactory* SMeshFactory::Create(const Root& root)
{
    return SkrNew<SMeshFactoryImpl>(root);
}

void SMeshFactory::Destroy(SMeshFactory* factory)
{
    SkrDelete(factory);
}

skr_type_id_t SMeshFactoryImpl::GetResourceType()
{
    const auto resource_type = skr::type::type_id<skr_mesh_resource_t>::get();
    return resource_type;
}

ESkrInstallStatus SMeshFactoryImpl::Install(skr_resource_record_t* record)
{
    auto mesh_resource = (skr_mesh_resource_t*)record->resource;
    if (!mesh_resource) return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
    if (auto render_device = root.render_device)
    {
        if (mesh_resource->install_to_vram)
        {
            // direct storage
            if (auto file_dstorage_queue = render_device->get_file_dstorage_queue() && !mesh_resource->install_to_ram)
            {
                return InstallWithDStorage(record);
            }
            else
            {
                return InstallWithUpload(record);
            }
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

ESkrInstallStatus SMeshFactoryImpl::InstallWithDStorage(skr_resource_record_t* record)
{
    const auto noCompression = true;
    auto mesh_resource = (skr_mesh_resource_t*)record->resource;
    auto guid = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        // direct storage
        if (auto file_dstorage_queue = render_device->get_file_dstorage_queue())
        {
            if (noCompression)
            {
                auto dRequest = SPtr<DStorageRequest>::Create();
                dRequest->absPaths.resize(mesh_resource->bins.size());
                dRequest->dRequests.resize(mesh_resource->bins.size());
                dRequest->dDestinations.resize(mesh_resource->bins.size());
                InstallType installType = {EInstallMethod::DSTORAGE, ECompressMethod::NONE};
                for (auto i = 0u; i < mesh_resource->bins.size(); i++)
                {
                    auto binPath = skr::format("{}.buffer{}", guid, i);
                    // TODO: REFACTOR THIS WITH VFS PATH
                    auto fullBinPath = skr::filesystem::path(root.dstorage_root) / binPath.c_str();
                    const auto& thisBin = mesh_resource->bins[i];
                    auto&& thisRequest = dRequest->dRequests[i];
                    auto&& thisDestination = dRequest->dDestinations[i];
                    auto&& thisPath = dRequest->absPaths[i];

                    thisPath = (const char*)fullBinPath.u8string().c_str();
                    auto vram_buffer_io = make_zeroed<skr_vram_buffer_io_t>();
                    vram_buffer_io.device = render_device->get_cgpu_device();

                    vram_buffer_io.dstorage.path = (const char8_t*)thisPath.c_str();
                    vram_buffer_io.dstorage.queue = file_dstorage_queue;
                    vram_buffer_io.dstorage.compression = CGPU_DSTORAGE_COMPRESSION_NONE;
                    vram_buffer_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_FILE;
                    vram_buffer_io.dstorage.uncompressed_size = thisBin.byte_length;

                    CGPUResourceTypes flags = CGPU_RESOURCE_TYPE_NONE;
                    flags |= thisBin.used_with_index ? CGPU_RESOURCE_TYPE_INDEX_BUFFER : 0;
                    flags |= thisBin.used_with_vertex ? CGPU_RESOURCE_TYPE_VERTEX_BUFFER : 0;
                    vram_buffer_io.vbuffer.resource_types = flags;
                    vram_buffer_io.vbuffer.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
                    vram_buffer_io.vbuffer.flags = CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION;
                    vram_buffer_io.vbuffer.buffer_size = thisBin.byte_length;
                    vram_buffer_io.vbuffer.buffer_name = nullptr; // TODO: set name

                    root.vram_service->request(&vram_buffer_io, &thisRequest, &thisDestination);
                }
                mDStorageRequests.emplace(mesh_resource, dRequest);
                mInstallTypes.emplace(mesh_resource, installType);
            }
            else
            {
                SKR_UNIMPLEMENTED_FUNCTION();
            }        
        }
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus SMeshFactoryImpl::InstallWithUpload(skr_resource_record_t* record)
{
    const auto noCompression = true;
    auto mesh_resource = (skr_mesh_resource_t*)record->resource;
    auto guid = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        if (noCompression)
        {
            auto uRequest = SPtr<UploadRequest>::Create(this, mesh_resource);
            uRequest->resource_uris.resize(mesh_resource->bins.size());
            uRequest->ram_requests.resize(mesh_resource->bins.size());
            uRequest->ram_destinations.resize(mesh_resource->bins.size());
            uRequest->vram_requests.resize(mesh_resource->bins.size());
            uRequest->buffer_destinations.resize(mesh_resource->bins.size());
            InstallType installType = {EInstallMethod::UPLOAD, ECompressMethod::NONE};
            auto found = mUploadRequests.find(mesh_resource);
            SKR_ASSERT(found == mUploadRequests.end());

            for (auto i = 0u; i < mesh_resource->bins.size(); i++)
            {
                auto binPath = skr::format("{}.buffer{}", guid, i);
                auto fullBinPath = skr::filesystem::path(root.dstorage_root) / binPath.c_str();
                auto&& ramRequest = uRequest->ram_requests[i];
                auto&& ramDestination = uRequest->ram_destinations[i];
                auto&& ramPath = uRequest->resource_uris[i];

                ramPath = fullBinPath.string();

                // emit ram requests
                auto ram_mesh_io = make_zeroed<skr_ram_io_t>();
                ram_mesh_io.path = (const char8_t*)binPath.c_str();
                ram_mesh_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
                    ZoneScopedN("Upload Mesh");
                    // upload
                    auto uRequest = (UploadRequest*)data;
                    const auto i = request - uRequest->ram_requests.data();
                    auto factory = uRequest->factory;
                    auto render_device = factory->root.render_device;
                    auto mesh_resource = uRequest->mesh_resource;

                    auto vram_buffer_io = make_zeroed<skr_vram_buffer_io_t>();
                    vram_buffer_io.device = render_device->get_cgpu_device();
                    vram_buffer_io.transfer_queue = render_device->get_cpy_queue();

                    auto& thisBin = mesh_resource->bins[i];
                    CGPUResourceTypes flags = CGPU_RESOURCE_TYPE_NONE;
                    flags |= thisBin.used_with_index ? CGPU_RESOURCE_TYPE_INDEX_BUFFER : 0;
                    flags |= thisBin.used_with_vertex ? CGPU_RESOURCE_TYPE_VERTEX_BUFFER : 0;
                    vram_buffer_io.vbuffer.resource_types = flags;
                    vram_buffer_io.vbuffer.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
                    vram_buffer_io.vbuffer.flags = CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION;
                    vram_buffer_io.vbuffer.buffer_size = thisBin.byte_length;
                    vram_buffer_io.vbuffer.buffer_name = nullptr; // TODO: set name
                    thisBin.bin.bytes = uRequest->ram_destinations[i].bytes;
                    thisBin.bin.size = uRequest->ram_destinations[i].size;

                    vram_buffer_io.src_memory.size = thisBin.byte_length;
                    vram_buffer_io.src_memory.bytes = thisBin.bin.bytes;
                    vram_buffer_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data){};
                    vram_buffer_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = nullptr;

                    factory->root.vram_service->request(&vram_buffer_io, &uRequest->vram_requests[i], &uRequest->buffer_destinations[i]);
                };
                ram_mesh_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)uRequest.get();
                root.ram_service->request(root.vfs, &ram_mesh_io, &ramRequest, &ramDestination);
            }
            mUploadRequests.emplace(mesh_resource, uRequest);
            mInstallTypes.emplace(mesh_resource, installType);
        }
        else
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus SMeshFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto mesh_resource = (skr_mesh_resource_t*)record->resource;
    auto installType = mInstallTypes[mesh_resource];
    if (installType.install_method == EInstallMethod::DSTORAGE)
    {
        auto dRequest = mDStorageRequests.find(mesh_resource);
        if (dRequest != mDStorageRequests.end())
        {
            bool okay = true;
            for (auto&& dRequest : dRequest->second->dRequests)
            {
                okay &= dRequest.is_ready();
            }
            auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
            if (okay)
            {
                auto render_mesh = mesh_resource->render_mesh = SkrNew<skr_render_mesh_t>();
                // TODO: remove these requests
                render_mesh->buffers.resize(dRequest->second->dDestinations.size());
                for (auto i = 0u; i < dRequest->second->dDestinations.size(); i++)
                {
                    render_mesh->buffers[i] = dRequest->second->dDestinations[i].buffer;
                }
                skr_render_mesh_initialize(render_mesh, mesh_resource);

                mDStorageRequests.erase(mesh_resource);
                mInstallTypes.erase(mesh_resource);
            }
            return status;
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
    else if (installType.install_method == EInstallMethod::UPLOAD)
    {
        auto uRequest = mUploadRequests.find(mesh_resource);
        if (uRequest != mUploadRequests.end())
        {
            bool okay = true;
            for (auto&& rRequest : uRequest->second->ram_requests)
            {
                okay &= rRequest.is_ready();
            }
            for (auto&& vRequest : uRequest->second->vram_requests)
            {
                okay &= vRequest.is_ready();
            }
            auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
            if (okay)
            {
                auto render_mesh = mesh_resource->render_mesh = SkrNew<skr_render_mesh_t>();
                // TODO: remove these requests
                render_mesh->buffers.resize(uRequest->second->buffer_destinations.size());
                for (auto i = 0u; i < uRequest->second->buffer_destinations.size(); i++)
                {
                    render_mesh->buffers[i] = uRequest->second->buffer_destinations[i].buffer;
                }
                skr_render_mesh_initialize(render_mesh, mesh_resource);
                if (!mesh_resource->install_to_ram)
                {
                    for (auto&& bin : mesh_resource->bins)
                    {
                        sakura_free(bin.bin.bytes);
                        bin.bin.bytes = nullptr;
                    }
                }
                mDStorageRequests.erase(mesh_resource);
                mInstallTypes.erase(mesh_resource);
            }
            return status;
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }

    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

bool SMeshFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto mesh_resource = (skr_mesh_resource_id)record->resource;
    SkrDelete(mesh_resource);
    return true; 
}

bool SMeshFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    auto mesh_resource = (skr_mesh_resource_id)record->resource;
    if (mesh_resource->install_to_ram)
    {
        for (auto&& bin : mesh_resource->bins)
        {
            sakura_free(bin.bin.bytes);
            bin.bin.bytes = nullptr;
        }
    }
    skr_render_mesh_free(mesh_resource->render_mesh);
    mesh_resource->render_mesh = nullptr;
    return true; 
}

} // namespace resource
} // namespace skr