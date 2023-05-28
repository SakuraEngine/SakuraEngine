#pragma once
#include "GameRuntime/module.configure.h"
#include "platform/vfs.h"
#include "misc/log.h"
#include "SkrRenderer/skr_renderer.h"
#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "SkrRenderGraph/frontend/render_graph.hpp"
    #include "io/io.h"
    #include "cgpu/io.h"

struct dual_storage_t;
namespace skr::resource
{
struct SLocalResourceRegistry;
struct STextureFactory;
struct SMeshFactory;
struct SShaderResourceFactory;
}
class GAME_RUNTIME_API SGameRTModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

public:
    static SGameRTModule* Get();
};

namespace skg
{
struct GameContext {
};
GAME_RUNTIME_API bool GameLoop(GameContext& ctx);
} // namespace skg
#endif
