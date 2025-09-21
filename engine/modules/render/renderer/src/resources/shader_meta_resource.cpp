#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/string.hpp>
#include "option_utils.hpp"

namespace skr
{
using namespace skr;

bool ShaderOptionsResource::flatten_options(skr::Vector<ShaderOptionTemplate>& dst, skr::Span<ShaderOptionsResource*> srcs) SKR_NOEXCEPT
{
    skr::Set<skr::String>                                                               keys;
    skr::FlatHashMap<skr::String, ShaderOptionTemplate, skr::Hash<skr::String>> kvs;
    // collect all keys & ensure unique
    for (auto& src : srcs)
    {
        for (auto& opt : src->options)
        {
            if (auto found = keys.find(opt.key))
            {
                dst.is_empty();
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
    [](const ShaderOptionTemplate& a, const ShaderOptionTemplate& b) {
        return skr::Hash<skr::String>()(a.key) < skr::Hash<skr::String>()(b.key);
    });
    return true;
}

StableShaderHash ShaderOptionInstance::calculate_stable_hash(skr::Span<ShaderOptionInstance> ordered_options)
{
    skr::String signatureString;
    option_utils::stringfy(signatureString, ordered_options);
    return StableShaderHash::hash_string(signatureString.c_str_raw(), (uint32_t)signatureString.size());
}

struct SKR_RENDERER_API ShaderOptionsFactoryImpl : public ShaderOptionsFactory {
    ShaderOptionsFactoryImpl(const ShaderOptionsFactoryImpl::Root& root)
        : root(root)
    {
    }

    ~ShaderOptionsFactoryImpl() noexcept = default;

    bool       AsyncIO() override { return false; }
    GUID GetResourceType() override
    {
        const auto collection_type = ::skr::type_id_of<ShaderOptionsResource>();
        return collection_type;
    }

    ESkrInstallStatus Install(SResourceRecord* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }

    bool Unload(SResourceRecord* record) override
    {
        auto options = (ShaderOptionsResource*)record->resource;
        SkrDelete(options);
        return true;
    }
    bool Uninstall(SResourceRecord* record) override
    {
        return true;
    }

    Root root;
};

ShaderOptionsFactory* ShaderOptionsFactory::Create(const Root& root)
{
    return SkrNew<ShaderOptionsFactoryImpl>(root);
}

void ShaderOptionsFactory::Destroy(ShaderOptionsFactory* factory)
{
    return SkrDelete(factory);
}
} // namespace skr