#pragma once
#ifdef RUNTIME_SHARED
#define EA_DLL
#endif

#define USE_VK
#include "platform/adaptive_types.h"
#include <nvrhi/nvrhi.h>

namespace Render
{
    struct DefaultMessageCallback : public nvrhi::IMessageCallback
    {
        inline static DefaultMessageCallback& GetInstance()
        {
            static DefaultMessageCallback Instance;
            return Instance;
        }

        inline void message(nvrhi::MessageSeverity severity, const char* messageText) override
        {
            printf("%s", messageText);
        }
    };

    struct RenderDevice
    {
        //virtual int CreateSwapChain(const PlatformWindow& window) = 0;
        [[nodiscard]] virtual nvrhi::IDevice *GetDevice() const = 0;
        virtual void BeginFrame(int SwapChainIndex) = 0;
        virtual void Present(int SwapChainIndex) = 0;    
        virtual void ResizeSwapChain(int SwapChainIndex, uint32_t backBufferWidth, uint32_t backBufferHeight) = 0;
        virtual void ResizeSwapChain(int SwapChainIndex, const struct SwapChainParams& Params) = 0;
        virtual nvrhi::ITexture* GetCurrentBackBuffer(int SwapChainIndex) = 0;

        static RenderDevice* CreateVulkan(const PlatformWindow window, const struct DeviceCreationParameters& params);
    };

    struct SwapChainParams
    {
        nvrhi::Format swapChainFormat = nvrhi::Format::SRGBA8_UNORM;
        uint32_t backBufferWidth;
        uint32_t backBufferHeight;
        uint32_t swapChainBufferCount = 3;
        bool vsyncEnabled = true;
    };

    struct DeviceCreationParameters
    {
        bool startMaximized = false;
        bool startFullscreen = false;
        bool allowModeSwitch = true;
        int windowPosX = -1;            // -1 means use default placement
        int windowPosY = -1;
        SwapChainParams swapChainParams;
        uint32_t refreshRate = 0;
        uint32_t swapChainSampleCount = 1;
        uint32_t swapChainSampleQuality = 0;
        uint32_t maxFramesInFlight = 2;
        bool enableDebugRuntime = false;
        bool enableNvrhiValidationLayer = false;
        bool enableRayTracingExtensions = false; // for vulkan
        bool enableComputeQueue = false;
        bool enableCopyQueue = false;

#if USE_DX11 || USE_DX12
        // Adapter to create the device on. Setting this to non-null overrides adapterNameSubstring.
        // If device creation fails on the specified adapter, it will *not* try any other adapters.
        IDXGIAdapter* adapter = nullptr;
#endif

        // For use in the case of multiple adapters; only effective if 'adapter' is null. If this is non-null, device creation will try to match
        // the given string against an adapter name.  If the specified string exists as a sub-string of the
        // adapter name, the device and window will be created on that adapter.  Case sensitive.
        std::wstring adapterNameSubstring = L"";

        // set to true to enable DPI scale factors to be computed per monitor
        // this will keep the on-screen window size in pixels constant
        //
        // if set to false, the DPI scale factors will be constant but the system
        // may scale the contents of the window based on DPI
        //
        // note that the backbuffer size is never updated automatically; if the app
        // wishes to scale up rendering based on DPI, then it must set this to true
        // and respond to DPI scale factor changes by resizing the backbuffer explicitly
        bool enablePerMonitorDPI = false;

#if USE_DX11 || USE_DX12
        DXGI_USAGE swapChainUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
#endif
    };
}
