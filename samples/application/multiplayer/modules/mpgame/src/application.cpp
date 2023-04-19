#include "application.h"
#include "ecs/type_builder.hpp"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/window.h"
#include "steam/isteamnetworkingutils.h"
#include "steam/isteamnetworkingsockets.h"
#include "steam/steamnetworkingsockets.h"
#include <numeric>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "platform/filesystem.hpp"
#include "cgpu/api.h"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"
#include "imgui/imgui.h"
#include "runtime_module.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_effect.h"
#include "tracy/Tracy.hpp"
#include "imgui_impl_sdl.h"
#include "simdjson.h"
#include "utils/make_zeroed.hpp"
#include "platform/guid.hpp"

#include "math/vector.h"
#include "EASTL/shared_ptr.h"
#include "SkrScene/scene.h"
#include "utils/parallel_for.hpp"
#include "EASTL/fixed_vector.h"
#include "json/writer.h"
#include "ecs/set.hpp"
#include "SkrRenderer/render_viewport.h"
#include "SkrInputSystem/input_modifier.hpp"
#include "MPShared/components.h"
#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif
#include "MPShared/signal_client.h"
#define BACK_BUFFER_WIDTH 1920
#define BACK_BUFFER_HEIGHT 1080
extern void initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);
extern void register_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
extern void finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
MPApplication* g_client;

bool MPApplication::SDLEventHandler(const SDL_Event* event, SDL_Window* window)
{
    ImGui_ImplSDL2_ProcessEvent(event);
    switch (event->type)
    {
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                const int32_t ResizeWidth = event->window.data1;
                const int32_t ResizeHeight = event->window.data2;
                (void)ResizeWidth;
                (void)ResizeHeight;
            }
            else if (event->window.event == SDL_WINDOWEVENT_CLOSE)
                return false;
        default:
            return true;
    }
    return true;
}

int MPApplication::CreateMainWindow()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return -1;
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    mainWindow = skr_create_window("MP", &window_desc);
    inputSystem = skr::input::InputSystem::Create();
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    if (!mainWindow)
        return -1;
    return 0;
}

int MPApplication::CreateRenderer(SWindowHandle window)
{
    auto render_device = skr_get_default_render_device();
    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();
    renderer.swapChain = skr_render_device_register_window(render_device, window);
    renderer.presentFence = cgpu_create_fence(cgpu_device);
    renderer.renderGraph = skr::render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(cgpu_device)
        .with_gfx_queue(gfx_queue)
        .enable_memory_aliasing();
    });
    return 0;
}

int MPApplication::CreateVFS()
{
    auto resourceRoot = (skr::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);
    return 0;
}

static void read_bytes(skr_vfs_t* vfs, const char* file_name, uint8_t** bytes, uint32_t* length)
{
    auto vsfile = skr_vfs_fopen(vfs, file_name, SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    *length = skr_vfs_fsize(vsfile);
    *bytes = (uint8_t*)sakura_malloc(*length);
    skr_vfs_fread(vsfile, *bytes, 0, *length);
    skr_vfs_fclose(vsfile);
}

int MPApplication::InitializeImgui(Renderer& renderer, skr_vfs_t* vfs)
{
    ZoneScopedN("InitializeImgui");
    auto render_device = skr_get_default_render_device();
    const auto device = renderer.renderGraph->get_backend_device();
    const auto backend = device->adapter->instance->backend;
    const auto gfx_queue = renderer.renderGraph->get_gfx_queue();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char* font_path = "./../resources/font/SourceSansPro-Regular.ttf";
        uint8_t* font_bytes;
        uint32_t font_length;
        read_bytes(vfs, font_path, &font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!skr_runtime_is_dpi_aware())
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 0.5f;
        }
        else
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(dpi_scaling);
        }
        ImFontConfig cfg = {};
        cfg.SizePixels = 16.f * dpi_scaling;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_bytes,
        font_length, cfg.SizePixels, &cfg);
        ImGui::GetIO().Fonts->Build();
        sakura_free(font_bytes);
    }
    eastl::string vsname = u8"shaders/imgui_vertex";
    eastl::string fsname = u8"shaders/imgui_fragment";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");

    uint32_t im_vs_length;
    uint8_t* im_vs_bytes;
    read_bytes(vfs, vsname.c_str(), &im_vs_bytes, &im_vs_length);
    uint32_t im_fs_length;
    uint8_t* im_fs_bytes;
    read_bytes(vfs, fsname.c_str(), &im_fs_bytes, &im_fs_length);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "imgui_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = (uint32_t*)im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "imgui_fragment_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = (uint32_t*)im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGPUShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(im_vs_bytes);
    sakura_free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = renderer.renderGraph;
    imgui_graph_desc.backbuffer_format = render_device->get_swapchain_format();
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = render_device->get_linear_sampler();
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);

    ImGui_ImplSDL2_InitForCGPU((SDL_Window*)mainWindow, renderer.swapChain);
    return 0;
}

static SteamNetworkingMicroseconds g_logTimeZero;
static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
    if (eType <= k_ESteamNetworkingSocketsDebugOutputType_Msg)
    {
        SKR_LOG_INFO("%10.6f %s\n", time * 1e-6, pszMsg);
    }
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
    {
        // !KLUDGE! Our logging (which is done while we hold the lock)
        // is occasionally triggering this assert.  Just ignroe that one
        // error for now.
        // Yes, this is a kludge.
        if (strstr(pszMsg, "SteamNetworkingGlobalLock held for"))
            return;

        SKR_ASSERT(!"TEST FAILED");
    }
}
int InitializeSockets()
{
    // Initialize library, with the desired local identity
    g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugOutput);
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_LogLevel_P2PRendezvous, k_ESteamNetworkingSocketsDebugOutputType_Debug);
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_Unencrypted, 2);
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
    {
        SKR_LOG_FATAL("GameNetworkingSockets_Init failed.  %s", errMsg);
        return 1;
    }

    const char* turnList = "turn:benzzzx.ticp.io:3478";
    // Hardcode STUN servers
    SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_STUN_ServerList, "stun:benzzzx.ticp.io:3478");

    // Hardcode TURN servers
    // comma seperated setting lists
    const char* userList = "admin";
    const char* passList = "aaa";

    SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_ServerList, turnList);
    SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_UserList, userList);
    SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_PassList, passList);

    // Allow sharing of any kind of ICE address.
    // We don't have any method of relaying (TURN) in this example, so we are essentially
    // forced to disclose our public address if we want to pierce NAT.  But if we
    // had relay fallback, or if we only wanted to connect on the LAN, we could restrict
    // to only sharing private addresses.
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Enable, k_nSteamNetworkingConfig_P2P_Transport_ICE_Enable_All);

    return 0;
}

int MPApplication::Initialize()
{
    ZoneScopedN("MPApplication::Initialize");
    skr::input::Input::Initialize();

    if (auto result = CreateVFS(); result != 0)
        return result;
    initializeState = MP_INITIALIZE_VFS;
    if (auto result = CreateMainWindow(); result != 0)
        return result;
    initializeState = MP_INITIALIZE_WINDOW;
    if (auto result = CreateRenderer(mainWindow); result != 0)
        return result;
    initializeState = MP_INITIALIZE_RENDERER;
    if (auto result = InitializeImgui(renderer, resource_vfs); result != 0)
        return result;
    initializeState = MP_INITIALIZE_IMGUI;
    if (auto result = InitializeSockets())
        return result;
    initializeState = MP_INITIALIZE_SOCKET;

    
    taskScheduler.initialize({});
    taskScheduler.bind();

    EnterLoginState();
    g_client = this;
    return 0;
}

void MPApplication::Shutdown()
{
    dualJ_wait_all();
    taskScheduler.unbind();
    auto render_device = skr_get_default_render_device();
    auto gfx_queue = render_device->get_gfx_queue();
    auto moduleManager = skr_get_module_manager();
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_fence(renderer.presentFence);
    switch (initializeState)
    {
        case MP_INITIALIZE_SOCKET:
            GameNetworkingSockets_Kill();
        case MP_INITIALIZE_IMGUI:
            render_graph_imgui_finalize();
        case MP_INITIALIZE_RENDERER:
            finalize_render_effects(renderer.renderer, renderer.renderGraph);
            skr::render_graph::RenderGraph::destroy(renderer.renderGraph);
            skr_free_renderer(renderer.renderer);
        case MP_INITIALIZE_WINDOW:
            skr_free_window(mainWindow);
            break;
        case MP_INITIALIZE_VFS:
            skr_free_vfs(resource_vfs);
    }
    moduleManager->destroy_module_graph();
    SDL_Quit();
    delete this;
}

void MPApplication::Render()
{
    ZoneScopedN("MPApplication::Render");
    uint32_t backbuffer_index;
    {
        // acquire frame
        cgpu_wait_fences(&renderer.presentFence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = renderer.presentFence;
        backbuffer_index = cgpu_acquire_next_image(renderer.swapChain, &acquire_desc);
    }
    // render graph setup & compile & exec
    CGPUTextureId native_backbuffer = renderer.swapChain->back_buffers[backbuffer_index];
    auto back_buffer = renderer.renderGraph->create_texture(
    [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
        builder.set_name("backbuffer")
        .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
        .allow_render_target();
    });
    renderer.renderGraph->add_render_pass(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
            eastl::string name = "clear";
            builder.set_name(name.c_str())
                .write(0, back_buffer, CGPU_LOAD_ACTION_CLEAR);
        },
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& context) {
        }
    );
    if(stage == MP_STAGE_GAME)
    {
        // Update camera
        auto cameraUpdate = [=](dual_chunk_view_t* view) {
            auto cameras = dual::get_owned_rw<skr_camera_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                cameras[i].renderer = renderer.renderer;
                cameras[i].viewport_id = 0u; // TODO: viewport id
                cameras[i].viewport_width = renderer.swapChain->back_buffers[0]->width;
                cameras[i].viewport_height = renderer.swapChain->back_buffers[0]->height;
            }
        };
        dualQ_get_views(renderWorld.cameraQuery, DUAL_LAMBDA(cameraUpdate));
        // resolve camera to viewports
        auto viewport_manager = renderer.renderer->get_viewport_manager();
        skr_resolve_cameras_to_viewport(viewport_manager, world.storage);
        {
            ZoneScopedN("RegisterPasses");
            register_render_effects(renderer.renderer, renderer.renderGraph);
        }
        skr_renderer_render_frame(renderer.renderer, renderer.renderGraph);
    }
    render_graph_imgui_add_render_pass(renderer.renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
    renderer.renderGraph->add_present_pass(
    [=](skr::render_graph::RenderGraph& g, skr::render_graph::PresentPassBuilder& builder) {
        builder.set_name("present_pass")
        .swapchain(renderer.swapChain, backbuffer_index)
        .texture(back_buffer, true);
    });
    renderer.renderGraph->compile();
    renderer.renderGraph->execute();
    // present
    CGPUQueuePresentDescriptor present_desc = {};
    present_desc.index = backbuffer_index;
    present_desc.swapchain = renderer.swapChain;
    auto render_device = skr_get_default_render_device();
    auto gfx_queue = render_device->get_gfx_queue();
    cgpu_queue_present(gfx_queue, &present_desc);
}

void MPApplication::Run()
{
    // loop
    while (!ProcessEvent())
    {
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        Update();
        Render();
        FrameMark;
    }
}
const char* pszTrivialSignalingService = "benzzzx.ticp.io:10000";

bool MPApplication::ProcessEvent()
{
    bool quit = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        auto sdl_window = (SDL_Window*)mainWindow;
        if (SDL_GetWindowID(sdl_window) == event.window.windowID)
        {
            if (!SDLEventHandler(&event, sdl_window))
            {
                quit = true;
            }
        }
    }
    return quit;
}

void MPApplication::Update()
{
    ZoneScopedN("MPApplication::Update");
    skr::input::Input::GetInstance()->Tick();
    auto currentTime = skr_hires_timer_get_seconds(&timer, false);
    deltaTime = currentTime - lastUpdateTime;
    lastUpdateTime = currentTime;
    inputSystem->update(deltaTime);
    switch (stage)
    {
        case MP_STAGE_LOGIN:
            UpdateLogin();
            break;
        case MP_STAGE_MENU:
            UpdateMenu();
            break;
        case MP_STAGE_ENTERING_GAME:
            UpdateEnteringGame();
            break;
        case MP_STAGE_GAME:
            UpdateGame();
            break;
    }
}
HSteamListenSocket g_hListenSock;
constexpr bool quickEnterGame = true;
void MPApplication::UpdateLogin()
{
    srand((unsigned)time(NULL));
    static char name[256] = "str:";
    name[4] = rand() % 26 + 'a';
    name[5] = rand() % 26 + 'a';
    name[6] = rand() % 26 + 'a';
    name[7] = '\0';
    auto login = [&]
    {
        id.ParseString(name);
        if (!id.IsInvalid())
        {
            SteamNetworkingSockets()->ResetIdentity(&id);
            SteamDatagramErrMsg errMsg;
            // Create the signaling service
            signaling = CreateTrivialSignalingClient(pszTrivialSignalingService, SteamNetworkingSockets(), errMsg);
            if (signaling == nullptr)
                SKR_LOG_FATAL("Failed to initializing signaling client.  %s", errMsg);

            SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(&MPApplication::OnSteamNetConnectionStatusChanged);
            stage = MP_STAGE_MENU;
        }
    };
    if(quickEnterGame)
    {
        login();
        return;
    }
    ImGui::Begin("Lobby");
    ImGui::InputText("Name", name, 256);
    ImGui::SameLine();
    if (ImGui::Button("SignIn"))
    {
        id.ParseString(name);
        if (!id.IsInvalid())
        {
            login();
        }
    }
    ImGui::End();
}

void MPApplication::OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo )
{
    // What's the state of the connection?
	switch ( pInfo->m_info.m_eState )
	{
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
    {
		SKR_LOG_INFO( "[%s] %s, reason %d: %s\n",
			pInfo->m_info.m_szConnectionDescription,
			( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ? "closed by peer" : "problem detected locally" ),
			pInfo->m_info.m_eEndReason,
			pInfo->m_info.m_szEndDebug
		);
		// Close our end
		SteamNetworkingSockets()->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
    }
    break;

	case k_ESteamNetworkingConnectionState_None:
		// Notification that a connection was destroyed.  (By us, presumably.)
		// We don't need this, so ignore it.
		break;

	case k_ESteamNetworkingConnectionState_Connecting:

		// Is this a connection we initiated, or one that we are receiving?
		if ( g_hListenSock != k_HSteamListenSocket_Invalid && pInfo->m_info.m_hListenSocket == g_hListenSock )
		{
			SKR_LOG_INFO( "[%s] Accepting\n", pInfo->m_info.m_szConnectionDescription );
            SteamNetworkingSockets()->AcceptConnection( pInfo->m_hConn );
		}
		else
		{
			// Note that we will get notification when our own connection that
			// we initiate enters this state.
			SKR_LOG_INFO( "[%s] Entered connecting state\n", pInfo->m_info.m_szConnectionDescription );
		}
		break;

	case k_ESteamNetworkingConnectionState_FindingRoute:
		// P2P connections will spend a brief time here where they swap addresses
		// and try to find a route.
		SKR_LOG_INFO( "[%s] finding route\n", pInfo->m_info.m_szConnectionDescription );
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		// We got fully connected
		SKR_LOG_INFO( "[%s] connected\n", pInfo->m_info.m_szConnectionDescription );
		break;

	default:
		SKR_ASSERT( false );
		break;
	}
}

void MPApplication::UpdateMenu()
{
    ImGui::Begin("Lobby");
    static char name[256] = "str:server";
    auto connect = [&]()
    {
        SteamNetworkingIdentity identityRemote;
        identityRemote.ParseString(name);
        if (identityRemote.IsInvalid())
            return;
        SteamDatagramErrMsg errMsg;
        // Begin connecting to peer, unless we are the server
        std::vector<SteamNetworkingConfigValue_t> vecOpts;

        // If we want the local and virtual port to differ, we must set
        // an option.  This is a pretty rare use case, and usually not needed.
        // The local virtual port is only usually relevant for symmetric
        // connections, and then, it almost always matches.  Here we are
        // just showing in this example code how you could handle this if you
        // needed them to differ.
        if (virtualPortRemote != virtualPortLocal)
        {
            SteamNetworkingConfigValue_t opt;
            opt.SetInt32(k_ESteamNetworkingConfig_LocalVirtualPort, virtualPortLocal);
            vecOpts.push_back(opt);
        }

        // Symmetric mode?  Noce that since we created a listen socket on this local
        // virtual port and tagged it for symmetric connect mode, any connections
        // we create that use the same local virtual port will automatically inherit
        // this setting.  However, this is really not recommended.  It is best to be
        // explicit.
        SKR_LOG_INFO("Connecting to '%s', virtual port %d, from local virtual port %d.\n",
        SteamNetworkingIdentityRender(identityRemote).c_str(), virtualPortRemote,
        virtualPortLocal);

        // Connect using the "custom signaling" path.  Note that when
        // you are using this path, the identity is actually optional,
        // since we don't need it.  (Your signaling object already
        // knows how to talk to the peer) and then the peer identity
        // will be confirmed via rendezvous.
        ISteamNetworkingConnectionSignaling* pConnSignaling = signaling->CreateSignalingForConnection(
        identityRemote,
        errMsg);
        SKR_ASSERT(pConnSignaling);
        world.serverConnection = SteamNetworkingSockets()->ConnectP2PCustomSignaling(pConnSignaling, &identityRemote, virtualPortRemote, (int)vecOpts.size(), vecOpts.data());
        SteamNetworkingSockets()->SetConnectionUserData(world.serverConnection, (int64)this);
        SKR_ASSERT(world.serverConnection != k_HSteamNetConnection_Invalid);
        EnterGameState();
    };
    if(quickEnterGame)
    {
        connect();
        return;
    }
    ImGui::InputText("Server", name, 256);
    ImGui::SameLine();
    if(signaling)
        signaling->Poll();
    SteamNetworkingSockets()->RunCallbacks();
    if (ImGui::Button("Connect"))
    {
        connect();
    }
    ImGui::End();
}

void MPApplication::EnterLoginState()
{
    stage = MP_STAGE_LOGIN;
}

void MPApplication::EnterGameState()
{
    stage = MP_STAGE_ENTERING_GAME;

    // // allocate 1 movable cubes
    // auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    // renderableT_builder
    //     .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t, CController, CMovement>()
    //     .with<skr_render_effect_t>();
    // // allocate renderable
    // auto renderableT = make_zeroed<dual_entity_type_t>();
    // renderableT.type = renderableT_builder.build();
    // auto primSetup = [&](dual_chunk_view_t* view) {
    //     auto translations = (skr_translation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
    //     auto rotations = (skr_rotation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_comp_t>::get());
    //     auto scales = (skr_scale_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_comp_t>::get());
    //     auto movements = (CMovement*)dualV_get_owned_ro(view, dual_id_of<CMovement>::get());
    //     auto controllers = (CController*)dualV_get_owned_ro(view, dual_id_of<CController>::get());
    //     for (uint32_t i = 0; i < view->count; i++)
    //     {
    //         translations[i].value = { 0, 0, 0 };
    //         rotations[i].euler = { 0.f, 0.f, 0.f };
    //         scales[i].value = { 8.f, 8.f, 8.f };
    //         movements[i].speed = 50.f;
    //         controllers[i].playerId = 0;
    //     }
    //     auto renderer = skr_renderer_get_renderer();
    //     skr_render_effect_attach(renderer, view, "ForwardEffect");
    // };
    // dualS_allocate_type(world.storage, &renderableT, 1, DUAL_LAMBDA(primSetup));

}


void MPApplication::UpdateEnteringGame()
{
    ZoneScopedN("UpdateEnteringGame");
    if(signaling)
        signaling->Poll();
    SteamNetworkingSockets()->RunCallbacks();

    SteamNetworkingMessage_t* pMessages[10];
    int r = 0;
    do
    {
        r = SteamNetworkingSockets()->ReceiveMessagesOnConnection(world.serverConnection, pMessages, 1);
        SKR_ASSERT(r != -1);
        for (int i = 0; i < r; ++i)
        {
            auto pMessage = pMessages[i];
            auto data = pMessage->GetData();
            auto type = (MPEventType)*(uint32_t*)data;
            auto size = pMessage->GetSize();
            data = (char*)data + sizeof(uint32_t);
            size -= sizeof(uint32_t);

            switch (type)
            {
                case MPEventType::SyncWorld: {
                    world.Initialize();
                    renderWorld.Initialize(&world);
                    renderer.renderer = skr_create_renderer(skr_get_default_render_device(), renderWorld.storage);
                    renderWorld.renderer = renderer.renderer;
                    // Viewport
                    auto viewport_manager = renderer.renderer->get_viewport_manager();
                    viewport_manager->register_viewport("main_viewport");
                    initialize_render_effects(renderer.renderer, renderer.renderGraph, resource_vfs);
                    renderWorld.LoadScene();
                    world.ReceiveWorldDelta(data, size);
                    world.ApplyWorldDelta();
                    SetupInput(*inputSystem);
                    stage = MP_STAGE_GAME;
                    return;
                }
                break;
                default:
                    SKR_LOG_FATAL("[MPClientWorld::Process] Unknown event: %d", (int)type);
                    break;
            }
            pMessage->Release();
        }
    } while(r);
}

void MPApplication::SetupInput(skr::input::InputSystem &inputSystem)
{
    using namespace skr::input;
    
    auto moveAction = inputSystem.create_input_action(EValueType::kFloat2);
    moveAction->bind_event<skr_float2_t>([this](skr_float2_t value)
    {
        world.input.inputs[0].move = value;
    });
    moveAction->add_trigger(inputSystem.create_trigger<skr::input::InputTriggerChanged>());
    auto mapping_ctx = inputSystem.create_mapping_context();
    inputSystem.add_mapping_context(mapping_ctx, 0, {});
    static skr::input::InputModifierShuffle shuffleToY;
    shuffleToY.shuffle = { 1, 0, 2, 3 };
    static skr::input::InputModifierScale inverse;
    inverse.scale = { -1.f, 1.f, 1.f, 1.f };
    auto mapping_W = inputSystem.create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_W);
    mapping_W->add_modifier(shuffleToY);
    auto mapping_S = inputSystem.create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_S);
    mapping_S->add_modifier(inverse);
    mapping_S->add_modifier(shuffleToY);
    auto mapping_A = inputSystem.create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_A);
    mapping_A->add_modifier(inverse);
    auto mapping_D = inputSystem.create_mapping<skr::input::InputMapping_Keyboard>(EKeyCode::KEY_CODE_D);
    mapping_W->action = moveAction;
    mapping_S->action = moveAction;
    mapping_A->action = moveAction;
    mapping_D->action = moveAction;
    mapping_ctx->add_mapping(mapping_W);
    mapping_ctx->add_mapping(mapping_S);
    mapping_ctx->add_mapping(mapping_A);
    mapping_ctx->add_mapping(mapping_D);

    auto fireAction = inputSystem.create_input_action(EValueType::kBool);
    fireAction->bind_event<bool>([this](bool value)
    {
        world.input.inputs[0].fire = value;
    });
    fireAction->add_trigger(inputSystem.create_trigger<skr::input::InputTriggerChanged>());
    auto mapping_LM = inputSystem.create_mapping<skr::input::InputMapping_MouseButton>(EMouseKey::MOUSE_KEY_LB);
    mapping_LM->action = fireAction;
    mapping_ctx->add_mapping(mapping_LM);

    auto skillAction = inputSystem.create_input_action(EValueType::kBool);
    skillAction->bind_event<bool>([this](bool value)
    {
        world.input.inputs[0].skill = value;
    });
    skillAction->add_trigger(inputSystem.create_trigger<skr::input::InputTriggerChanged>());
    auto mapping_RM = inputSystem.create_mapping<skr::input::InputMapping_MouseButton>(EMouseKey::MOUSE_KEY_RB);
    mapping_RM->action = skillAction;
    mapping_ctx->add_mapping(mapping_RM);

}

void MPApplication::UpdateGame()
{
    ZoneScopedN("UpdateGame");
    inputSystem->update(deltaTime);
    
    if(signaling)
        signaling->Poll();
    SteamNetworkingSockets()->RunCallbacks();
    SteamNetworkingMessage_t* pMessages[10];
    int r = 0;
    bool worldUpdated = false;
    do
    {
        r = SteamNetworkingSockets()->ReceiveMessagesOnConnection(world.serverConnection, pMessages, 10);
        SKR_ASSERT(r != -1);
        for (int i = 0; i < r; ++i)
        {
            auto pMessage = pMessages[i];
            auto data = pMessage->GetData();
            auto type = (MPEventType)*(uint32_t*)data;
            auto size = pMessage->GetSize();
            data = (char*)data + sizeof(uint32_t);
            size -= sizeof(uint32_t);

            switch (type)
            {
                case MPEventType::SyncWorld: {
                    world.ReceiveWorldDelta(data, size);
                    world.ApplyWorldDelta();
                    worldUpdated = true;
                }
                break;
                default:
                    SKR_LOG_FATAL("[MPClientWorld::Process] Unknown event: %d", (int)type);
                    break;
            }
            pMessage->Release();
        }
    } while(r);
    if(worldUpdated)
        world.RollForward();
    worldUpdated = world.Update() || worldUpdated;
    if(worldUpdated)
        renderWorld.UpdateStructuralChanges();
    renderWorld.Update();

    {
        ZoneScopedN("ImGui");
        ImGui::Begin(u8"Networking Statics");
        SteamNetConnectionRealTimeStatus_t status;
        SteamNetworkingSockets()->GetConnectionRealTimeStatus(world.serverConnection, &status, 0, nullptr);
        SteamNetConnectionInfo_t info;
        SteamNetworkingSockets()->GetConnectionInfo(world.serverConnection, &info);
        uint64_t shouldPredictFrames = status.m_nPing * 0.001 / world.GetTickInterval() + 1;
        ImGui::LabelText("frame rate", "%lld", (uint64_t)(1 / deltaTime));
        ImGui::LabelText("ping", "%d", status.m_nPing);
        ImGui::LabelText("current frame", "%llu", world.currentFrame);
        ImGui::LabelText("verified frame", "%llu", world.verifiedFrame);
        ImGui::LabelText("predicted frame", "%llu", world.predictedFrame - world.verifiedFrame);
        ImGui::LabelText("target predict frame", "%llu", shouldPredictFrames);
        ImGui::LabelText("current game time", "%lf", world.currentGameTime);
        ImGui::LabelText("predicted game time", "%lf", world.predictedGameTime);
        ImGui::LabelText("time scale", "%lf", world.timeScale);
        ImGui::LabelText("out bytes per sec", "%f", status.m_flOutBytesPerSec);
        ImGui::LabelText("in bytes per sec", "%f", status.m_flInBytesPerSec);
        ImGui::LabelText("out bandwidth", "%d", status.m_nSendRateBytesPerSecond);
        ImGui::LabelText("network entity count", "%d", dualQ_get_count(world.snapshotQuery));
        ImGui::LabelText("actual in bytes per sec", "%lf", world.GetBytePerSecond());
        ImGui::LabelText("actual in bytes per sec uncompressed", "%lf", world.GetBytePerSecondBeforeCompress());
        ImGui::LabelText("compress ratio", "%lf", world.GetCompressRatio());
        ImGui::LabelText("lan mode", ((info.m_nFlags & k_nSteamNetworkConnectionInfoFlags_Fast) != 0) ? "true" : "false");
        ImGui::LabelText("relay mode", ((info.m_nFlags & k_nSteamNetworkConnectionInfoFlags_Relayed) != 0) ? "true" : "false");
        ImGui::LabelText("encrypted mode", ((info.m_nFlags & k_nSteamNetworkConnectionInfoFlags_Unencrypted) == 0) ? "true" : "false");
        char buf[256];
        info.m_addrRemote.ToString(buf, 256, true);
        ImGui::LabelText("remote address", "%s", buf);
        ImGui::LabelText("description", "%s", info.m_szConnectionDescription);
        if(ImGui::CollapsingHeader("detailed component bandwidth"))
        {
            auto type = GetNetworkComponents();
            double totalComponentBandwidth = 0;
            for(int i = 0; i < type.length; ++i)
            {
                auto name = dualT_get_desc(type.data[i])->name;
                auto bandwidth = world.worldDeltaApplier->GetBandwidthOf(type.data[i]);
                totalComponentBandwidth += bandwidth;
                ImGui::LabelText(name, "%f", bandwidth);
            }
            ImGui::LabelText("other", "%f", world.GetBytePerSecondBeforeCompress() - totalComponentBandwidth);
        }

        bool predictionEnabled = world.predictionEnabled;
        if(ImGui::Checkbox("prediction", &predictionEnabled))
        {
            world.SetPredictionEnabled(predictionEnabled);
        }
        ImGui::End();

        ImGui::Begin(u8"Game Statics");
        ImGui::ProgressBar(world.GetPlayerHealth() / 100.f, ImVec2(-FLT_MIN, 0.f), "health");
        ImGui::End();
    }
}

IApplication* CreateMPApplication()
{
    return SkrNew<MPApplication>();
}