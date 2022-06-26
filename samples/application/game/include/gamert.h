#pragma once
#include "gamert_configure.h"
#include "platform/vfs.h"
#include "utils/log.h"
#ifdef __cplusplus
#include "module/module_manager.hpp"
#include "render_graph/frontend/render_graph.hpp"

struct dual_storage_t;
namespace skr::resource
{
struct SLocalResourceRegistry;
}
class GAMERT_API SGameRTModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual void main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

public:
    skr_vfs_t* resource_vfs = nullptr;
    skr::resource::SLocalResourceRegistry* registry;
    struct dual_storage_t* ecs_world;
};

namespace skg
{
struct GameContext {
};
GAMERT_API bool GameLoop(GameContext& ctx);
} // namespace skg

extern "C"
{
#endif
GAMERT_API struct dual_storage_t* gamert_get_ecs_world();
#ifdef __cplusplus
}
#endif