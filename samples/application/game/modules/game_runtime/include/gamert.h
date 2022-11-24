#pragma once
#include "GameRT/module.configure.h"
#include "platform/vfs.h"
#include "utils/log.h"
#include "SkrRenderer/skr_renderer.h"
#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "SkrRenderGraph/frontend/render_graph.hpp"
    #include "utils/io.hpp"
    #include "cgpu/io.hpp"

struct dual_storage_t;
namespace skr::resource
{
struct SLocalResourceRegistry;
struct STextureFactory;
struct SMeshFactory;
struct SShaderResourceFactory;
}
class GAMERT_API SGameRTModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

public:
    static SGameRTModule* Get();
};

namespace skg
{
struct GameContext {
};
GAMERT_API bool GameLoop(GameContext& ctx);
} // namespace skg
#endif
