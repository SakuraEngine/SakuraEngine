#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <random>
#include <chrono>
#include <thread>

#define SDL_MAIN_HANDLED
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "signal_client.h"
#include "utils/log.h"
#include <EASTL/vector.h>
#include <imgui/imgui.h>
#include <EASTL/string_hash_map.h>
#include "module/module_manager.hpp"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/window.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "imgui_impl_sdl.h"
#include "cgpu/api.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "runtime_module.h"
#include "SkrRenderer/skr_renderer.h"
#include "imgui/skr_imgui_rg.h"
#include "tracy/Tracy.hpp"
#include "utils/make_zeroed.hpp"
#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif
#define BACK_BUFFER_WIDTH 900
#define BACK_BUFFER_HEIGHT 900
static SteamNetworkingMicroseconds g_logTimeZero;

HSteamListenSocket g_hListenSock;
struct ChatClient* g_client;
const char* pszTrivialSignalingService = "benzzzx.ticp.io:10000";

int g_nVirtualPortLocal = 0;  // Used when listening, and when connecting
int g_nVirtualPortRemote = 0; // Only used when connecting

inline static bool SDLEventHandler(const SDL_Event* event, SDL_Window* window)
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

struct Renderer {
    skr::render_graph::RenderGraph* renderGraph;
    CGPUFenceId presentFence;
    CGPUSwapChainId swapChain;
};

Renderer renderer;
static SWindowHandle mainWindow;
skr_vfs_t* resource_vfs;

int CreateMainWindow()
{
#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    mainWindow = skr_create_window("AAA", &window_desc);
    return 0;
}

int CreateRenderer(SWindowHandle window)
{
    renderer.swapChain = skr_render_device_register_window(window);
    renderer.presentFence = cgpu_create_fence(skr_renderer_get_cgpu_device());
    renderer.renderGraph = skr::render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(skr_renderer_get_cgpu_device())
        .with_gfx_queue(skr_renderer_get_gfx_queue())
        .enable_memory_aliasing();
    });
    return 0;
}

int CreateVFS()
{
    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);
    return 0;
}

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);
int InitializeChat();

int InitializeSockets()
{
    // Initialize library, with the desired local identity
    g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugOutput);
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_LogLevel_P2PRendezvous, k_ESteamNetworkingSocketsDebugOutputType_Debug);
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

void read_bytes(skr_vfs_t* vfs, const char* file_name, uint8_t** bytes, uint32_t* length)
{
    auto vsfile = skr_vfs_fopen(vfs, file_name, SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    *length = (uint32_t)skr_vfs_fsize(vsfile);
    *bytes = (uint8_t*)sakura_malloc(*length);
    skr_vfs_fread(vsfile, *bytes, 0, *length);
    skr_vfs_fclose(vsfile);
}

int InitializeImgui(Renderer& renderer, skr_vfs_t* vfs)
{
    auto renderModule = SkrRendererModule::Get();
    const auto device = renderer.renderGraph->get_backend_device();
    const auto backend = device->adapter->instance->backend;
    const auto gfx_queue = renderer.renderGraph->get_gfx_queue();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
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
    imgui_graph_desc.backbuffer_format = renderModule->get_swapchain_format();
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = renderModule->get_linear_sampler();
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);

    ImGui_ImplSDL2_InitForCGPU((SDL_Window*)mainWindow, renderer.swapChain);
    return 0;
}

enum
{
    AAA_INITIALIZE_WINDOW,
    AAA_INITIALIZE_RENDERER,
    AAA_INITIALIZE_VFS,
    AAA_INITIALIZE_IMGUI,
    AAA_INITIALIZE_SOCKET,
    AAA_INITIALIZE_CHAT,
} initializeState;

int initialize(int argc, const char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("SkrRenderer", true);
    moduleManager->init_module_graph(argc, argv);

    if (auto result = CreateMainWindow(); result != 0)
        return result;
    initializeState = AAA_INITIALIZE_WINDOW;
    if (auto result = CreateRenderer(mainWindow); result != 0)
        return result;
    initializeState = AAA_INITIALIZE_RENDERER;
    if (auto result = CreateVFS(); result != 0)
        return result;
    initializeState = AAA_INITIALIZE_VFS;
    if (auto result = InitializeImgui(renderer, resource_vfs); result != 0)
        return result;
    initializeState = AAA_INITIALIZE_IMGUI;
    if (auto result = InitializeSockets())
        return result;
    initializeState = AAA_INITIALIZE_SOCKET;
    if (auto result = InitializeChat())
        return result;
    initializeState = AAA_INITIALIZE_CHAT;

    return 0;
}

void UpdateChat();

void run()
{
    // loop
    bool quit = false;
    uint32_t backbuffer_index;
    while (!quit)
    {
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

        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // static Uint64 frequency = SDL_GetPerformanceFrequency();
        // Uint64 current_time = SDL_GetPerformanceCounter();
        // io.DeltaTime = bd->Time > 0 ? (float)((double)(current_time - bd->Time) / frequency) : (float)(1.0f / 60.0f);
        // skr_imgui_new_frame(mainWindow, 1.f / 60.f);
        UpdateChat();
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
        render_graph_imgui_add_render_pass(renderer.renderGraph, back_buffer, CGPU_LOAD_ACTION_CLEAR);
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
        cgpu_queue_present(skr_renderer_get_gfx_queue(), &present_desc);
        FrameMark
    }
}

void ShutdownChat();
void Quit(int rc);

void shutdown()
{
    auto moduleManager = skr_get_module_manager();
    cgpu_wait_queue_idle(skr_renderer_get_gfx_queue());
    cgpu_free_fence(renderer.presentFence);
    switch (initializeState)
    {
        case AAA_INITIALIZE_CHAT:
            ShutdownChat();
        case AAA_INITIALIZE_SOCKET:
            GameNetworkingSockets_Kill();
        case AAA_INITIALIZE_IMGUI:
            render_graph_imgui_finalize();
        case AAA_INITIALIZE_VFS:
            skr_free_vfs(resource_vfs);
        case AAA_INITIALIZE_RENDERER:
            skr::render_graph::RenderGraph::destroy(renderer.renderGraph);
        case AAA_INITIALIZE_WINDOW:
            skr_free_window(mainWindow);
            break;
    }
    moduleManager->destroy_module_graph();

    SDL_Quit();
    Quit(0);
}

void Quit(int rc)
{
    if (rc == 0)
    {
        // OK, we cannot just exit the process, because we need to give
        // the connection time to actually send the last message and clean up.
        // If this were a TCP connection, we could just bail, because the OS
        // would handle it.  But this is an application protocol over UDP.
        // So give a little bit of time for good cleanup.  (Also note that
        // we really ought to continue pumping the signaling service, but
        // in this exampple we'll assume that no more signals need to be
        // exchanged, since we've gotten this far.)  If we just terminated
        // the program here, our peer could very likely timeout.  (Although
        // it's possible that the cleanup packets have already been placed
        // on the wire, and if they don't drop, things will get cleaned up
        // properly.)
        SKR_LOG_INFO("Waiting for any last cleanup packets.\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    GameNetworkingSockets_Kill();
    exit(rc);
}

#ifdef _MSC_VER
    #pragma warning(disable : 4702) /* unreachable code */
#endif

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
void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

struct ChatClient {
    SteamNetworkingIdentity id;
    ITrivialSignalingClient* signaling = nullptr;
    struct Connection {
        HSteamNetConnection handle;
        SteamNetworkingIdentity id;
        eastl::vector<eastl::string> messages;
    };
    eastl::vector<Connection> connections;

    ~ChatClient()
    {
        if (signaling)
        {
            signaling->Release();
        }
        if (!connections.empty())
        {
            for (auto& connection : connections)
            {
                SteamNetworkingSockets()->CloseConnection(connection.handle, 0, "quit", true);
            }
        }
    }

    bool Todo()
    {
        // SteamNetworkingSockets()->CloseConnection( g_hConnection, 0, "Test completed OK", true );
        return true;
    }
    void AcceptConnection(HSteamNetConnection conn)
    {
        Connection c;
        c.handle = conn;
        SteamNetConnectionInfo_t Info;
        SteamNetworkingSockets()->GetConnectionInfo(conn, &Info);
        c.id = std::move(Info.m_identityRemote);
        connections.emplace_back(std::move(c));
    }
    void RemoveConnection(HSteamNetConnection conn)
    {
        auto iter = std::find_if(g_client->connections.begin(), g_client->connections.end(), [=](const Connection& connection) {
            return connection.handle == conn;
        });
        if (iter != g_client->connections.end())
            g_client->connections.erase_unsorted(iter);
    }
    void Update()
    {
        // Check for incoming signals, and dispatch them
        if (signaling)
            signaling->Poll();
        // Check callbacks
        SteamNetworkingSockets()->RunCallbacks();
        ImGui::Begin("ControlPanel");
        if (!signaling)
        {
            static char name[256];
            ImGui::InputText("Name", name, 256);
            ImGui::SameLine();
            if (ImGui::Button("SignIn"))
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

                    SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(OnSteamNetConnectionStatusChanged);

                    // Comment this line in for more detailed spew about signals, route finding, ICE, etc
                    // SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_LogLevel_P2PRendezvous, k_ESteamNetworkingSocketsDebugOutputType_Verbose );

                    // Currently you must create a listen socket to use symmetric mode,
                    // even if you know that you will always create connections "both ways".
                    // In the future we might try to remove this requirement.  It is a bit
                    // less efficient, since it always triggered the race condition case
                    // where both sides create their own connections, and then one side
                    // decides to their theirs away.  If we have a listen socket, then
                    // it can be the case that one peer will receive the incoming connection
                    // from the other peer, and since he has a listen socket, can save
                    // the connection, and then implicitly accept it when he initiates his
                    // own connection.  Without the listen socket, if an incoming connection
                    // request arrives before we have started connecting out, then we are forced
                    // to ignore it, as the app has given no indication that it desires to
                    // receive inbound connections at all.
                    SKR_LOG_INFO("Creating listen socket in symmetric mode, local virtual port %d\n", g_nVirtualPortLocal);
                    SteamNetworkingConfigValue_t opt;
                    opt.SetInt32(k_ESteamNetworkingConfig_SymmetricConnect, 1); // << Note we set symmetric mode on the listen socket
                    g_hListenSock = SteamNetworkingSockets()->CreateListenSocketP2P(g_nVirtualPortLocal, 1, &opt);
                    SKR_ASSERT(g_hListenSock != k_HSteamListenSocket_Invalid);
                }
            }
        }
        else
        {
            static char name[256];
            ImGui::InputText("Target", name, 256);
            ImGui::SameLine();
            if (ImGui::Button("Connect"))
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
                if (g_nVirtualPortRemote != g_nVirtualPortLocal)
                {
                    SteamNetworkingConfigValue_t opt;
                    opt.SetInt32(k_ESteamNetworkingConfig_LocalVirtualPort, g_nVirtualPortLocal);
                    vecOpts.push_back(opt);
                }

                // Symmetric mode?  Noce that since we created a listen socket on this local
                // virtual port and tagged it for symmetric connect mode, any connections
                // we create that use the same local virtual port will automatically inherit
                // this setting.  However, this is really not recommended.  It is best to be
                // explicit.
                SteamNetworkingConfigValue_t opt;
                opt.SetInt32(k_ESteamNetworkingConfig_SymmetricConnect, 1);
                vecOpts.push_back(opt);
                SKR_LOG_INFO("Connecting to '%s' in symmetric mode, virtual port %d, from local virtual port %d.\n",
                SteamNetworkingIdentityRender(identityRemote).c_str(), g_nVirtualPortRemote,
                g_nVirtualPortLocal);

                // Connect using the "custom signaling" path.  Note that when
                // you are using this path, the identity is actually optional,
                // since we don't need it.  (Your signaling object already
                // knows how to talk to the peer) and then the peer identity
                // will be confirmed via rendezvous.
                ISteamNetworkingConnectionSignaling* pConnSignaling = signaling->CreateSignalingForConnection(
                identityRemote,
                errMsg);
                SKR_ASSERT(pConnSignaling);
                auto handle = SteamNetworkingSockets()->ConnectP2PCustomSignaling(pConnSignaling, &identityRemote, g_nVirtualPortRemote, (int)vecOpts.size(), vecOpts.data());
                SteamNetworkingSockets()->SetConnectionUserData(handle, (int64)this);
                SKR_ASSERT(handle != k_HSteamNetConnection_Invalid);
            }
        }
        ImGui::End();
        for (auto& connection : connections)
        {
            SteamNetworkingMessage_t* pMessage[10];
            int r = SteamNetworkingSockets()->ReceiveMessagesOnConnection(connection.handle, pMessage, 10);
            for (int i = 0; i < r; ++i)
            {
                connection.messages.push_back((char*)pMessage[i]->GetData());
                pMessage[i]->Release();
            }
            static char txt[256];

            ImGui::Begin(SteamNetworkingIdentityRender(connection.id).c_str());
            ImGui::InputText("message", txt, 256);
            ImGui::SameLine();
            if (ImGui::Button("Send"))
            {
                auto len = strlen(txt);
                if (len != 0)
                {
                    EResult r = SteamNetworkingSockets()->SendMessageToConnection(
                    connection.handle, txt, (int)strlen(txt) + 1, k_nSteamNetworkingSend_Reliable, nullptr);
                    SKR_ASSERT(r == k_EResultOK);
                    connection.messages.push_back(txt);
                }
            }
            for (auto& message : connection.messages)
            {
                ImGui::Text("%s", message.c_str());
            }
            ImGui::End();
        }
    }
};

int InitializeChat()
{
    g_client = new ChatClient();
    return 0;
}

void UpdateChat()
{
    g_client->Update();
}

void ShutdownChat()
{
    delete g_client;
}

// Called when a connection undergoes a state transition.
void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    // What's the state of the connection?
    switch (pInfo->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            SKR_LOG_INFO("[%s] %s, reason %d: %s\n",
            pInfo->m_info.m_szConnectionDescription,
            (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ? "closed by peer" : "problem detected locally"),
            pInfo->m_info.m_eEndReason,
            pInfo->m_info.m_szEndDebug);
            // Close our end
            SteamNetworkingSockets()->CloseConnection(pInfo->m_hConn, 0, nullptr, false);

            g_client->RemoveConnection(pInfo->m_hConn);
        }
        break;

        case k_ESteamNetworkingConnectionState_None:
            // Notification that a connection was destroyed.  (By us, presumably.)
            // We don't need this, so ignore it.
            break;

        case k_ESteamNetworkingConnectionState_Connecting:

            // Is this a connection we initiated, or one that we are receiving?
            if (g_hListenSock != k_HSteamListenSocket_Invalid && pInfo->m_info.m_hListenSocket == g_hListenSock)
            {
                SKR_LOG_INFO("[%s] Accepting\n", pInfo->m_info.m_szConnectionDescription);
                SteamNetworkingSockets()->AcceptConnection(pInfo->m_hConn);
            }
            else
            {
                // Note that we will get notification when our own connection that
                // we initiate enters this state.
                SKR_LOG_INFO("[%s] Entered connecting state\n", pInfo->m_info.m_szConnectionDescription);
            }
            break;

        case k_ESteamNetworkingConnectionState_FindingRoute:
            // P2P connections will spend a brief time here where they swap addresses
            // and try to find a route.
            SKR_LOG_INFO("[%s] finding route\n", pInfo->m_info.m_szConnectionDescription);
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            // We got fully connected
            SKR_LOG_INFO("[%s] connected\n", pInfo->m_info.m_szConnectionDescription);
            g_client->AcceptConnection(pInfo->m_hConn);
            break;

        default:
            SKR_ASSERT(false);
            break;
    }
}

int main(int argc, const char** argv)
{
    auto result = initialize(argc, argv);
    if (result != 0)
    {
        shutdown();
        return result;
    }

    run();

    shutdown();
    return 0;
}