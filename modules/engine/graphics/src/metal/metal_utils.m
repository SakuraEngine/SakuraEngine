#include "metal_utils.h"
#include "SkrGraphics/api.h"
#include "SkrGraphics/backend/metal/cgpu_metal_types.h"

static const uint32_t kAMDVendorId = 0x1002;
static const uint32_t kAppleVendorId = 0x106b;
static const uint32_t kIntelVendorId = 0x8086;
static const uint32_t kNVVendorId = 0x10de;

static const uint32_t kAMDRadeonRX5700DeviceId = 0x731f;
static const uint32_t kAMDRadeonRX5500DeviceId = 0x7340;
static const uint32_t kAMDRadeonRX6800DeviceId = 0x73bf;
static const uint32_t kAMDRadeonRX6700DeviceId = 0x73df;

void MetalUtil_QueryVendorIdAndDeviceId(id<MTLDevice> device, uint32_t* outVenderId, uint32_t* outDeviceId);

void MetalUtil_RecordAdapterDetail(struct CGPUAdapter_Metal* MAdapter)
{
    MAdapter->adapter_detail.is_cpu = false;
    MAdapter->adapter_detail.is_uma = isMTLDeviceUMA(MAdapter->device.pDevice);
    MAdapter->adapter_detail.is_virtual = false;
    MAdapter->adapter_detail.uniform_buffer_alignment = 256;

    CGPUVendorPreset* vendor = &MAdapter->adapter_detail.vendor_preset;
    const char* mDeviceName = [MAdapter->device.pDevice.name UTF8String];
    strncpy(vendor->gpu_name, mDeviceName, MAX_GPU_VENDOR_STRING_LENGTH);
    MetalUtil_QueryVendorIdAndDeviceId(MAdapter->device.pDevice, &vendor->vendor_id, &vendor->device_id);
}

NSArray<id<MTLDevice>>* MetalUtil_GetAvailableMTLDeviceArray()
{
    NSMutableArray* mtlDevs = [NSMutableArray array];
#ifndef TARGET_IOS
    NSArray* rawMTLDevs = MTLCopyAllDevices();
    if (rawMTLDevs)
    {
        const bool forceLowPower = false;

        // Populate the array of appropriate MTLDevices
        for (id<MTLDevice> md in rawMTLDevs)
        {
            if (!forceLowPower || md.isLowPower) { [mtlDevs addObject:md]; }
        }

        // Sort by power
        [mtlDevs sortUsingComparator:^(id<MTLDevice> md1, id<MTLDevice> md2) {
            BOOL md1IsLP = md1.isLowPower;
            BOOL md2IsLP = md2.isLowPower;

            if (md1IsLP == md2IsLP)
            {
                // If one device is headless and the other one is not, select the
                // one that is not headless first.
                BOOL md1IsHeadless = md1.isHeadless;
                BOOL md2IsHeadless = md2.isHeadless;
                if (md1IsHeadless == md2IsHeadless)
                {
                    return NSOrderedSame;
                }
                return md2IsHeadless ? NSOrderedAscending : NSOrderedDescending;
            }

            return md2IsLP ? NSOrderedAscending : NSOrderedDescending;
        }];
    }
#else  // _IOS_OR_TVOS
    id<MTLDevice> md = [MTLCreateSystemDefaultDevice() autorelease];
    if (md) { [mtlDevs addObject:md]; }
#endif // TARGET_IOS

    return mtlDevs; // retained
}

static const ECGPUTextureDimension gTexDimLUT[] = {
    CGPU_TEX_DIMENSION_1D,        // MTLTextureType1D
    CGPU_TEX_DIMENSION_UNDEFINED, // MTLTextureType1DArray
    CGPU_TEX_DIMENSION_2D,        // MTLTextureType2D
    CGPU_TEX_DIMENSION_UNDEFINED, // MTLTextureType2DArray
    CGPU_TEX_DIMENSION_2DMS,      // MTLTextureType2DMultisample
    CGPU_TEX_DIMENSION_CUBE,      // MTLTextureTypeCube
    CGPU_TEX_DIMENSION_UNDEFINED, // MTLTextureTypeCubeArray
    CGPU_TEX_DIMENSION_3D,        // MTLTextureType3D
    CGPU_TEX_DIMENSION_UNDEFINED, // MTLTextureType2DMultisampleArray
};

static const MTLResourceOptions gResourceOptionsLUT[VK_MAX_MEMORY_TYPES] = {
    MTLResourceStorageModePrivate,
    MTLResourceStorageModePrivate,
    MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined,
    MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache,
};

MTLResourceOptions MetalUtil_MemoryTypeToResourceOptions(MemoryType usage)
{
    return gResourceOptionsLUT[usage];
}

MTLTextureType MetalUtil_TextureDimensionToType(ECGPUTextureDimension dim)
{
    switch (dim)
    {
    case CGPU_TEX_DIMENSION_1D:
        return MTLTextureType1D;
    case CGPU_TEX_DIMENSION_2D:
        return MTLTextureType2D;
    case CGPU_TEX_DIMENSION_3D:
        return MTLTextureType3D;
    case CGPU_TEX_DIMENSION_CUBE:
        return MTLTextureTypeCube;
    case CGPU_TEX_DIMENSION_2DMS:
        return MTLTextureType2DMultisample;
    default:
        SKR_ASSERT(false && "Unsupported texture dimension");
        return MTLTextureType1D; // Default fallback
    }
}

MTLBindingAccess MetalUtil_ResourceTypeToAccess(ECGPUResourceType type)
{
    switch (type)
    {
    case CGPU_RESOURCE_TYPE_BUFFER:
    case CGPU_RESOURCE_TYPE_TEXTURE:
    case CGPU_RESOURCE_TYPE_SAMPLER:
    case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER:
        return MTLBindingAccessReadOnly;

    case CGPU_RESOURCE_TYPE_RW_BUFFER:
    case CGPU_RESOURCE_TYPE_INDIRECT_BUFFER:
    case CGPU_RESOURCE_TYPE_RW_TEXTURE:
        return MTLBindingAccessReadWrite;
        
    default:
        SKR_ASSERT(false && "Unsupported resource type");
        return MTLBindingAccessReadOnly;
    }
}

ECGPUResourceType MetalUtil_GetResourceType(MTLStructType* structure, ECGPUTextureDimension* dim)
{
    ECGPUResourceType r = CGPU_RESOURCE_TYPE_NONE;
    if (structure.members.count == 1)
    {
        MTLDataType T = structure.members[0].dataType;
        if (T == MTLDataTypeTexture)
        {
            MTLTextureReferenceType* TexType = structure.members[0].textureReferenceType;
            r = (TexType.access == MTLBindingAccessReadOnly) ? CGPU_RESOURCE_TYPE_TEXTURE : CGPU_RESOURCE_TYPE_RW_TEXTURE;
            *dim = gTexDimLUT[TexType.textureType];
            SKR_ASSERT(*dim != CGPU_TEX_DIMENSION_UNDEFINED);
        }
        else if (T == MTLDataTypeSampler)
            r = CGPU_RESOURCE_TYPE_SAMPLER;
        else if (T == MTLDataTypePointer)
            r = CGPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        else if (T == MTLDataTypeInstanceAccelerationStructure)
            r = CGPU_RESOURCE_TYPE_ACCELERATION_STRUCTURE;
    }
    else if (structure.members.count == 2)
    {
        MTLDataType T = structure.members[0].dataType;
        if (T == MTLDataTypePointer) // RW/RO Buffer
        {
            MTLBindingAccess Access = structure.members[0].pointerType.access;
            r = (Access == MTLBindingAccessReadOnly) ? CGPU_RESOURCE_TYPE_BUFFER : CGPU_RESOURCE_TYPE_RW_BUFFER;
        }
    }
    return r;
}

void MetalUtil_GetShaderResourceType(uint32_t set, const MTLStructMember* member, CGPUShaderResource* resource)
{
    MTLStructType* structure = member.structType;
    MTLArrayType* arrayType = member.arrayType;
    MTLPointerType* pointerType = member.pointerType;
    const bool is_array = (arrayType != nil) || (pointerType != nil);
    ECGPUResourceType resource_type = CGPU_RESOURCE_TYPE_NONE;
    resource->dim = CGPU_TEX_DIMENSION_UNDEFINED;
    resource->set = set;
    resource->binding = member.argumentIndex;
    resource->name = MetalUtil_DuplicateString(member.name.UTF8String);
    resource->name_hash = cgpu_name_hash(resource->name, member.name.length);
    resource->offset = member.offset;

    if (!is_array)
    {
        resource_type = MetalUtil_GetResourceType(structure, &resource->dim);
        resource->size = 1;
    }
    else 
    {
        MTLStructType* elementStructType = arrayType ? arrayType.elementStructType : pointerType.elementStructType;
        resource_type = MetalUtil_GetResourceType(elementStructType, &resource->dim);
        resource->size = arrayType ? arrayType.arrayLength : ~0;
    }
    resource->type = resource_type;
    SKR_ASSERT(resource->type != CGPU_RESOURCE_TYPE_NONE);
}

MemoryType MetalUtil_MemoryUsageToMemoryType(ECGPUMemoryUsage usage)
{
    switch (usage)
    {
    case CGPU_MEM_USAGE_GPU_ONLY:
        return MEMORY_TYPE_GPU_ONLY;
    case CGPU_MEM_USAGE_CPU_TO_GPU:
        return MEMORY_TYPE_CPU_TO_GPU;
    case CGPU_MEM_USAGE_GPU_TO_CPU:
        return MEMORY_TYPE_GPU_TO_CPU;
    case CGPU_MEM_USAGE_CPU_ONLY:
        return MEMORY_TYPE_CPU_TO_GPU;
    default:
        SKR_ASSERT(false && "Unsupported memory usage");
        return MEMORY_TYPE_GPU_ONLY;
    }
}

bool MetalUtil_DSHasBindAtIndex(const CGPUDescriptorSet_Metal* ds, uint32_t binding_index, uint32_t* out_index)
{
    if (ds->mtlBindSlots == CGPU_NULLPTR || ds->mtlBindSlotCount == 0)
        return false;
    for (uint32_t i = 0; i < ds->mtlBindSlotCount; i++)
    {
        if (ds->mtlBindSlots[i].binding_index == binding_index)
        {
            *out_index = i;
            return true;
        }
    }
    return false;
}

bool MetalUtil_DSBindResourceAtIndex(CGPUDescriptorSet_Metal* ds, uint32_t binding_index, __unsafe_unretained id<MTLResource> resource, MTLResourceUsage usage)
{
    uint32_t existed = 0;
    if (MetalUtil_DSHasBindAtIndex(ds, binding_index, &existed))
    {
        ds->mtlBindSlots[existed].mtlResource = resource;
        ds->mtlBindSlots[existed].mtlUsage = usage;
        return true;
    }
    else
    {
        // Add new bind slot
        if (ds->mtlBindSlotCount < ds->super.root_signature->tables[ds->super.index].resources_count)
        {
            ds->mtlBindSlots[ds->mtlBindSlotCount].binding_index = binding_index;
            ds->mtlBindSlots[ds->mtlBindSlotCount].mtlResource = resource;
            ds->mtlBindSlots[ds->mtlBindSlotCount].mtlUsage = usage;
            ds->mtlBindSlotCount++;
            return true;
        }
        else
        {
            SKR_ASSERT(false && "Expected Error: Metal descriptor set bind slots overflow");
        }
    }
    return false;
}

char8_t* MetalUtil_DuplicateString(const char8_t* src_string)
{
    if (src_string != CGPU_NULLPTR)
    {
        const size_t source_len = strlen((const char*)src_string);
        char8_t* result = (char8_t*)cgpu_malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
        strcpy_s((char*)result, source_len + 1, (const char*)src_string);
#else
        strcpy((char*)result, (const char*)src_string);
#endif
        return result;
    }
    return CGPU_NULLPTR;
}

void MetalUtil_FlushUtilEncoders(CGPUCommandBuffer_Metal* CMD, MTLUtilEncoderTypes types)
{
    if (CMD->UtilEncoders.mtlBlitEncoder && (types & MTLUtilEncoderTypeBlit))
    {
        [CMD->UtilEncoders.mtlBlitEncoder endEncoding];
        CMD->UtilEncoders.mtlBlitEncoder = nil;
    }

    if (CMD->UtilEncoders.mtlASCommandEncoder && (types & MTLUtilEncoderTypeAS))
    {
        [CMD->UtilEncoders.mtlASCommandEncoder endEncoding];
        CMD->UtilEncoders.mtlASCommandEncoder = nil;
    }
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