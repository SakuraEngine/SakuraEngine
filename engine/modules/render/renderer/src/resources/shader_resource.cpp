#include "SkrRenderer/graphics/shader_map.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrRuntime/resource/resource_factory.h"
#include "option_utils.hpp"

namespace skr
{
using namespace skr;

StableShaderHash::StableShaderHash(uint32_t a, uint32_t b, uint32_t c, uint32_t d) SKR_NOEXCEPT
    : valuea(a)
    , valueb(b)
    , valuec(c)
    , valued(d)
{
}

size_t StableShaderHash::hasher::operator()(const StableShaderHash& hash) const
{
    return skr_hash_of(&hash, sizeof(hash), 114514u);
}

StableShaderHash StableShaderHash::hash_string(const char* str, uint32_t size) SKR_NOEXCEPT
{
    if (!size) return StableShaderHash(0, 0, 0, 0);
    auto result = make_zeroed<StableShaderHash>();
    const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
    result.valuea = skr_hash32_of(str, size, seeds[0]);
    result.valueb = skr_hash32_of(str, size, seeds[1]);
    result.valuec = skr_hash32_of(str, size, seeds[2]);
    result.valued = skr_hash32_of(str, size, seeds[3]);
    return result;
}

StableShaderHash StableShaderHash::from_string(const char* str) SKR_NOEXCEPT
{
    StableShaderHash result;
    result.valuea = std::stoul((const char*)str);
    result.valueb = std::stoul((const char*)str + 8);
    result.valuec = std::stoul((const char*)str + 16);
    result.valued = std::stoul((const char*)str + 24);
    return result;
}

StableShaderHash::operator skr::String() const SKR_NOEXCEPT
{
    return skr::format(u8"{}{}{}{}", valuea, valueb, valuec, valued);
}

size_t PlatformShaderHash::hasher::operator()(const PlatformShaderHash& hash) const
{
    return skr_hash_of(&hash, sizeof(hash));
}

size_t PlatformShaderIdentifier::hasher::operator()(const PlatformShaderIdentifier& hash) const
{
    return skr_hash_of(&hash, sizeof(hash));
}

uint32_t ShaderOptionSequence::find_key_index(skr::StringView in_key) const SKR_NOEXCEPT
{
    for (uint32_t at = 0; at < keys.size(); at++)
    {
        const auto& key = keys[at];
        if (key == in_key) return at;
    }
    return UINT32_MAX;
}

uint32_t ShaderOptionSequence::find_value_index(uint32_t key_index, skr::StringView in_value) const SKR_NOEXCEPT
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

uint32_t ShaderOptionSequence::find_value_index(skr::StringView in_key, skr::StringView in_value) const SKR_NOEXCEPT
{
    uint32_t key_index = find_key_index(in_key);
    return find_value_index(key_index, in_value);
}

StableShaderHash ShaderOptionSequence::calculate_stable_hash(const ShaderOptionSequence& seq, skr::Span<uint32_t> indices)
{
    skr::String signatureString;
    option_utils::stringfy(signatureString, seq, indices);
    return StableShaderHash::hash_string(signatureString.c_str_raw(), (uint32_t)signatureString.size());
}

ShaderCollectionResource::~ShaderCollectionResource() SKR_NOEXCEPT
{
}

struct SKR_RENDERER_API ShaderResourceFactoryImpl : public ShaderResourceFactory
{
    ShaderResourceFactoryImpl(const ShaderResourceFactory::Root& root)
        : root(root)
    {
    }

    ~ShaderResourceFactoryImpl() noexcept = default;
    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(SResourceRecord* record) override;
    ESkrInstallStatus Install(SResourceRecord* record) override;
    bool Uninstall(SResourceRecord* record) override;
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override;

    Root root;
};

ShaderResourceFactory* ShaderResourceFactory::Create(const Root& root)
{
    return SkrNew<ShaderResourceFactoryImpl>(root);
}

void ShaderResourceFactory::Destroy(ShaderResourceFactory* factory)
{
    return SkrDelete(factory);
}

ECGPUShaderBytecodeType ShaderResourceFactory::GetRuntimeBytecodeType(ECGPUBackend backend)
{
    switch (backend)
    {
    case CGPU_BACKEND_D3D12:
        return CGPU_SHADER_BYTECODE_TYPE_DXIL;
    case CGPU_BACKEND_VULKAN:
        return CGPU_SHADER_BYTECODE_TYPE_SPIRV;
    case CGPU_BACKEND_METAL:
        return CGPU_SHADER_BYTECODE_TYPE_MTL;
    default:
        return CGPU_SHADER_BYTECODE_TYPE_COUNT;
    }
}

GUID ShaderResourceFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::type_id_of<ShaderCollectionResource>();
    return resource_type;
}

bool ShaderResourceFactoryImpl::Unload(SResourceRecord* record)
{
    auto shader_collection = (ShaderCollectionResource*)record->resource;
    if (!root.dont_create_shader)
    {
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
    }
    SkrDelete(shader_collection);
    return true;
}

ESkrInstallStatus ShaderResourceFactoryImpl::Install(SResourceRecord* record)
{
    if (root.dont_create_shader) return SKR_INSTALL_STATUS_SUCCEED;

    auto shader_collection = static_cast<ShaderCollectionResource*>(record->resource);
    auto&& root_switch_variant = shader_collection->GetRootStaticVariant();
    auto&& root_option_variant = root_switch_variant.GetRootDynamicVariants();
    bool launch_success = false;
    // load bytecode and create CGPU shader
    for (uint32_t i = 0u; i < root_option_variant.size(); i++)
    {
        const auto& identifier = root_option_variant[i];
        const auto runtime_bytecode_type = ShaderResourceFactory::GetRuntimeBytecodeType(root.render_device->get_backend());
        if (identifier.bytecode_type == runtime_bytecode_type)
        {
            const auto status = root.shadermap->install_shader(identifier);
            if (status != EShaderMapShaderStatus::FAILED)
            {
                launch_success = true;
            }
            break;
        }
    }
    return launch_success ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
}

bool ShaderResourceFactoryImpl::Uninstall(SResourceRecord* record)
{
    return true;
}

ESkrInstallStatus ShaderResourceFactoryImpl::UpdateInstall(SResourceRecord* record)
{
    return SKR_INSTALL_STATUS_SUCCEED;
}

} // namespace skr