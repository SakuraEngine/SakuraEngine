#include "metal_utils.h"
#include "cgpu/api.h"
#include "string.h"
#import "cgpu/backend/metal/cgpu_metal_types.h"

static const uint32_t kAMDVendorId = 0x1002;
static const uint32_t kAppleVendorId = 0x106b;
static const uint32_t kIntelVendorId = 0x8086;
static const uint32_t kNVVendorId = 0x10de;

static const uint32_t kAMDRadeonRX5700DeviceId = 0x731f;
static const uint32_t kAMDRadeonRX5500DeviceId = 0x7340;
static const uint32_t kAMDRadeonRX6800DeviceId = 0x73bf;
static const uint32_t kAMDRadeonRX6700DeviceId = 0x73df;

void MetalUtil_QueryVendorIdAndDeviceId(id<MTLDevice> device, uint32_t* outVenderId, uint32_t* outDeviceId);

void MetalUtil_RecordAdapterDetail(struct CGpuAdapter_Metal* MAdapter)
{
    MAdapter->adapter_detail.is_cpu = false;
    MAdapter->adapter_detail.is_uma = isMTLDeviceUMA(MAdapter->device.pDevice);
    MAdapter->adapter_detail.is_virtual = false;

    GPUVendorPreset* vendor = &MAdapter->adapter_detail.vendor_preset;
    const char* mDeviceName = [MAdapter->device.pDevice.name UTF8String];
    strncpy(vendor->gpu_name, mDeviceName, MAX_GPU_VENDOR_STRING_LENGTH);
    MetalUtil_QueryVendorIdAndDeviceId(MAdapter->device.pDevice, &vendor->vendor_id, &vendor->device_id);
}

#ifdef TARGET_MACOS
uint64_t mtlGetRegistryID(id<MTLDevice> mtlDevice)
{
    return [mtlDevice respondsToSelector:@selector(registryID)] ? mtlDevice.registryID : 0;
}

static uint32_t mtlGetEntryProperty(io_registry_entry_t entry, CFStringRef propertyName)
{
    uint32_t value = 0;
    CFTypeRef cfProp = IORegistryEntrySearchCFProperty(entry,
        kIOServicePlane,
        propertyName,
        kCFAllocatorDefault,
        kIORegistryIterateRecursively |
            kIORegistryIterateParents);
    if (cfProp)
    {
        const uint32_t* pValue = (uint32_t*)CFDataGetBytePtr((CFDataRef)cfProp);
        if (pValue) { value = *pValue; }
        CFRelease(cfProp);
    }
    return value;
}

void MetalUtil_QueryVendorIdAndDeviceId(id<MTLDevice> device, uint32_t* outVenderId, uint32_t* outDeviceId)
{
    bool isFound = false;
    bool isIntegrated = isMTLDeviceUMA(device);
    uint32 vendorID = 0;
    uint32 deviceID = 0;
    #if __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
    const mach_port_t IOPort = kIOMainPortDefault;
    #elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 100000
    const mach_port_t IOPort = kIOMasterPortDefault;
    #else

    #endif
    if (supportsMTLGPUFamily(device, Apple5))
    {
        // This is an Apple GPU. It won't have a 'device-id' property, so fill it in
        // like on iOS/tvOS.
        vendorID = kAppleVendorId;
    #ifdef TARGET_MACOS_APPLE_SILICON
        if (supportsMTLGPUFamily(device, Apple7))
        {
            deviceID = 0xa140;
        }
        else if (supportsMTLGPUFamily(device, Apple6))
        {
            deviceID = 0xa130;
        }
        else
        {
            deviceID = 0xa120;
        }
    #else
        deviceID = 0xa120;
    #endif
    }
    // If the device has an associated registry ID, we can use that to get the associated IOKit node.
    // The match dictionary is consumed by IOServiceGetMatchingServices and does not need to be released.
    io_registry_entry_t entry;
    uint64_t regID = mtlGetRegistryID(device);
    if (regID)
    {
        entry = IOServiceGetMatchingService(IOPort, IORegistryEntryIDMatching(regID));
        if (entry)
        {
            // That returned the IOGraphicsAccelerator nub. Its parent, then, is the actual PCI device.
            io_registry_entry_t parent;
            if (IORegistryEntryGetParentEntry(entry, kIOServicePlane, &parent) == kIOReturnSuccess)
            {
                isFound = true;
                vendorID = mtlGetEntryProperty(parent, CFSTR("vendor-id"));
                deviceID = mtlGetEntryProperty(parent, CFSTR("device-id"));
                IOObjectRelease(parent);
            }
            IOObjectRelease(entry);
        }
    }
    // Iterate all GPU's, looking for a match.
    // The match dictionary is consumed by IOServiceGetMatchingServices and does not need to be released.
    io_iterator_t entryIterator;
    if (!isFound && IOServiceGetMatchingServices(IOPort,
                        IOServiceMatching("IOPCIDevice"),
                        &entryIterator) == kIOReturnSuccess)
    {
        while (!isFound && (entry = IOIteratorNext(entryIterator)))
        {
            if (mtlGetEntryProperty(entry, CFSTR("class-code")) == 0x30000)
            { // 0x30000 : DISPLAY_VGA

                // The Intel GPU will always be marked as integrated.
                // Return on a match of either Intel && low power, or non-Intel and non-low-power.
                uint32_t _vendorID = mtlGetEntryProperty(entry, CFSTR("vendor-id"));
                if ((_vendorID == kIntelVendorId) == isIntegrated)
                {
                    isFound = true;
                    vendorID = _vendorID;
                    deviceID = mtlGetEntryProperty(entry, CFSTR("device-id"));
                }
            }
        }
        IOObjectRelease(entryIterator);
    }
    *outVenderId = vendorID;
    *outDeviceId = deviceID;
}
#endif