#pragma once
#include "SkrBase/config.h"
#include "SkrCore/platform/vfs.h"
#include "SkrCore/log.h"
#include "SkrRenderer/skr_renderer.h"
#ifdef __cplusplus
    #include "SkrCore/module/module_manager.hpp"
    #include "SkrRenderGraph/frontend/render_graph.hpp"
    #include "SkrRT/io/ram_io.hpp"
    #include "SkrRT/io/vram_io.hpp"


struct sugoi_storage_t;
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
