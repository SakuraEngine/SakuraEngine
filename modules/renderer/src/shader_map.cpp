#include "SkrRenderer/shader_map.h"
#include "platform/guid.hpp"
#include "containers/hashmap.hpp"

namespace skr
{
struct ShaderMapImpl : public skr_shader_map_t
{
    CGPUShaderLibraryId find_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT override;
    void free_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT override;

    ESkrShaderMapShaderStatus install_shader(const skr_platform_shader_hash_t& id, skr_shader_collection_resource_t* collection) SKR_NOEXCEPT override;
    bool uninstall_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT override;

    void new_frame(uint64_t frame_index) SKR_NOEXCEPT override;
    void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override;

    uint64_t frame_index = 0;
};

CGPUShaderLibraryId ShaderMapImpl::find_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT
{

    return nullptr;
}

void ShaderMapImpl::free_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT
{

}

ESkrShaderMapShaderStatus ShaderMapImpl::install_shader(const skr_platform_shader_hash_t& id, skr_shader_collection_resource_t* collection) SKR_NOEXCEPT
{
    return SKR_SHADER_MAP_SHADER_STATUS_NONE;
}

bool ShaderMapImpl::uninstall_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT
{
    return false;
}

void ShaderMapImpl::new_frame(uint64_t index) SKR_NOEXCEPT
{
    frame_index = index;
}

void ShaderMapImpl::garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT
{

}


} // namespace skr