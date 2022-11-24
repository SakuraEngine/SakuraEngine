#pragma once
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "platform/guid.hpp"
#include "resource/resource_factory.h"
#include "type/type_registry.h"
#include "binary/reader_fwd.h"
#include "binary/writer_fwd.h"

typedef struct skr_config_resource_t skr_config_resource_t;

#if defined(__cplusplus)
sreflect_struct("guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")
sattr("rtti" : true)
skr_config_resource_t 
{
    sattr("no-rtti" : true)
    skr_type_id_t configType;
    sattr("no-rtti" : true)
    void* configData;
    ~skr_config_resource_t();
};
namespace skr::binary
{
    template<>
    struct RUNTIME_API ReadHelper<skr_config_resource_t>
    {
    public:
        static int Read(skr_binary_reader_t* reader, skr_config_resource_t& config);
    };

    template<>
    struct RUNTIME_API WriteHelper<const skr_config_resource_t&>
    {
    public:
        static int Write(skr_binary_writer_t* writer, const skr_config_resource_t& config);
    };
}

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
{struct RUNTIME_API SConfigFactory : public SResourceFactory {
    skr_type_id_t GetResourceType() override;

    bool AsyncIO() override { return false; }
};
} // namespace resource
} // namespace skr
#endif