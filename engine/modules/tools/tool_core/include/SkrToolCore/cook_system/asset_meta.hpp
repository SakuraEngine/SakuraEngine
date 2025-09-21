#pragma once
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/asset_meta.generated.h"

namespace skd::asset
{

struct [[sattr(
    guid = "8e78354d-acf9-40e2-8175-1ad40654360d"
    serde = @enable
)]] TOOL_CORE_API AssetMetadata
{
public:
    using LoadFromJson = void (*)(skr::ArReadJson* reader, skr::RC<AssetMetadata> object);
    using StoreToJson = void (*)(skr::ArWriteJson* writer, skr::RC<AssetMetadata> object);
    virtual ~AssetMetadata();

    template <typename T>
    inline static skr::RC<T> Create()
    {
        auto m = skr::RC<T>::New();
        m->Load = +[](skr::ArReadJson* reader, skr::RC<AssetMetadata> object) {
            auto derived = object.cast_static<T>();
            reader->value(*derived);
        };
        m->Store = +[](skr::ArWriteJson* writer, skr::RC<AssetMetadata> object) {
            auto derived = object.cast_static<T>();
            writer->value(*derived);
        };
        return m;
    }

protected:
    [[sattr(serde = @disable)]]
    LoadFromJson Load;

    [[sattr(serde = @disable)]]
    StoreToJson Store;

    AssetMetadata();
    SKR_RC_IMPL();
};

struct [[sattr(
    guid = "6c429147-d680-4345-a4a3-e8aefd671e6a"
)]] TOOL_CORE_API AssetMetaFile final
{
public:
    AssetMetaFile(const URI& uri);
    AssetMetaFile(const URI& uri, skr::GUID guid, skr::GUID resource_type, skr::GUID cooker);

    template <typename T>
    inline skr::RC<T> GetMetadata();
    inline auto GetProject() const { return project; }
    inline const auto& GetURI() const { return uri; }
    inline skr::GUID GetGUID() const { return guid; }
    inline skr::GUID GetResourceType() const { return resource_type; }
    inline skr::GUID GetCooker() const { return cooker; }
    inline auto GetImporter() const { return importer; }

private:
    inline void SetContent(skr::String&& content)
    {
        meta_content = std::move(content);
    }

    URI uri;
    skr::GUID guid;
    skr::GUID resource_type;
    skr::GUID cooker;

    [[sattr(serde = @disable)]]
    skr::RC<AssetMetadata> metadata = nullptr;

    [[sattr(serde = @disable)]]
    skr::RC<Importer> importer = nullptr;

    [[sattr(serde = @disable)]]
    SProject* project = nullptr;

    [[sattr(serde = @disable)]]
    skr::String meta_content;

    friend struct CookSystemImpl;
    friend struct Serialize<skd::asset::AssetMetaFile>;
    SKR_RC_IMPL();
};

template <typename T>
inline skr::RC<T> AssetMetaFile::GetMetadata()
{
    if (metadata != nullptr)
    {
        return metadata.cast_static<T>();
    }
    else if (!meta_content.is_empty())
    {
        auto METADATA = AssetMetadata::Create<T>();
        auto reader = skr::ArReadJson::ReadBuffer(meta_content.data(), meta_content.size());
        {
            skr::Archive::ObjectScope scope(reader);
            SKR_FAST_CHECK(scope.is_success(), nullptr);

            SKR_FAST_CHECK(reader.key(u8"metadata"), nullptr);
            reader.value(*METADATA);
        }
        metadata = METADATA;
        return METADATA.template cast_static<T>();
    }
    return nullptr;
}

} // namespace skd::asset

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <>
struct Serialize<skd::asset::AssetMetaFile>
{
    inline static void read(ArchiveRead& r, skd::asset::AssetMetaFile& v)
    {
        Archive::ObjectScope scope(r);
        SKR_FAST_CHECK(scope.is_success(), );
        read_fields(r, v);
    }

    inline static void write(ArchiveWrite& w, const skd::asset::AssetMetaFile& v)
    {
        Archive::ObjectScope scope(w);
        SKR_FAST_CHECK(scope.is_success(), );
        write_fields(w, v);
    }

    inline static void read_fields(ArchiveRead& r, skd::asset::AssetMetaFile& v)
    {
        if (!r.is_structured()) [[unlikely]]
        {
            r.error(u8"Non-structured ArchiveRead is not supported for skd::asset::AssetMetaFile");
            return;
        }

        SKR_FAST_CHECK(r.key_value_required(u8"guid", v.guid), );
        SKR_FAST_CHECK(r.key_value_required(u8"resource_type", v.resource_type), );
        SKR_FAST_CHECK(r.key_value(u8"cooker", v.cooker), );
        // TODO. load importer
        // if (r.key(u8"importer"))
        // {
        //     v.importer = skd::asset::GetImporterRegistry()->LoadImporter(&r);
        // }
    }

    inline static void write_fields(ArchiveWrite& w, const skd::asset::AssetMetaFile& v)
    {
        if (!w.is_structured()) [[unlikely]]
        {
            w.error(u8"Non-structured ArchiveWrite is not supported for skd::asset::AssetMetaFile");
            return;
        }

        SKR_FAST_CHECK(w.key_value(u8"guid", v.guid), );
        SKR_FAST_CHECK(w.key_value(u8"resource_type", v.resource_type), );
        SKR_FAST_CHECK(w.key_value(u8"cooker", v.cooker), );

        if (v.importer != nullptr)
        {
            // TODO. store importer
            // w.key(u8"importer");
            // skd::asset::GetImporterRegistry()->StoreImporter(w, v.importer);
        }

        if (v.metadata != nullptr)
        {
            // TODO. store metadata
            // w.key(u8"metadata");
            // v.metadata->Store(w, v.metadata);
        }
    }
};
} // namespace skr