#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/resource/resource_factory.h"
#include "SkrRT/serde/binary/reader_fwd.h"
#include "SkrRT/serde/binary/writer_fwd.h"
#include "SkrRT/rttr/rttr_traits.hpp"

typedef struct skr_config_resource_t skr_config_resource_t;

#if defined(__cplusplus)
sreflect_struct("guid" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")
sattr("rtti" : true)
SKR_RUNTIME_API skr_config_resource_t 
{
    sattr("no-rtti" : true)
    skr_guid_t configType;
    sattr("no-rtti" : true)
    void* configData = nullptr;
    void SetType(skr_guid_t type);
    ~skr_config_resource_t();
};

SKR_RTTR_TYPE(skr_config_resource_t, "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74")

namespace skr::binary
{
    template<>
    struct SKR_RUNTIME_API ReadTrait<skr_config_resource_t>
    {
    public:
        static int Read(skr_binary_reader_t* reader, skr_config_resource_t& config);
    };

    template<>
    struct SKR_RUNTIME_API WriteTrait<const skr_config_resource_t&>
    {
    public:
        static int Write(skr_binary_writer_t* writer, const skr_config_resource_t& config);
    };
}

namespace skr
{
namespace resource
{struct SKR_RUNTIME_API SConfigFactory : public SResourceFactory {
    skr_guid_t GetResourceType() override;

    bool AsyncIO() override { return false; }
};
} // namespace resource
} // namespace skr
#endif