#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/string.hpp>
#include "option_utils.hpp"

bool skr_shader_options_resource_t::flatten_options(skr::Vector<skr_shader_option_template_t>& dst, skr::span<skr_shader_options_resource_t*> srcs) SKR_NOEXCEPT
{
    skr::Set<skr::String>                                                              keys;
    skr::FlatHashMap<skr::String, skr_shader_option_template_t, skr::Hash<skr::String>> kvs;
    // collect all keys & ensure unique
    for (auto& src : srcs)
    {
        for (auto& opt : src->options)
        {
            if (auto found = keys.find(opt.key))
            {
                dst.empty();
                return false;
            }
            keys.add(opt.key);
            kvs.insert({ opt.key, opt });
        }
    }
    dst.reserve(keys.size());
    for (auto& key : keys)
    {
        dst.add(kvs[key]);
    }
    // sort result by key
    dst.sort_stable(
    [](const skr_shader_option_template_t& a, const skr_shader_option_template_t& b) {
        return skr::Hash<skr::String>()(a.key) < skr::Hash<skr::String>()(b.key);
    });
    return true;
}

skr_stable_shader_hash_t skr_shader_option_instance_t::calculate_stable_hash(skr::span<skr_shader_option_instance_t> ordered_options)
{
    skr::String signatureString;
    option_utils::stringfy(signatureString, ordered_options);
    return skr_stable_shader_hash_t::hash_string(signatureString.c_str(), (uint32_t)signatureString.size());
}

namespace skr
{
namespace renderer
{
using namespace skr::resource;

struct SKR_RENDERER_API SShaderOptionsFactoryImpl : public SShaderOptionsFactory {
    SShaderOptionsFactoryImpl(const SShaderOptionsFactoryImpl::Root& root)
        : root(root)
    {
    }

    ~SShaderOptionsFactoryImpl() noexcept = default;

    bool       AsyncIO() override { return false; }
    skr_guid_t GetResourceType() override
    {
        const auto collection_type = ::skr::rttr::type_id<skr_shader_options_resource_t>();
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

void SShaderOptionsFactory::Destroy(SShaderOptionsFactory* factory)
{
    return SkrDelete(factory);
}
} // namespace renderer
} // namespace skr