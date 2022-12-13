#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "utils/format.hpp"
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include "utils/io.h"
#include "option_utils.hpp"

#include "tracy/Tracy.hpp"

skr_stable_shader_hash_t::skr_stable_shader_hash_t(uint32_t a, uint32_t b, uint32_t c, uint32_t d) SKR_NOEXCEPT
    : valuea(a), valueb(b), valuec(c), valued(d)
{

}

size_t skr_stable_shader_hash_t::hasher::operator()(const skr_stable_shader_hash_t &hash) const
{
    return skr_hash(&hash, sizeof(hash), 114514u);
}

skr_stable_shader_hash_t skr_stable_shader_hash_t::hash_string(const char* str, uint32_t size) SKR_NOEXCEPT
{
    if (!size) return skr_stable_shader_hash_t(0, 0, 0, 0);
    auto result = make_zeroed<skr_stable_shader_hash_t>();
    const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
    result.valuea = skr_hash32(str, size, seeds[0]);
    result.valueb = skr_hash32(str, size, seeds[1]);
    result.valuec = skr_hash32(str, size, seeds[2]);
    result.valued = skr_hash32(str, size, seeds[3]);
    return result;
}

skr_stable_shader_hash_t skr_stable_shader_hash_t::from_string(const char* str) SKR_NOEXCEPT
{
    skr_stable_shader_hash_t result;
    result.valuea = std::stoul(str);
    result.valueb = std::stoul(str + 8);
    result.valuec = std::stoul(str + 16);
    result.valued = std::stoul(str + 24);
    return result;
}

skr_stable_shader_hash_t::operator skr::string() const SKR_NOEXCEPT
{
    return skr::format("{}{}{}{}", valuea, valueb, valuec, valued);
}

size_t skr_platform_shader_hash_t::hasher::operator()(const skr_platform_shader_hash_t& hash) const
{
    return skr_hash(&hash, sizeof(hash), 114514u);
}

size_t skr_platform_shader_identifier_t::hasher::operator()(const skr_platform_shader_identifier_t& hash) const
{
    return skr_hash(&hash, sizeof(hash), 114514u);
}

uint32_t skr_shader_option_sequence_t::find_key_index(skr::string_view in_key) const SKR_NOEXCEPT
{
    for (uint32_t at = 0; at < keys.size(); at++)
    {
        const auto& key = keys[at];
        if (key == in_key) return at;
    }
    return UINT32_MAX;
}

uint32_t skr_shader_option_sequence_t::find_value_index(uint32_t key_index, skr::string_view in_value) const SKR_NOEXCEPT
{
    for (uint32_t v_idx = 0; v_idx < values[key_index].size(); v_idx++)
    {
        const auto& value = values[key_index][v_idx];
        if (value == in_value)
        {
            return v_idx;
        }
    }
    return UINT32_MAX;
}

uint32_t skr_shader_option_sequence_t::find_value_index(skr::string_view in_key, skr::string_view in_value) const SKR_NOEXCEPT
{
    uint32_t key_index = find_key_index(in_key);
    return find_value_index(key_index, in_value);
}

skr_stable_shader_hash_t skr_shader_option_sequence_t::calculate_stable_hash(const skr_shader_option_sequence_t& seq, skr::span<uint32_t> indices)
{
    option_utils::opt_signature_string signatureString;
    option_utils::stringfy(signatureString, seq, indices);
    return skr_stable_shader_hash_t::hash_string(signatureString.c_str(), (uint32_t)signatureString.size());
}

namespace skr
{
namespace resource
{
struct SKR_RENDERER_API SShaderResourceFactoryImpl : public SShaderResourceFactory
{
    SShaderResourceFactoryImpl(const SShaderResourceFactory::Root& root)
        : root(root)
    {

    }

    ~SShaderResourceFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;

    ECGPUShaderBytecodeType GetRuntimeBytecodeType() const
    {
        const auto backend = root.render_device->get_backend();
        switch (backend)
        {
        case CGPU_BACKEND_D3D12: return CGPU_SHADER_BYTECODE_TYPE_DXIL;
        case CGPU_BACKEND_VULKAN: return CGPU_SHADER_BYTECODE_TYPE_SPIRV;
        case CGPU_BACKEND_METAL: return CGPU_SHADER_BYTECODE_TYPE_MTL;
        default: return CGPU_SHADER_BYTECODE_TYPE_COUNT;
        }
    }

    Root root;
};

SShaderResourceFactory* SShaderResourceFactory::Create(const Root& root)
{
    return SkrNew<SShaderResourceFactoryImpl>(root);
}

void SShaderResourceFactory::Destroy(SShaderResourceFactory *factory)
{
    return SkrDelete(factory);
}

skr_type_id_t SShaderResourceFactoryImpl::GetResourceType()
{
    const auto resource_type = skr::type::type_id<skr_shader_collection_resource_t>::get();
    return resource_type;
}

bool SShaderResourceFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto shader_collection = (skr_shader_collection_resource_t*)record->resource;
    for (auto&& [hash, variant] : shader_collection->switch_variants)
    {
        for (auto&& [platform, opt_variant] : variant.option_variants)
        {
            for (auto&& identifier : opt_variant)
            {
                root.shadermap->free_shader(identifier);
            }
        }
    }
    SkrDelete(shader_collection);
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::Install(skr_resource_record_t* record)
{
    if (root.dont_create_shader) return SKR_INSTALL_STATUS_SUCCEED;
    
    auto shader_collection = static_cast<skr_shader_collection_resource_t*>(record->resource);
    auto&& root_switch_variant = shader_collection->GetRootStaticVariant();
    auto&& root_option_variant = root_switch_variant.GetRootDynamicVariants();
    bool launch_success = false;
    // load bytecode and create CGPU shader
    for (uint32_t i = 0u; i < root_option_variant.size(); i++)
    {
        const auto& identifier = root_option_variant[i];
        const auto runtime_bytecode_type = GetRuntimeBytecodeType();
        if (identifier.bytecode_type == runtime_bytecode_type)
        {
            const auto status = root.shadermap->install_shader(identifier);
            if (status != SKR_SHADER_MAP_SHADER_STATUS_FAILED)
            {
                launch_success = true;
            }
            break;
        }
    }
    return launch_success ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
}

bool SShaderResourceFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    return SKR_INSTALL_STATUS_SUCCEED;
}

} // namespace resource
} // namespace skr