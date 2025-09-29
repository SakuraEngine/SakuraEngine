#pragma once
#include "SkrBase/types/guid.hpp"
#include "SkrCore/memory/rc.hpp"
#include "SkrCore/serialize/json_archive.hpp"
#include "SkrToolCore/fwd_types.hpp"
#include "SkrToolCore/cook_system/importer.generated.h" // IWYU pragma: export

namespace skr
{
using namespace skr;
template <class T>
void RegisterImporter(skr::GUID guid);

struct [[sattr(
    guid = "76044661-E2C9-43A7-A4DE-AEDD8FB5C847";
    serde = @enable
)]] TOOL_CORE_API Importer
{
public:
    SKR_GENERATE_BODY(Importer);

    using CreateFN = skr::RC<Importer> (*)();
    using LoadFromJson = void (*)(skr::ArReadJson* reader, skr::RC<Importer> object);
    using StoreToJson = void (*)(skr::ArWriteJson* writer, skr::RC<Importer> object);

    static constexpr uint32_t kDevelopmentVersion = UINT32_MAX;
    static uint32_t Version() { return kDevelopmentVersion; }

    virtual ~Importer() = default;
    virtual void* Import(skr::io::IRAMService*, CookContext* context) = 0;
    virtual void Destroy(void*) = 0;
    inline skr::GUID GetType() const { return importer_type; }

    template <typename T>
    inline static skr::RC<T> Create()
    {
        auto i = skr::RC<T>::New();
        i->Load = +[](skr::ArReadJson* reader, skr::RC<Importer> object) {
            auto derived = object.cast_static<T>();
            reader->value(*derived);
        };
        i->Store = +[](skr::ArWriteJson* writer, skr::RC<Importer> object) {
            auto derived = object.cast_static<T>();
            writer->value(*derived);
        };
        i->importer_type = skr::type_id_of<T>();
        return i;
    }

protected:
    skr::GUID importer_type;

    [[sattr(serde = @disable)]]
    LoadFromJson Load = nullptr;

    [[sattr(serde = @disable)]]
    StoreToJson Store = nullptr;

    Importer();
    SKR_RC_IMPL();
};

struct TOOL_CORE_API ImporterTypeInfo
{
    Importer::CreateFN Create;
    Importer::LoadFromJson Load;
    Importer::StoreToJson Store;
    uint32_t (*Version)();
};

struct ImporterRegistry
{
    virtual skr::RC<Importer> LoadImporter(skr::ArReadJson* importer) = 0;
    virtual void StoreImporter(skr::ArWriteJson* writer, skr::RC<Importer> importer) = 0;
    virtual uint32_t GetImporterVersion(skr::GUID type) = 0;
    virtual void RegisterImporter(skr::GUID type, ImporterTypeInfo info) = 0;
};

TOOL_CORE_API ImporterRegistry* GetImporterRegistry();
} // namespace skr

template <class T>
void skr::RegisterImporter(skr::GUID guid)
{
    auto registry = GetImporterRegistry();
    auto create = +[]() {
        auto derived = Importer::Create<T>();
        return derived.template cast_static<Importer>();
    };
    auto loader = +[](skr::ArReadJson* reader, skr::RC<Importer> object) -> void {
        auto derived = object.cast_static<T>();
        reader->value(*derived);
    };
    auto store = +[](skr::ArWriteJson* writer, skr::RC<Importer> object) -> void {
        auto derived = object.cast_static<T>();
        writer->value(*object);
    };
    ImporterTypeInfo info{ create, loader, store, T::Version };
    registry->RegisterImporter(guid, info);
}