#include "SkrProfile/profile.h"
#include "SkrOS/thread.h"
#include "SkrCore/memory/sp.hpp"
#include "SkrGraphics/cgpux.hpp"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/resources/mesh_resource.h"

namespace skr
{

MeshResource::~MeshResource() SKR_NOEXCEPT
{
    
}

void MeshComponent::SetMeshResource(skr::AsyncResource<skr::MeshResource> mesh) 
{ 
    mesh_resource = mesh; 
}

skr::AsyncResource<skr::MeshResource>& MeshComponent::GetMeshResource() 
{ 
    return mesh_resource; 
}

const skr::AsyncResource<skr::MeshResource>& MeshComponent::GetMeshResource() const 
{ 
    return mesh_resource; 
}

void MeshComponent::SetOverrideMaterial(uint32_t slot, skr::AsyncResource<MaterialResource> mat)
{
    override_materials.add(slot, mat);
}

skr::AsyncResource<MaterialResource> MeshComponent::GetOverrideMaterial(uint32_t slot)
{
    if (auto found = override_materials.find(slot))
        return found.value();
    return {};
}

static struct SkrMeshResourceUtil
{
    struct RegisteredVertexLayout : public CGPUVertexLayout
    {
        RegisteredVertexLayout(const CGPUVertexLayout& layout, VertexLayoutId id, const char8_t* name)
            : CGPUVertexLayout(layout)
            , id(id)
            , name(name)
        {
            hash = cgpux::hash<CGPUVertexLayout>()(layout);
        }
        VertexLayoutId id;
        skr::String name;
        uint64_t hash;
    };

    using VertexLayoutIdMap = skr::FlatHashMap<VertexLayoutId, skr::SP<RegisteredVertexLayout>, skr::Hash<GUID>>;
    using VertexLayoutHashMap = skr::FlatHashMap<uint64_t, RegisteredVertexLayout*>;

    SkrMeshResourceUtil()
    {
        skr_init_mutex_recursive(&vertex_layouts_mutex_);
    }

    ~SkrMeshResourceUtil()
    {
        skr_destroy_mutex(&vertex_layouts_mutex_);
    }

    inline static VertexLayoutId AddVertexLayout(VertexLayoutId id, const char8_t* name, const CGPUVertexLayout& layout)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto pLayout = skr::SP<RegisteredVertexLayout>::New(layout, id, name);
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

    inline static CGPUVertexLayout* HasVertexLayout(const CGPUVertexLayout& layout, VertexLayoutId* outGuid = nullptr)
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

    inline static VertexLayoutId GetVertexLayoutId(CGPUVertexLayout* pLayout)
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

    inline static const char* GetVertexLayout(VertexLayoutId id, CGPUVertexLayout* layout = nullptr)
    {
        SMutexLock lock(vertex_layouts_mutex_);

        auto iter = id_map.find(id);
        if (iter == id_map.end()) return nullptr;
        if (layout) *layout = *iter->second;
        return iter->second->name.c_str_raw();
    }

    static VertexLayoutIdMap id_map;
    static VertexLayoutHashMap hash_map;
    static SMutex vertex_layouts_mutex_;
} mesh_resource_util;
SkrMeshResourceUtil::VertexLayoutIdMap SkrMeshResourceUtil::id_map;
SkrMeshResourceUtil::VertexLayoutHashMap SkrMeshResourceUtil::hash_map;
SMutex SkrMeshResourceUtil::vertex_layouts_mutex_;

} // namespace skr

void skr_mesh_resource_free(skr::MeshResource* mesh_resource)
{
    SkrDelete(mesh_resource);
}

void skr_mesh_resource_register_vertex_layout(skr::VertexLayoutId id, const char8_t* name, const struct CGPUVertexLayout* in_vertex_layout)
{
    if (auto layout = skr::mesh_resource_util.GetVertexLayout(id))
        return; // existed
    else if (auto layout = skr::mesh_resource_util.HasVertexLayout(*in_vertex_layout))
        return; // existed
    else        // do register
    {
        auto result = skr::mesh_resource_util.AddVertexLayout(id, name, *in_vertex_layout);
        SKR_ASSERT(result == id);
    }
}

const char* skr_mesh_resource_query_vertex_layout(skr::VertexLayoutId id, struct CGPUVertexLayout* out_vertex_layout)
{
    if (auto name = skr::mesh_resource_util.GetVertexLayout(id, out_vertex_layout))
        return name;
    else
        return nullptr;
}