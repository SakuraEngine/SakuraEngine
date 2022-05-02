#include "resource/config_resource.h"
#include "bitsery/deserializer.h"
#include "bitsery/details/adapter_common.h"
#include "platform/configure.h"
#include "resource/resource_factory.h"
#include "resource/resource_header.h"
#include "type/type_registry.h"

namespace skr
{
namespace resource
{
bool SConfigFactory::Deserialize(skr_resource_record_t* record, SBinaryArchive& archive)
{
    if (!SResourceFactory::Deserialize(record, archive))
        return false;
    skr_type_id_t typeId;
    archive(typeId);
    auto type = skr_get_type(&typeId);
    auto align = type->Align();
    auto baseSize = sizeof(skr_config_resource_t);
    auto offset = ((baseSize + align - 1) / align) * align;
    auto size = offset + type->Size();
    auto mem = sakura_malloc(size);
    auto res = new (mem) skr_config_resource_t;
    res->configType = typeId;
    res->configData = (char*)mem + offset;
    DeserializeConfig(typeId, res->configData, archive);
    record->resource = res;
    record->destructor = [](void* mem) {
        auto res = (skr_config_resource_t*)(mem);
        auto type = skr_get_type(&res->configType);
        type->Destruct(res->configData);
        SkrDelete(res);
    };
    return archive.adapter().error() == bitsery::ReaderError::NoError;
}
} // namespace resource
} // namespace skr
