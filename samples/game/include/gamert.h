#pragma once
#include "gamert_configure.h"
#include "platform/vfs.h"

#include "module/module_manager.hpp"
#include "render_graph/frontend/render_graph.hpp"
#include "utils/log.h"

namespace skr::resource
{
struct SLocalResourceRegistry;
}
class SGameRTModule : public skr::IDynamicModule
{
    virtual void on_load() override;
    virtual void main_module_exec() override;
    virtual void on_unload() override;

public:
    skr_vfs_t* resource_vfs = nullptr;
    skr::resource::SLocalResourceRegistry* registry;
};

namespace skg
{
struct GameContext {
};
GAMERT_API bool GameLoop(GameContext& ctx);
} // namespace skg
