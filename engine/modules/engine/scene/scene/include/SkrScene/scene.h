#pragma once

#include "SkrScene/actor.h"
#include "SkrScene/scene.generated.h"

namespace skr
{
struct Actor;
struct RootActor;

struct [[sattr(
    guid = "01990eb5-b4e9-741c-b764-8d1c71e0beb2";
    rttr = @enable
)]] SKR_SCENE_API Scene
{
public:
    Scene() {}
    ~Scene() SKR_NOEXCEPT;
    void serialize();
    void deserialize();
    skr_guid_t root_actor_guid;
    [[sattr(serde = @disable)]]
    skr::Map<skr_guid_t, skr::RC<skr::Actor>> actors;
};

template <>
struct SKR_SCENE_API Serialize<Scene>
{
    static void read(ArchiveRead& r, skr::Scene& v);
    static void write(ArchiveWrite& w, const skr::Scene& v);
};
} // namespace skr