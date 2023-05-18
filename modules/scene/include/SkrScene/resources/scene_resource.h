#pragma once
#include "SkrScene/module.configure.h"
#include "ecs/dual.h"
#include "resource/resource_factory.h"
#include "serde/binary/reader_fwd.h"
#include "serde/binary/writer_fwd.h"
#include "platform/configure.h"
#ifndef __meta__
    #include "SkrScene/resources/scene_resource.generated.h"
#endif

sreflect_struct("guid": "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A")
sattr("rtti" : true) // this is required for the resource to be automatically deserialized
skr_scene_resource_t
{
    sattr("no-rtti" : true)
    dual_storage_t* storage;
};

namespace skr::resource
{
    // scene resource factory, base class
    // derive from this class to implement your own game specific install logic
    class SKR_SCENE_API SSceneFactory : public SResourceFactory
    {
    public:
        virtual ~SSceneFactory() noexcept = default;
        skr_type_id_t GetResourceType() override;
        bool AsyncIO() override { return true; }
        float AsyncSerdeLoadFactor() override { return 0.5f; }
    };
}

namespace skr::binary
{
template <>
struct SKR_SCENE_API ReadTrait<skr_scene_resource_t> {
    static int Read(skr_binary_reader_t* reader, skr_scene_resource_t& value);
};
template <>
struct SKR_SCENE_API WriteTrait<const skr_scene_resource_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_scene_resource_t& value);
};
} // namespace skr::binary