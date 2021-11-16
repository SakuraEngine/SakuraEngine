#define RUNTIME_DLL

#include "gtest/gtest.h"
#include "cgpu/api.h"
#if defined(_WIN32) || defined(_WIN64)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#include "windows.h"
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND createWin32Window();
#elif defined(_MACOS)
#include "platform/macos/window.h"
#endif

class SwapChainCreation : public::testing::TestWithParam<ECGPUBackEnd>
{
protected:
	void SetUp() override
	{
        ECGPUBackEnd backend = GetParam();
        CGpuInstanceDescriptor desc;
        desc.backend = backend;
        desc.enableDebugLayer = true;
        desc.enableGpuBasedValidation = true;
        instance = cgpu_create_instance(&desc);

        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGpuAdapterId> adapters; adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        adapter = adapters[0];

        CGpuQueueGroupDescriptor G = {ECGpuQueueType_Graphics, 1};
        DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);

#ifdef _WIN32 
        hwnd = createWin32Window();
#elif defined (_MACOS)
        nswin = nswindow_create();
#endif
    }

    CGpuSwapChainId CreateSwapChainWithSurface(CGpuSurfaceId surface)
    {
        auto mainQueue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        DECLARE_ZERO(CGpuSwapChainDescriptor, descriptor)
        descriptor.presentQueues = &mainQueue;
        descriptor.presentQueuesCount = 1;
        descriptor.surface = surface;
        descriptor.imageCount = 3;
        descriptor.format = PF_R8G8B8A8_UNORM;
        descriptor.enableVsync = true;
        
        auto swapchain = cgpu_create_swapchain(device, &descriptor);
        return swapchain;
    }

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    CGpuInstanceId instance;
    CGpuAdapterId adapter;
    CGpuDeviceId device;
#ifdef _WIN32 
    HWND hwnd;
#elif defined (_MACOS)
    void* nswin;
#endif
};

#if defined(_WIN32) || defined(_WIN64)
TEST_P(SwapChainCreation, CreateFromHWND)
{
    auto surface = cgpu_surface_from_hwnd(device, hwnd);

    EXPECT_NE(surface, CGPU_NULLPTR);
    EXPECT_NE(surface, nullptr);

    auto swapchain = CreateSwapChainWithSurface(surface);

    EXPECT_NE(swapchain, CGPU_NULLPTR);
    EXPECT_NE(swapchain, nullptr);

    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
}
#elif defined (__APPLE__) 
#define BACK_BUFFER_WIDTH 1280
#define BACK_BUFFER_HEIGHT 720

TEST_P(SwapChainCreation, CreateFromNSView)
{
    auto ns_view = (struct NSView*)nswindow_get_content_view(
        nswin
    );
    auto surface = cgpu_surface_from_ns_view(device, ns_view);

    EXPECT_NE(surface, CGPU_NULLPTR);
    EXPECT_NE(surface, nullptr);

    auto swapchain = CreateSwapChainWithSurface(surface);

    EXPECT_NE(swapchain, CGPU_NULLPTR);
    EXPECT_NE(swapchain, nullptr);

    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
}
#endif

static const auto allPlatforms = testing::Values(
#ifndef TEST_WEBGPU    
    #ifdef CGPU_USE_VULKAN
        ECGPUBackEnd_VULKAN
    #endif
    #ifdef CGPU_USE_D3D12
        ,ECGPUBackEnd_D3D12
    #endif
#endif
);

INSTANTIATE_TEST_SUITE_P(SwapChainCreation, SwapChainCreation, allPlatforms);


#if defined(_WIN32) || defined(_WIN64)
LRESULT CALLBACK WindowProcedure( HWND window, UINT msg, WPARAM wp, LPARAM lp )
{
    switch(msg)
    {
    case WM_DESTROY:
        std::cout << "\ndestroying window\n" ;
        PostQuitMessage(0) ;
        return 0L ;
    case WM_LBUTTONDOWN:
        std::cout << "\nmouse left button down at (" << LOWORD(lp) << ',' << HIWORD(lp) << ")\n" ;
    default:
        return DefWindowProc( window, msg, wp, lp ) ;
    }
}

HWND createWin32Window()
{
    // Register the window class.
    auto myclass = TEXT("myclass") ;
    WNDCLASSEX wndclass = 
    { 
        sizeof(WNDCLASSEX), CS_DBLCLKS,
        WindowProcedure,
        0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION),
        LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW+1),
        0, myclass, LoadIcon(0,IDI_APPLICATION) 
    };
    static bool bRegistered = RegisterClassEx(&wndclass);
    if(bRegistered)
    {
        HWND window = CreateWindowEx( 0, myclass, TEXT("title"),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0 ) ;
        if(window)
        {
            ShowWindow( window, SW_SHOWDEFAULT ) ;
        }
        return window;
    }
    return CGPU_NULLPTR;
}
#endif

//137900114