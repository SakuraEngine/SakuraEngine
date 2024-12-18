#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrCore/memory/memory.h"
#include "SkrCore/platform/vfs.h"
#include "SkrGraphics/cgpux.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrOS/thread.h"
#include <SkrOS/filesystem.hpp>
#include "SkrRenderer/render_mesh.h"
#include "SkrRT/resource/resource_factory.h"
#include "SkrRT/resource/resource_system.h"
#include "SkrRenderer/render_device.h"

#include "SkrContainers/sptr.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/hashmap.hpp"

#include "SkrProfile/profile.h"

static struct SkrMeshResourceUtil {
    struct RegisteredVertexLayout : public CGPUVertexLayout {
        RegisteredVertexLayout(const CGPUVertexLayout& layout, skr_vertex_layout_id id, const char8_t* name)
            : CGPUVertexLayout(layout)
            , id(id)
            , name(name)
        {
            hash = cgpux::hash<CGPUVertexLayout>()(layout);
        }
        skr_vertex_layout_id id;
        skr::String          name;
        uint64_t             hash;
    };

    using VertexLayoutIdMap   = skr::FlatHashMap<skr_vertex_layout_id, skr::SPtr<RegisteredVertexLayout>, skr::Hash<skr_guid_t>>;
    using VertexLayoutHashMap = skr::FlatHashMap<uint64_t, RegisteredVertexLayout*>;

    SkrMeshResourceUtil()
    {
        skr_init_mutex_recursive(&vertex_layouts_mutex_);
    }

    ~SkrMeshResourceUtil()
    {
        skr_destroy_mutex(&vertex_layouts_mutex_);
    }

    inline static skr_vertex_layout_id AddVertexLayout(skr_vertex_layout_id id, const char8_t* name, const CGPUVertexLayout& layout)
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
        return iter->second->name.c_str_raw();
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
        return iter->second->name.c_str_raw();
    }

    static VertexLayoutIdMap   id_map;
    static VertexLayoutHashMap hash_map;
    static SMutex              vertex_layouts_mutex_;
} mesh_resource_util;
SkrMeshResourceUtil::VertexLayoutIdMap   SkrMeshResourceUtil::id_map;
SkrMeshResourceUtil::VertexLayoutHashMap SkrMeshResourceUtil::hash_map;
SMutex                                   SkrMeshResourceUtil::vertex_layouts_mutex_;

void skr_mesh_resource_free(skr_mesh_resource_id mesh_resource)
{
    SkrDelete(mesh_resource);
}

void skr_mesh_resource_register_vertex_layout(skr_vertex_layout_id id, const char8_t* name, const struct CGPUVertexLayout* in_vertex_layout)
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
namespace renderer
{
using namespace skr::resource;

MeshResource::~MeshResource() SKR_NOEXCEPT
{
}

// 1.deserialize mesh resource
// 2.install indices/vertices to GPU
// 3?.update LOD information during runtime
struct SKR_RENDERER_API SMeshFactoryImpl : public SMeshFactory {
    SMeshFactoryImpl(const SMeshFactory::Root& root)
        : root(root)
    {
        dstorage_root            = skr::String::From(root.dstorage_root);
        this->root.dstorage_root = dstorage_root.c_str();
    }

    ~SMeshFactoryImpl() noexcept = default;
    skr_guid_t        GetResourceType() override;
    bool              AsyncIO() override { return true; }
    bool              Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool              Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;

    enum class ECompressMethod : uint32_t
    {
        NONE,
        ZLIB,
        COUNT
    };

    struct InstallType {
        ECompressMethod compress_method;
    };

    struct BufferRequest {
        BufferRequest() SKR_NOEXCEPT  = default;
        ~BufferRequest() SKR_NOEXCEPT = default;

        skr::Vector<std::string>             absPaths;
        skr::Vector<skr_io_future_t>         dFutures;
        skr::Vector<skr::io::VRAMIOBufferId> dBuffers;
    };

    struct UploadRequest {
        UploadRequest() SKR_NOEXCEPT = default;
        UploadRequest(SMeshFactoryImpl* factory, skr_mesh_resource_id mesh_resource) SKR_NOEXCEPT
            : factory(factory),
              mesh_resource(mesh_resource)
        {
        }
        ~UploadRequest() SKR_NOEXCEPT = default;

        SMeshFactoryImpl*                    factory       = nullptr;
        skr_mesh_resource_id                 mesh_resource = nullptr;
        skr::Vector<std::string>             resource_uris;
        skr::Vector<skr_io_future_t>         ram_futures;
        skr::Vector<skr::BlobId>             blobs;
        skr::Vector<skr_io_future_t>         vram_futures;
        skr::Vector<skr::io::VRAMIOBufferId> uBuffers;
    };

    ESkrInstallStatus InstallImpl(skr_resource_record_t* record);

    skr::String                                                 dstorage_root;
    Root                                                        root;
    skr::FlatHashMap<skr_mesh_resource_id, InstallType>         mInstallTypes;
    skr::FlatHashMap<skr_mesh_resource_id, SPtr<BufferRequest>> mRequests;
};

SMeshFactory* SMeshFactory::Create(const Root& root)
{
    return SkrNew<SMeshFactoryImpl>(root);
}

void SMeshFactory::Destroy(SMeshFactory* factory)
{
    SkrDelete(factory);
}

skr_guid_t SMeshFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::rttr::type_id_of<skr_mesh_resource_t>();
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

ESkrInstallStatus SMeshFactoryImpl::InstallImpl(skr_resource_record_t* record)
{
    const auto noCompression = true;
    auto       vram_service  = root.vram_service;
    auto       mesh_resource = (skr_mesh_resource_t*)record->resource;
    auto       guid          = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        [[maybe_unused]] auto dsqueue = render_device->get_file_dstorage_queue();
        auto                  batch   = vram_service->open_batch(mesh_resource->bins.size());
        if (noCompression)
        {
            auto dRequest = SPtr<BufferRequest>::Create();
            dRequest->absPaths.resize_default(mesh_resource->bins.size());
            dRequest->dFutures.resize_zeroed(mesh_resource->bins.size());
            dRequest->dBuffers.resize_zeroed(mesh_resource->bins.size());
            InstallType installType = { ECompressMethod::NONE };
            for (auto i = 0u; i < mesh_resource->bins.size(); i++)
            {
                auto binPath = skr::format(u8"{}.buffer{}", guid, i);
                // TODO: REFACTOR THIS WITH VFS PATH
                // auto fullBinPath = skr::filesystem::path(root.dstorage_root) / binPath.c_str();
                // auto&& thisPath = dRequest->absPaths[i];
                const auto& thisBin         = mesh_resource->bins[i];
                auto&&      thisFuture      = dRequest->dFutures[i];
                auto&&      thisDestination = dRequest->dBuffers[i];

                CGPUResourceTypes flags = CGPU_RESOURCE_TYPE_NONE;
                flags |= thisBin.used_with_index ? CGPU_RESOURCE_TYPE_INDEX_BUFFER : 0;
                flags |= thisBin.used_with_vertex ? CGPU_RESOURCE_TYPE_VERTEX_BUFFER : 0;

                CGPUBufferDescriptor bdesc = {};
                bdesc.descriptors          = flags;
                bdesc.memory_usage         = CGPU_MEM_USAGE_GPU_ONLY;
                bdesc.flags                = CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION;
                bdesc.size                 = thisBin.byte_length;
                bdesc.name                 = nullptr; // TODO: set name

                auto request = vram_service->open_buffer_request();
                request->set_vfs(root.vfs);
                request->set_path(binPath.c_str());
                request->set_buffer(render_device->get_cgpu_device(), &bdesc);
                request->set_transfer_queue(render_device->get_cpy_queue());
                if (mesh_resource->install_to_ram)
                {
                    auto blob                   = request->pin_staging_buffer();
                    mesh_resource->bins[i].blob = blob.get();
                    blob->add_refcount();
                }
                auto result     = batch->add_request(request, &thisFuture);
                thisDestination = skr::static_pointer_cast<skr::io::IVRAMIOBuffer>(result);
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

ESkrInstallStatus SMeshFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto mesh_resource = (skr_mesh_resource_t*)record->resource;
    auto dRequest      = mRequests.find(mesh_resource);
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
            auto render_mesh = mesh_resource->render_mesh = SkrNew<skr_render_mesh_t>();
            // TODO: remove these requests
            const auto N = dRequest->second->dBuffers.size();
            render_mesh->buffers.resize_default(N);
            for (auto i = 0u; i < N; i++)
            {
                auto pBuffer            = dRequest->second->dBuffers[i]->get_buffer();
                render_mesh->buffers[i] = pBuffer;
            }
            skr_render_mesh_initialize(render_mesh, mesh_resource);

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
            bin.blob->release();
        }
    }
    skr_render_mesh_free(mesh_resource->render_mesh);
    mesh_resource->render_mesh = nullptr;
    return true;
}

} // namespace renderer
} // namespace skr