#include "common/common.h"
#include <EASTL/unordered_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/queue.h>
#ifdef USE_VK
#include <nvrhi/vulkan.h>
#include <nvrhi/validation.h>

#if _WINDOWS
#include "vulkan/vulkan_win32.h"
#endif
#if __has_include("SDL2/SDL_main.h")
#include "SDL2/SDL_vulkan.h"
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char *layerPrefix,
    const char *msg,
    void *userData)
{
    printf("[Vulkan: location=%zu code=%d, layerPrefix='%s'] %s", location, code, layerPrefix, msg);

    return VK_FALSE;
}

static eastl::vector<const char *> stringSetToVector(const eastl::unordered_set<eastl::string>& set)
{
    eastl::vector<const char *> ret;
    for(const auto& s : set)
    {
        ret.push_back(s.c_str());
    }
    return ret;
}

template <typename T>
static eastl::vector<T> setToVector(const eastl::unordered_set<T>& set)
{
    eastl::vector<T> ret;
    for(const auto& s : set)
    {
        ret.push_back(s);
    }

    return ret;
}

class RenderDeviceVK : public Render::RenderDevice
{
public:
    RenderDeviceVK(const PlatformWindow window, const struct Render::DeviceCreationParameters& params)
        :m_DeviceParams(params)
    {
        if (!createDeviceAndSwapChain(window))
        {
            printf("Initialize Vulkan Device Failed!");
        }
    }
    [[nodiscard]] nvrhi::IDevice *GetDevice() const override;
    void BeginFrame(int SwapChainIndex) override;
    void Present(int SwapChainIndex) override;    
    void ResizeSwapChain(int SwapChainIndex, const Render::SwapChainParams& Params) override;
    void ResizeSwapChain(int SwapChainIndex, uint32_t backBufferWidth, uint32_t backBufferHeight) override;
    nvrhi::ITexture* GetCurrentBackBuffer(int SwapChainIndex) override;
private:
    bool createDeviceAndSwapChain(const PlatformWindow window);
    bool createInstance();
    bool createWindowSurface(const PlatformWindow window, vk::SurfaceKHR* p_SurfaceKHR);
    void installDebugCallback();
    bool pickPhysicalDevice(vk::SurfaceKHR surface);
    bool findQueueFamilies(vk::PhysicalDevice physicalDevice);
    bool createDevice();
    int32_t createSwapChain(vk::SurfaceKHR SurfaceKHR, const Render::SwapChainParams& Params);
    void destroySwapChain(uint32_t index);
protected:
    vk::Instance m_VulkanInstance;
    vk::DebugReportCallbackEXT vkdebug_callback;
    vk::PhysicalDevice m_VulkanPhysicalDevice;
    int m_GraphicsQueueFamily = -1;
    int m_ComputeQueueFamily = -1;
    int m_TransferQueueFamily = -1;
    int m_PresentQueueFamily = -1;

    vk::Device m_VulkanDevice;
    vk::Queue m_GraphicsQueue;
    vk::Queue m_ComputeQueue;
    vk::Queue m_TransferQueue;
    vk::Queue m_PresentQueue;
    
    struct SwapChainImage
    {
        vk::Image image;
        nvrhi::TextureHandle rhiHandle;
    };

    struct SwapChain
    {
        vk::SurfaceKHR m_WindowSurface;
        vk::SurfaceFormatKHR m_SwapChainFormat;
        vk::SwapchainKHR m_SwapChain;
        eastl::vector<SwapChainImage> m_SwapChainImages;
        uint32_t m_SwapChainIndex = uint32_t(-1);

        inline uint32_t GetCurrentBackBufferIndex() 
        {
            return m_SwapChainIndex;
        }
        inline uint32_t GetBackBufferCount() 
        {
            return uint32_t(m_SwapChainImages.size());
        }
        inline nvrhi::ITexture* GetCurrentBackBuffer() 
        {
            return m_SwapChainImages[m_SwapChainIndex].rhiHandle;
        }
        inline nvrhi::ITexture* GetBackBuffer(uint32_t index) 
        {
            if (index < m_SwapChainImages.size())
                return m_SwapChainImages[index].rhiHandle;
            return nullptr;
        }
    };
    eastl::unordered_map<int, SwapChain> m_SwapChains;
    int m_SwapChainCount = 0;

    nvrhi::vulkan::DeviceHandle m_NvrhiDevice;
    nvrhi::DeviceHandle m_ValidationLayer;

    nvrhi::CommandListHandle m_BarrierCommandList;
    vk::Semaphore m_PresentSemaphore;

    eastl::queue<nvrhi::EventQueryHandle> m_FramesInFlight;
    eastl::vector<nvrhi::EventQueryHandle> m_QueryPool;

    Render::DeviceCreationParameters m_DeviceParams;
    eastl::string m_DeviceName;

    struct VulkanExtensionSet
    {
        eastl::unordered_set<eastl::string> instance;
        eastl::unordered_set<eastl::string> layers;
        eastl::unordered_set<eastl::string> device;
    };
    
    eastl::unordered_set<eastl::string> m_RayTracingExtensions = {
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
    };
    // optional extensions
    VulkanExtensionSet optionalExtensions = {
        // instance
        { 
            VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME
        },
        // layers
        eastl::unordered_set<eastl::string>(),
        // device
        { 
            "VK_KHR_portability_subset",

            VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_NV_MESH_SHADER_EXTENSION_NAME,
            VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME
        },
    };
    // minimal set of required extensions
    VulkanExtensionSet enabledExtensions = {
        // instance
        {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        },
        // layers
        eastl::unordered_set<eastl::string>(),
        // device
        { 
#ifdef __APPLE__
            "VK_KHR_create_renderpass2",
            "VK_KHR_timeline_semaphore",
#endif
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME
        },
    };
};

nvrhi::IDevice* RenderDeviceVK::GetDevice() const
{
    return m_NvrhiDevice;
}

#ifdef max
#undef max
#endif
void RenderDeviceVK::BeginFrame(int SwapChainIndex) 
{
    auto&& swapchain = m_SwapChains[SwapChainIndex];
    const vk::Result res = m_VulkanDevice.acquireNextImageKHR(
        swapchain.m_SwapChain, eastl::numeric_limits<uint64_t>::max(), // timeout
        m_PresentSemaphore, vk::Fence(), &swapchain.m_SwapChainIndex);

    assert(res == vk::Result::eSuccess);

    m_NvrhiDevice->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics, m_PresentSemaphore, 0);
}

void RenderDeviceVK::Present(int SwapChainIndex) 
{
    m_NvrhiDevice->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, m_PresentSemaphore, 0);

    m_BarrierCommandList->open(); // umm...
    m_BarrierCommandList->close();
    m_NvrhiDevice->executeCommandList(m_BarrierCommandList);

    vk::PresentInfoKHR info = vk::PresentInfoKHR()
                                .setWaitSemaphoreCount(1)
                                .setPWaitSemaphores(&m_PresentSemaphore)
                                .setSwapchainCount(1)
                                .setPSwapchains(&m_SwapChains[SwapChainIndex].m_SwapChain)
                                .setPImageIndices(&m_SwapChains[SwapChainIndex].m_SwapChainIndex);

    const vk::Result res = m_PresentQueue.presentKHR(&info);
    assert(res == vk::Result::eSuccess || res == vk::Result::eErrorOutOfDateKHR);

    if (m_DeviceParams.enableDebugRuntime)
    {
        // according to vulkan-tutorial.com, "the validation layer implementation expects
        // the application to explicitly synchronize with the GPU"
        m_PresentQueue.waitIdle();
    }
    else
    {
#ifndef _WIN32
        if (m_DeviceParams.swapChainParams.vsyncEnabled)
        {
            m_PresentQueue.waitIdle();
        }
#endif

        while (m_FramesInFlight.size() > m_DeviceParams.maxFramesInFlight)
        {
            auto query = m_FramesInFlight.front();
            m_FramesInFlight.pop();

            m_NvrhiDevice->waitEventQuery(query);

            m_QueryPool.push_back(query);
        }

        nvrhi::EventQueryHandle query;
        if (!m_QueryPool.empty())
        {
            query = m_QueryPool.back();
            m_QueryPool.pop_back();
        }
        else
        {
            query = m_NvrhiDevice->createEventQuery();
        }

        m_NvrhiDevice->resetEventQuery(query);
        m_NvrhiDevice->setEventQuery(query, nvrhi::CommandQueue::Graphics);
        m_FramesInFlight.push(query);
    }
}
void RenderDeviceVK::ResizeSwapChain(int SwapChainIndex, const Render::SwapChainParams& Params) 
{
    auto&& chain = m_SwapChains[SwapChainIndex];
    if (m_VulkanDevice)
    {
        auto surf = chain.m_WindowSurface;
        destroySwapChain(SwapChainIndex);
        auto idx = createSwapChain(surf, Params);
        m_SwapChains[SwapChainIndex] = m_SwapChains[idx];
        m_SwapChains.erase(idx);
        m_SwapChainCount--;
    }
}
void RenderDeviceVK::ResizeSwapChain(int SwapChainIndex, uint32_t backBufferWidth, uint32_t backBufferHeight) 
{
    Render::SwapChainParams Params = m_DeviceParams.swapChainParams;
    Params.backBufferWidth = backBufferWidth;
    Params.backBufferHeight = backBufferHeight;
    ResizeSwapChain(SwapChainIndex, Params);
}
nvrhi::ITexture* RenderDeviceVK::GetCurrentBackBuffer(int SwapChainIndex) 
{
    return m_SwapChains[SwapChainIndex].GetCurrentBackBuffer();
}

bool RenderDeviceVK::createInstance()
{
    // figure out which optional extensions are supported
    for(const auto& instanceExt : vk::enumerateInstanceExtensionProperties())
    {
        const eastl::string name = instanceExt.extensionName.data();
        if (optionalExtensions.instance.find(name) != optionalExtensions.instance.end())
        {
            enabledExtensions.instance.insert(name);
        }
    }

    printf("Enabled Vulkan instance extensions:\n");
    for (const auto& ext : enabledExtensions.instance)
    {
        printf("    %s\n", ext.c_str());
    }

    for(const auto& layer : vk::enumerateInstanceLayerProperties())
    {
        const eastl::string name = layer.layerName.data();
        if (optionalExtensions.layers.find(name) != optionalExtensions.layers.end())
        {
            enabledExtensions.layers.insert(name);
        }
    }

    printf("Enabled Vulkan layers:\n");
    for (const auto& layer : enabledExtensions.layers)
    {
        printf("    %s\n", layer.c_str());
    }

    auto instanceExtVec = stringSetToVector(enabledExtensions.instance);
    auto layerVec = stringSetToVector(enabledExtensions.layers);

    auto applicationInfo = vk::ApplicationInfo()
        .setApiVersion(VK_MAKE_VERSION(1, 2, 0));

    // create the vulkan instance
    vk::InstanceCreateInfo info = vk::InstanceCreateInfo()
        .setEnabledLayerCount(uint32_t(layerVec.size()))
        .setPpEnabledLayerNames(layerVec.data())
        .setEnabledExtensionCount(uint32_t(instanceExtVec.size()))
        .setPpEnabledExtensionNames(instanceExtVec.data())
        .setPApplicationInfo(&applicationInfo);

    const vk::Result res = vk::createInstance(&info, nullptr, &m_VulkanInstance);
    if (res != vk::Result::eSuccess)
    {
        printf("Failed to create a Vulkan instance, error code = %s\n", nvrhi::vulkan::resultToString(res));
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_VulkanInstance);

    return true;
}

void RenderDeviceVK::installDebugCallback()
{
    auto info = vk::DebugReportCallbackCreateInfoEXT()
                    .setFlags(vk::DebugReportFlagBitsEXT::eError |
                              vk::DebugReportFlagBitsEXT::eWarning |
                            //   vk::DebugReportFlagBitsEXT::eInformation |
                              vk::DebugReportFlagBitsEXT::ePerformanceWarning)
                    .setPfnCallback(vulkanDebugCallback);

    vk::Result res = m_VulkanInstance.createDebugReportCallbackEXT(&info, nullptr, &vkdebug_callback);
    assert(res == vk::Result::eSuccess);
}

bool RenderDeviceVK::createDeviceAndSwapChain(const PlatformWindow window)
{
    if (m_DeviceParams.enableDebugRuntime)
    {
        enabledExtensions.instance.insert("VK_EXT_debug_report");
        enabledExtensions.layers.insert("VK_LAYER_KHRONOS_validation");
    }
#if __has_include("SDL2/SDL_vulkan.h")
    if(window.sdl.window != nullptr)
    {
        unsigned int sdl_exts_count;
        auto get_sdl_ext_res = SDL_Vulkan_GetInstanceExtensions(window.sdl.window, &sdl_exts_count, nullptr);
        eastl::vector<const char*> sdl_exts(sdl_exts_count);

        get_sdl_ext_res = SDL_Vulkan_GetInstanceExtensions(window.sdl.window, &sdl_exts_count, sdl_exts.data());
        if(get_sdl_ext_res)
        {
            for(uint32_t i = 0; i < sdl_exts_count; i++)
            {
                enabledExtensions.instance.insert(eastl::string(sdl_exts[i]));
            }
        }
    }
#endif
    const vk::DynamicLoader dl;
    const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =   // NOLINT(misc-misplaced-const)
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

#define CHECK(a) if (!(a)) { return false; }

    CHECK(createInstance())
    
    if (m_DeviceParams.enableDebugRuntime)
    {
        installDebugCallback();
    }

    if (m_DeviceParams.swapChainParams.swapChainFormat == nvrhi::Format::SRGBA8_UNORM)
        m_DeviceParams.swapChainParams.swapChainFormat = nvrhi::Format::SBGRA8_UNORM;
    else if (m_DeviceParams.swapChainParams.swapChainFormat == nvrhi::Format::RGBA8_UNORM)
        m_DeviceParams.swapChainParams.swapChainFormat = nvrhi::Format::BGRA8_UNORM;
    
    vk::SurfaceKHR SurfaceKHR;
    CHECK(createWindowSurface(window, &SurfaceKHR))
    CHECK(pickPhysicalDevice(SurfaceKHR))
    CHECK(findQueueFamilies(m_VulkanPhysicalDevice))
    CHECK(createDevice())

    auto vecInstanceExt = stringSetToVector(enabledExtensions.instance);
    auto vecLayers = stringSetToVector(enabledExtensions.layers);
    auto vecDeviceExt = stringSetToVector(enabledExtensions.device);

    nvrhi::vulkan::DeviceDesc deviceDesc;
    deviceDesc.errorCB = &Render::DefaultMessageCallback::GetInstance();
    deviceDesc.instance = m_VulkanInstance;
    deviceDesc.physicalDevice = m_VulkanPhysicalDevice;
    deviceDesc.device = m_VulkanDevice;
    deviceDesc.graphicsQueue = m_GraphicsQueue;
    deviceDesc.graphicsQueueIndex = m_GraphicsQueueFamily;
    if (m_DeviceParams.enableComputeQueue)
    {
        deviceDesc.computeQueue = m_ComputeQueue;
        deviceDesc.computeQueueIndex = m_ComputeQueueFamily;
    }
    if (m_DeviceParams.enableCopyQueue)
    {
        deviceDesc.transferQueue = m_TransferQueue;
        deviceDesc.transferQueueIndex = m_TransferQueueFamily;
    }
    deviceDesc.instanceExtensions = vecInstanceExt.data();
    deviceDesc.numInstanceExtensions = vecInstanceExt.size();
    deviceDesc.deviceExtensions = vecDeviceExt.data();
    deviceDesc.numDeviceExtensions = vecDeviceExt.size();

    m_NvrhiDevice = nvrhi::vulkan::createDevice(deviceDesc);

    if (m_DeviceParams.enableNvrhiValidationLayer)
    {
        m_ValidationLayer = nvrhi::validation::createValidationLayer(m_NvrhiDevice);
    }
    CHECK(createSwapChain(SurfaceKHR, m_DeviceParams.swapChainParams) == 0)
    m_BarrierCommandList = m_NvrhiDevice->createCommandList();
    m_PresentSemaphore = m_VulkanDevice.createSemaphore(vk::SemaphoreCreateInfo());
#undef CHECK
    return true;
}

bool RenderDeviceVK::createWindowSurface(const PlatformWindow window, vk::SurfaceKHR* p_SurfaceKHR)
{
    VkResult res = VkResult::VK_ERROR_UNKNOWN;
#if __has_include("SDL2/SDL_vulkan.h")
    if(window.sdl.window != nullptr)
    {
        auto sdl_res = SDL_Vulkan_CreateSurface(window.sdl.window, m_VulkanInstance, (VkSurfaceKHR*)p_SurfaceKHR);
        res = (sdl_res == SDL_TRUE) ? VkResult::VK_SUCCESS : VkResult::VK_ERROR_UNKNOWN;
        if (res != VK_SUCCESS)
        {
            printf("Failed to create a SDL window surface, error: %s\n", SDL_GetError());
        }
    }
#endif
#if _WINDOWS
    if (res != VkResult::VK_SUCCESS)
    {
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.hwnd = window.win.hWnd;
        createInfo.hinstance = window.win.hInstance;
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        // TODO: link properly or fetch this procaddr
        //res = vkCreateWin32SurfaceKHR(m_VulkanInstance, &createInfo, nullptr, (VkSurfaceKHR *)&m_WindowSurface);
    }
#endif
    if (res != VK_SUCCESS)
    {
        printf("Failed to create a window surface, error code = %s\n", nvrhi::vulkan::resultToString(res));
        return false;
    }
    return true;
}

void RenderDeviceVK::destroySwapChain(uint32_t index)
{
    if (m_VulkanDevice)
    {
        m_VulkanDevice.waitIdle();
    }

    if (m_SwapChains[index].m_SwapChain)
    {
        m_VulkanDevice.destroySwapchainKHR(m_SwapChains[index].m_SwapChain);
        m_SwapChains[index].m_SwapChain = nullptr;
    }

    m_SwapChains[index].m_SwapChainImages.clear();
}

int32_t RenderDeviceVK::createSwapChain(vk::SurfaceKHR SurfaceKHR, const Render::SwapChainParams& Params)
{
    SwapChain NewSwapChain = {};
    NewSwapChain.m_WindowSurface = SurfaceKHR;

    NewSwapChain.m_SwapChainFormat = {
        vk::Format(nvrhi::vulkan::convertFormat(Params.swapChainFormat)),
        vk::ColorSpaceKHR::eSrgbNonlinear
    };

    vk::Extent2D extent = vk::Extent2D(Params.backBufferWidth, Params.backBufferHeight);

    eastl::unordered_set<uint32_t> uniqueQueues = {
        uint32_t(m_GraphicsQueueFamily),
        uint32_t(m_PresentQueueFamily) };
    
    eastl::vector<uint32_t> queues = setToVector(uniqueQueues);

    const bool enableSwapChainSharing = queues.size() > 1;

    auto desc = vk::SwapchainCreateInfoKHR()
        .setSurface(NewSwapChain.m_WindowSurface)
        .setMinImageCount(Params.swapChainBufferCount)
        .setImageFormat(NewSwapChain.m_SwapChainFormat.format)
        .setImageColorSpace(NewSwapChain.m_SwapChainFormat.colorSpace)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .setImageSharingMode(enableSwapChainSharing ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive)
        .setQueueFamilyIndexCount(enableSwapChainSharing ? uint32_t(queues.size()) : 0)
        .setPQueueFamilyIndices(enableSwapChainSharing ? queues.data() : nullptr)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(Params.vsyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate)
        .setClipped(true)
        .setOldSwapchain(nullptr);

    const vk::Result res = m_VulkanDevice.createSwapchainKHR(&desc, nullptr, &NewSwapChain.m_SwapChain);
    if (res != vk::Result::eSuccess)
    {
        printf("Failed to create a Vulkan swap chain, error code = %s\n", nvrhi::vulkan::resultToString(res));
        return false;
    }

    // retrieve swap chain images
    auto images = m_VulkanDevice.getSwapchainImagesKHR(NewSwapChain.m_SwapChain);
    for(auto image : images)
    {
        SwapChainImage sci;
        sci.image = image;
        
        nvrhi::TextureDesc textureDesc;
        textureDesc.width = Params.backBufferWidth;
        textureDesc.height = Params.backBufferHeight;
        textureDesc.format = Params.swapChainFormat;
        textureDesc.debugName = "Swap chain image";
        textureDesc.initialState = nvrhi::ResourceStates::Present;
        textureDesc.keepInitialState = true;
        textureDesc.isRenderTarget = true;

        sci.rhiHandle = m_NvrhiDevice->createHandleForNativeTexture(
            nvrhi::ObjectTypes::VK_Image, nvrhi::Object(sci.image), textureDesc);
        NewSwapChain.m_SwapChainImages.push_back(sci);
    }
    NewSwapChain.m_SwapChainIndex = 0;
    m_SwapChains[m_SwapChainCount] = NewSwapChain;
    m_SwapChainCount += 1;
    return m_SwapChainCount - 1;
}

bool RenderDeviceVK::pickPhysicalDevice(vk::SurfaceKHR surface)
{
    vk::Format requestedFormat = nvrhi::vulkan::convertFormat(m_DeviceParams.swapChainParams.swapChainFormat);
    vk::Extent2D requestedExtent(m_DeviceParams.swapChainParams.backBufferWidth, m_DeviceParams.swapChainParams.backBufferHeight);

    auto devices = m_VulkanInstance.enumeratePhysicalDevices();

    // build a list of GPUs
    eastl::vector<vk::PhysicalDevice> discreteGPUs;
    eastl::vector<vk::PhysicalDevice> otherGPUs;
    for(const auto& dev : devices)
    {
        auto prop = dev.getProperties();

        // check that all required device extensions are present
        eastl::unordered_set<eastl::string> requiredExtensions = enabledExtensions.device;
        auto deviceExtensions = dev.enumerateDeviceExtensionProperties();
        for(const auto& ext : deviceExtensions)
        {
            requiredExtensions.erase(eastl::string(ext.extensionName.data()));
        }

        if (!requiredExtensions.empty())
        {
            // device is missing one or more required extensions
            continue;
        }

        auto deviceFeatures = dev.getFeatures();
        if (!deviceFeatures.samplerAnisotropy)
        {
            // device is a toaster oven
            continue;
        }
        if (!deviceFeatures.textureCompressionBC)
        {
            // uh-oh
            continue;
        }

        // check that this device supports our intended swap chain creation parameters
        auto surfaceCaps = dev.getSurfaceCapabilitiesKHR(surface);
        auto surfaceFmts = dev.getSurfaceFormatsKHR(surface);
        auto surfacePModes = dev.getSurfacePresentModesKHR(surface);

        if (surfaceCaps.minImageCount > m_DeviceParams.swapChainParams.swapChainBufferCount ||
            surfaceCaps.maxImageCount < m_DeviceParams.swapChainParams.swapChainBufferCount ||
            surfaceCaps.minImageExtent.width > requestedExtent.width ||
            surfaceCaps.minImageExtent.height > requestedExtent.height ||
            surfaceCaps.maxImageExtent.width < requestedExtent.width ||
            surfaceCaps.maxImageExtent.height < requestedExtent.height)
        {
            // swap chain parameters don't match device capabilities
            continue;
        }

        bool surfaceFormatPresent = false;
        for (const vk::SurfaceFormatKHR& surfaceFmt : surfaceFmts)
        {
            if (surfaceFmt.format == requestedFormat)
            {
                surfaceFormatPresent = true;
                break;
            }
        }

        if (!surfaceFormatPresent)
        {
            // can't create a swap chain using the format requested
            continue;
        }

        if (!findQueueFamilies(dev))
        {
            // device doesn't have all the queue families we need
            continue;
        }

        // check that we can present from the graphics queue
        uint32_t canPresent = dev.getSurfaceSupportKHR(m_GraphicsQueueFamily, surface);
        if (!canPresent)
        {
            continue;
        }

        if (prop.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            discreteGPUs.push_back(dev);
        } else {
            otherGPUs.push_back(dev);
        }
    }

    // pick the first discrete GPU if it exists, otherwise the first integrated GPU
    if (!discreteGPUs.empty())
    {
        m_VulkanPhysicalDevice = discreteGPUs[0];
        return true;
    }

    if (!otherGPUs.empty())
    {
        m_VulkanPhysicalDevice = otherGPUs[0];
        return true;
    }

    return false;
}

bool RenderDeviceVK::findQueueFamilies(vk::PhysicalDevice physicalDevice)
{
    auto props = physicalDevice.getQueueFamilyProperties();

    for(int i = 0; i < int(props.size()); i++)
    {
        const auto& queueFamily = props[i];

        if (m_GraphicsQueueFamily == -1)
        {
            if (queueFamily.queueCount > 0 &&
                (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
            {
                m_GraphicsQueueFamily = i;
            }
        }

        if (m_ComputeQueueFamily == -1)
        {
            if (queueFamily.queueCount > 0 &&
                (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
                !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
            {
                m_ComputeQueueFamily = i;
            }
        }

        if (m_TransferQueueFamily == -1)
        {
            if (queueFamily.queueCount > 0 &&
                (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && 
                !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
                !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
            {
                m_TransferQueueFamily = i;
            }
        }

        if (m_PresentQueueFamily == -1)
        {
            if (queueFamily.queueCount > 0)
            {
                m_PresentQueueFamily = i;
            }
        }
    }

    if (m_GraphicsQueueFamily == -1 || 
        m_PresentQueueFamily == -1 ||
        (m_ComputeQueueFamily == -1 && m_DeviceParams.enableComputeQueue) || 
        (m_TransferQueueFamily == -1 && m_DeviceParams.enableCopyQueue))
    {
        return false;
    }
    return true;
}

bool RenderDeviceVK::createDevice()
{
    // figure out which optional extensions are supported
    auto deviceExtensions = m_VulkanPhysicalDevice.enumerateDeviceExtensionProperties();
    for(const auto& ext : deviceExtensions)
    {
        const eastl::string name = ext.extensionName.data();
        if (optionalExtensions.device.find(name) != optionalExtensions.device.end())
        {
            enabledExtensions.device.insert(name);
        }

        if (m_DeviceParams.enableRayTracingExtensions 
            && m_RayTracingExtensions.find(name) != m_RayTracingExtensions.end())
        {
            enabledExtensions.device.insert(name);
        }
    }

    bool accelStructSupported = false;
    bool bufferAddressSupported = false;
    bool rayPipelineSupported = false;
    bool rayQuerySupported = false;
    bool meshletsSupported = false;
    bool vrsSupported = false;

    printf("Enabled Vulkan device extensions:\n");
    for (const auto& ext : enabledExtensions.device)
    {
        printf("    %s\n", ext.c_str());

        if (ext == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
            accelStructSupported = true;
        else if (ext == VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
            bufferAddressSupported = true;
        else if (ext == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
            rayPipelineSupported = true;
        else if (ext == VK_KHR_RAY_QUERY_EXTENSION_NAME)
            rayQuerySupported = true;
        else if (ext == VK_NV_MESH_SHADER_EXTENSION_NAME)
            meshletsSupported = true;
        else if (ext == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            vrsSupported = true;
    }

    eastl::unordered_set<int> uniqueQueueFamilies = {
        m_GraphicsQueueFamily,
        m_PresentQueueFamily };

    if (m_DeviceParams.enableComputeQueue)
        uniqueQueueFamilies.insert(m_ComputeQueueFamily);

    if (m_DeviceParams.enableCopyQueue)
        uniqueQueueFamilies.insert(m_TransferQueueFamily);

    float priority = 1.f;
    eastl::vector<vk::DeviceQueueCreateInfo> queueDesc;
    for(int queueFamily : uniqueQueueFamilies)
    {
        queueDesc.push_back(vk::DeviceQueueCreateInfo()
                                .setQueueFamilyIndex(queueFamily)
                                .setQueueCount(1)
                                .setPQueuePriorities(&priority));
    }

    auto accelStructFeatures = vk::PhysicalDeviceAccelerationStructureFeaturesKHR()
        .setAccelerationStructure(true);
    auto bufferAddressFeatures = vk::PhysicalDeviceBufferAddressFeaturesEXT()
        .setBufferDeviceAddress(true);
    auto rayPipelineFeatures = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR()
        .setRayTracingPipeline(true)
        .setRayTraversalPrimitiveCulling(true);
    auto rayQueryFeatures = vk::PhysicalDeviceRayQueryFeaturesKHR()
        .setRayQuery(true);
    auto meshletFeatures = vk::PhysicalDeviceMeshShaderFeaturesNV()
        .setTaskShader(true)
        .setMeshShader(true);
    auto vrsFeatures = vk::PhysicalDeviceFragmentShadingRateFeaturesKHR()
        .setPipelineFragmentShadingRate(true)
        .setPrimitiveFragmentShadingRate(true)
        .setAttachmentFragmentShadingRate(true);

    void* pNext = nullptr;
#define APPEND_EXTENSION(condition, desc) if (condition) { (desc).pNext = pNext; pNext = &(desc); }  // NOLINT(cppcoreguidelines-macro-usage)
    APPEND_EXTENSION(accelStructSupported, accelStructFeatures)
    APPEND_EXTENSION(bufferAddressSupported, bufferAddressFeatures)
    APPEND_EXTENSION(rayPipelineSupported, rayPipelineFeatures)
    APPEND_EXTENSION(rayQuerySupported, rayQueryFeatures)
    APPEND_EXTENSION(meshletsSupported, meshletFeatures)
    APPEND_EXTENSION(vrsSupported, vrsFeatures)
#undef APPEND_EXTENSION

    auto deviceFeatures = vk::PhysicalDeviceFeatures()
        .setShaderImageGatherExtended(true)
        .setSamplerAnisotropy(true)
    #ifndef __APPLE__
        .setShaderFloat64(true)
        .setGeometryShader(true)
    #endif
        .setTextureCompressionBC(true)
        .setDualSrcBlend(true)
        .setTessellationShader(true)
        .setImageCubeArray(true);

    auto vulkan12features = vk::PhysicalDeviceVulkan12Features()
        .setDescriptorIndexing(true)
        .setRuntimeDescriptorArray(true)
        .setDescriptorBindingPartiallyBound(true)
        .setDescriptorBindingVariableDescriptorCount(true)
        .setTimelineSemaphore(true)
        .setShaderSampledImageArrayNonUniformIndexing(true)
        .setPNext(pNext);

    auto layerVec = stringSetToVector(enabledExtensions.layers);
    auto extVec = stringSetToVector(enabledExtensions.device);

    auto deviceDesc = vk::DeviceCreateInfo()
        .setPQueueCreateInfos(queueDesc.data())
        .setQueueCreateInfoCount(uint32_t(queueDesc.size()))
        .setPEnabledFeatures(&deviceFeatures)
        .setEnabledExtensionCount(uint32_t(extVec.size()))
        .setPpEnabledExtensionNames(extVec.data())
        .setEnabledLayerCount(uint32_t(layerVec.size()))
        .setPpEnabledLayerNames(layerVec.data())
        .setPNext(&vulkan12features);
    
    const vk::Result res = m_VulkanPhysicalDevice.createDevice(&deviceDesc, nullptr, &m_VulkanDevice);
    if (res != vk::Result::eSuccess)
    {
        printf("Failed to create a Vulkan physical device, error code = %s\n", nvrhi::vulkan::resultToString(res));
        return false;
    }

    m_VulkanDevice.getQueue(m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
    if (m_DeviceParams.enableComputeQueue)
        m_VulkanDevice.getQueue(m_ComputeQueueFamily, 0, &m_ComputeQueue);
    if (m_DeviceParams.enableCopyQueue)
        m_VulkanDevice.getQueue(m_TransferQueueFamily, 0, &m_TransferQueue);
    m_VulkanDevice.getQueue(m_PresentQueueFamily, 0, &m_PresentQueue);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_VulkanDevice);

    // stash the renderer string
    auto prop = m_VulkanPhysicalDevice.getProperties();
    m_DeviceName = eastl::string(prop.deviceName.data());

    printf("Created Vulkan device: %s\n", m_DeviceName.c_str());

    return true;
}

Render::RenderDevice* Render::RenderDevice::CreateVulkan(
    const PlatformWindow window, const struct Render::DeviceCreationParameters& params)
{   
    return new RenderDeviceVK(window, params);
}
#endif