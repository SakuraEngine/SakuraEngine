#pragma once
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "platform/guid.hpp"
#include "resource/resource_factory.h"
#include "type/type_registry.h"
#include "binary/reader.h"
#include "binary/writer.h"

struct skr_config_resource_t;

#if defined(__cplusplus)
inline static constexpr skr_guid_t get_type_id_skr_config_resource_t()
{ 
    return {0x8F2DE9A2, 0xFE05, 0x4EB7, {0xA0, 0x7F, 0xA9, 0x73, 0xE3, 0xE9, 0x2B, 0x74}}; 
}

namespace skr
{
namespace type
{
template <> struct type_id<skr_config_resource_t> {
    static const skr_guid_t get() { return get_type_id_skr_config_resource_t(); }
};
}
namespace resource
{
struct SConfigTypeInfo {
    void (*Serialize)(void* address, skr_binary_writer_t& archive) = nullptr;
    void (*Deserialize)(void* address, skr_binary_reader_t& archive) = nullptr;
};
struct SConfigRegistry {
    virtual ~SConfigRegistry() {}
    virtual void RegisterConfigType(const skr_guid_t& guid, const SConfigTypeInfo& info) = 0;
    virtual const SConfigTypeInfo* FindConfigType(const skr_guid_t& guid) = 0;
};
RUNTIME_API SConfigRegistry* GetConfigRegistry();

struct RUNTIME_API SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override;
    static struct skr_config_resource_t* NewConfig(skr_type_id_t& id);
    static void DestroyConfig(struct skr_config_resource_t* config);

    bool AsyncIO() override { return false; }
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    bool Deserialize(skr_resource_record_t* record, skr_binary_reader_t& archive);
    static void Serialize(const skr_config_resource_t& config, skr_binary_writer_t& archive);
    static void DeserializeConfig(const skr_type_id_t& id, void* address, skr_binary_reader_t& archive);
    static void SerializeConfig(const skr_type_id_t& id, void* address, skr_binary_writer_t& archive);
};

template<class T>
inline static void RegisterConfig(skr_guid_t guid)
{
    SConfigTypeInfo typeInfo {
        +[](void* address, skr_binary_writer_t& archive)
        {
            skr::binary::WriteValue<const T&>(&archive, *(T*)address);
        },
        +[](void* address, skr_binary_reader_t& archive)
        {
            skr::binary::ReadValue<T>(&archive, *(T*)address);
        }
    };
    GetConfigRegistry()->RegisterConfigType(guid, typeInfo);
}
#define sregister_config() sstatic_ctor(skr::resource::RegisterConfig<$T>($guid));
} // namespace resource
} // namespace skr
#endif


sreflect_struct("guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")
skr_config_resource_t 
{
    typedef struct skr_config_resource_t RawData;
    skr_type_id_t configType;
    void* configData;
};
typedef struct skr_config_resource_t skr_config_resource_t;