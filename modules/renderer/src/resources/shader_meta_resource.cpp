#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/set.h>

bool skr_shader_options_resource_t::flatten_options(eastl::vector<skr_shader_option_t>& dst, skr::span<skr_shader_options_resource_t*> srcs) SKR_NOEXCEPT
{
    eastl::set<eastl::string> keys;
    skr::flat_hash_map<eastl::string, eastl::vector<eastl::string>, eastl::hash<eastl::string>> kvs;
    // collect all keys & ensure unique
    for (auto& src : srcs)
    {
        for (auto& opt : src->options)
        {
            auto&& found = keys.find(opt.key);
            if (found != keys.end())
            {
                dst.empty();
                return false;
            }
            keys.insert(opt.key);
            kvs.insert({opt.key, opt.value_selections});
        }
    }
    dst.reserve(keys.size());
    for (auto& key : keys)
    {
        dst.push_back({key, kvs[key]});
    }
    return true;
}

skr_stable_shader_hash_t skr_shader_option_instance_t::calculate_stable_hash(skr::span<skr_shader_option_instance_t> ordered_options)
{
    // TODO: check ordered here
    eastl::string sourceString;
    for (auto&& option : ordered_options)
    {
        sourceString += option.key;
        sourceString += "=";
        sourceString += option.value;
        sourceString += ";";
    }
    auto result = make_zeroed<skr_stable_shader_hash_t>();
    const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
    result.valuea = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[0]);
    result.valueb = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[1]);
    result.valuec = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[2]);
    result.valued = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[3]);
    return result;
}

namespace skr
{
namespace resource
{
struct SKR_RENDERER_API SShaderOptionsFactoryImpl : public SShaderOptionsFactory
{
    SShaderOptionsFactoryImpl(const SShaderOptionsFactoryImpl::Root& root)
        : root(root)
    {

    }

    ~SShaderOptionsFactoryImpl() noexcept = default;

    bool AsyncIO() override { return false; }
    skr_type_id_t GetResourceType() override
    {
        const auto collection_type = skr::type::type_id<skr_shader_options_resource_t>::get();
        return collection_type;
    }

    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }

    bool Unload(skr_resource_record_t* record) override
    {
        auto options = (skr_shader_options_resource_t*)record->resource;
        SkrDelete(options);
        return true; 
    }
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    
    Root root;
};

SShaderOptionsFactory* SShaderOptionsFactory::Create(const Root& root)
{
    return SkrNew<SShaderOptionsFactoryImpl>(root);
}

void SShaderOptionsFactory::Destroy(SShaderOptionsFactory *factory)
{
    return SkrDelete(factory);
}
} // namespace resource
} // namespace skr