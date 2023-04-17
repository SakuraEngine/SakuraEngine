#pragma once
#include "MPShared/client_world.h"
#include "MPGame/mp_interface.h"
#include "SkrInputSystem/input_system.hpp"
#include "render_world.h"

struct skr_vfs_t;
typedef union SDL_Event SDL_Event;
typedef struct SDL_Window SDL_Window;
typedef struct SWindow SWindow;
typedef SWindow* SWindowHandle;

struct MP_GAME_API MPApplication : IApplication
{
    static bool SDLEventHandler(const SDL_Event* event, SDL_Window* window);

    struct Renderer
    {
        skr::render_graph::RenderGraph* renderGraph;
        SRendererId renderer;
        CGPUFenceId presentFence;
        CGPUSwapChainId swapChain;
    };

    Renderer renderer;
    SWindowHandle mainWindow;
    skr_vfs_t* resource_vfs;
    MPClientWorld world;
    MPRenderWorld renderWorld;
    SteamNetworkingIdentity id;
    class ITrivialSignalingClient* signaling = nullptr;
    int virtualPortLocal = 0;
    int virtualPortRemote = 0;
    SHiresTimer timer;
    double lastUpdateTime;
    double deltaTime;
    skr::input::InputSystem* inputSystem;
    skr::task::scheduler_t taskScheduler;

    int CreateMainWindow();

    int CreateRenderer(SWindowHandle window);

    int CreateVFS();

    int InitializeImgui(Renderer& renderer, skr_vfs_t* vfs);

    void SetupInput(skr::input::InputSystem &inputSystem);

    enum {
        MP_INITIALIZE_VFS,
        MP_INITIALIZE_WINDOW,
        MP_INITIALIZE_RENDERER,
        MP_INITIALIZE_IMGUI,
        MP_INITIALIZE_SOCKET,
    } initializeState;

    enum {
        MP_STAGE_LOGIN,
        MP_STAGE_MENU,
        MP_STAGE_ENTERING_GAME,
        MP_STAGE_GAME,
    } stage;

    int Initialize() override;

    void Run() override;

    bool ProcessEvent();
    void Update();

    void UpdateLogin();
    void UpdateMenu();
    void UpdateEnteringGame();
    void UpdateGame();

    void Render();
    void ProcessEvents();

    void Shutdown() override;

    void EnterGameState();
    void EnterLoginState();
    void EnterMenuState();

    static void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo );

    virtual ~MPApplication() {}
};