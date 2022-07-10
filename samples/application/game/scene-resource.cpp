#include "scene-resource.hpp"
#include "GameRT/typeid.generated.hpp"
#include "ecs/dual.h"
#include "skr_scene/scene.h"
#include "utils/io.hpp"
#include "resource/resource_system.h"

namespace skg::resource
{
    skr_type_id_t SSceneFactory::GetResourceType()
    {
        return dual_id_of<game_scene_t>();
    }

    ESkrLoadStatus SSceneFactory::Load(skr_resource_record_t* record)
    {
        SBinaryDeserializer archive{ record->activeRequest->GetData() };
        bitsery::serialize(archive, record->header);
        dual_serializer_v v;
        v.is_serialize = +[](void*) { return 0;};
        v.peek = +[](void* u, void* dst, uint32_t size)
        {
            auto& archive = *(SBinaryDeserializer*)u;
            auto pos = archive.adapter().currentReadPos();
            archive.adapter().readBuffer<1>((uint8_t*)dst, size);
            archive.adapter().currentReadPos(pos);
        };
        v.stream = +[](void* u, void* dst, uint32_t size)
        {
            auto& archive = *(SBinaryDeserializer*)u;
            archive.adapter().readBuffer<1>((uint8_t*)dst, size);
        };
        auto storage = dualS_create();
        game_scene_t *scene = SkrNew<game_scene_t>();
        scene->world = storage;
        record->resource = scene;
        record->destructor = +[](void* data)
        {
            game_scene_t *scene = (game_scene_t*)(data);
            dualS_release(scene->world);
            SkrDelete(scene);
        };
        dualS_deserialize(storage, &v, &archive);
        return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED;
    }   

    ESkrInstallStatus SSceneFactory::Install(skr_resource_record_t *record)
    {
        
        return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED;
    }
}