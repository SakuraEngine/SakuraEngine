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
    CGPU_TEXTURE_DIMENSION_1D,        // MTLTextureType1D
    CGPU_TEXTURE_DIMENSION_UNDEFINED, // MTLTextureType1DArray
    CGPU_TEXTURE_DIMENSION_2D,        // MTLTextureType2D
    CGPU_TEXTURE_DIMENSION_UNDEFINED, // MTLTextureType2DArray
    CGPU_TEXTURE_DIMENSION_2DMS,      // MTLTextureType2DMultisample
    CGPU_TEXTURE_DIMENSION_CUBE,      // MTLTextureTypeCube
    CGPU_TEXTURE_DIMENSION_UNDEFINED, // MTLTextureTypeCubeArray
    CGPU_TEXTURE_DIMENSION_3D,        // MTLTextureType3D
    CGPU_TEXTURE_DIMENSION_UNDEFINED, // MTLTextureType2DMultisampleArray
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
    case CGPU_TEXTURE_DIMENSION_1D:
        return MTLTextureType1D;
    case CGPU_TEXTURE_DIMENSION_2D:
        return MTLTextureType2D;
    case CGPU_TEXTURE_DIMENSION_3D:
        return MTLTextureType3D;
    case CGPU_TEXTURE_DIMENSION_CUBE:
        return MTLTextureTypeCube;
    case CGPU_TEXTURE_DIMENSION_2DMS:
        return MTLTextureType2DMultisample;
    default:
        SKR_ASSERT(false && "Unsupported texture dimension");
        return MTLTextureType1D; // Default fallback
    }
}

typedef struct ShaderResourceView
{
    ECGPUResourceType type;
    CGPUViewUsages usage;
} ShaderResourceView;

ShaderResourceView MetalUtil_GetResourceType(MTLStructType* structure, ECGPUTextureDimension* dim)
{
    ECGPUResourceType t = CGPU_RESOURCE_TYPE2_NONE;
    CGPUViewUsages u = CGPU_VIEW_USAGE_NONE;

    if (structure.members.count == 1)
    {
        MTLDataType T = structure.members[0].dataType;
        if (T == MTLDataTypeTexture)
        {
            t = CGPU_RESOURCE_TYPE2_TEXTURE;

            MTLTextureReferenceType* TexType = structure.members[0].textureReferenceType;
            u = (TexType.access == MTLBindingAccessReadOnly) ? CGPU_TEXTURE_VIEW_USAGE_SRV : CGPU_TEXTURE_VIEW_USAGE_UAV;
            *dim = gTexDimLUT[TexType.textureType];
            SKR_ASSERT(*dim != CGPU_TEXTURE_DIMENSION_UNDEFINED);
        }
        else if (T == MTLDataTypeSampler)
        {
            t = CGPU_RESOURCE_TYPE2_SAMPLER;
        }
        else if (T == MTLDataTypePointer)
        {
            t = CGPU_RESOURCE_TYPE2_BUFFER;
            u = CGPU_BUFFER_VIEW_USAGE_CBV;
        }
        else if (T == MTLDataTypeInstanceAccelerationStructure)
        {
            t = CGPU_RESOURCE_TYPE2_ACCELERATION_STRUCTURE;
        }
        else if (T == MTLDataTypeStruct)
        {
            t = CGPU_RESOURCE_TYPE2_BUFFER;
            u = CGPU_BUFFER_VIEW_USAGE_PUSH_CONSTANT;
        }
        SKR_ASSERT(t != CGPU_RESOURCE_TYPE2_NONE);
    }
    else if (structure.members.count == 2)
    {
        MTLDataType T = structure.members[0].dataType;
        if (T == MTLDataTypePointer) // RW/RO Buffer
        {
            MTLBindingAccess Access = structure.members[0].pointerType.access;
            t = CGPU_RESOURCE_TYPE2_BUFFER;
            u = (Access == MTLBindingAccessReadOnly) ? CGPU_BUFFER_VIEW_USAGE_SRV_RAW : CGPU_BUFFER_VIEW_USAGE_UAV_RAW;
        }
        else if (T == MTLDataTypeTexture) // Texel Buffer
        {
            MTLBindingAccess Access = structure.members[0].pointerType.access;
            t = CGPU_RESOURCE_TYPE2_BUFFER;
            u = (Access == MTLBindingAccessReadOnly) ? CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL : CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL;
        }
        SKR_ASSERT(t != CGPU_RESOURCE_TYPE2_NONE);
    }
    ShaderResourceView r;
    r.type = t;
    r.usage = u;
    SKR_ASSERT(r.type != CGPU_RESOURCE_TYPE2_NONE);
    return r;
}

ECGPUResourceType MetalUtil_GetShaderResourceType(id<MTLBufferBinding> SRT, uint32_t set, const MTLStructMember* member, CGPUShaderResource* resource)
{
    MTLStructType* structure = member.structType;
    MTLArrayType* arrayType = member.arrayType;
    MTLPointerType* pointerType = member.pointerType;
    const bool is_array = (arrayType != nil) || (pointerType != nil);
    ECGPUResourceType resource_type = CGPU_RESOURCE_TYPE2_NONE;
    resource->dim = CGPU_TEXTURE_DIMENSION_UNDEFINED;
    resource->set = set;
    resource->binding = member.argumentIndex;
    resource->name = MetalUtil_DuplicateString(member.name.UTF8String);
    resource->name_hash = skr_hash_of(resource->name, member.name.length, SKR_DEFAULT_HASH_SEED);
    resource->offset = member.offset;

    if (!is_array)
    {
        ShaderResourceView v = MetalUtil_GetResourceType(structure, &resource->dim);
        resource_type = v.type;
        resource->view_usages = v.usage;
        if (resource->view_usages == CGPU_BUFFER_VIEW_USAGE_PUSH_CONSTANT)
            resource->size = SRT.bufferDataSize;
        else
            resource->size = 1;
    }
    else 
    {
        MTLStructType* elementStructType = arrayType ? arrayType.elementStructType : pointerType.elementStructType;
        resource->size = arrayType ? arrayType.arrayLength : ~0;
        ShaderResourceView v = MetalUtil_GetResourceType(elementStructType, &resource->dim);
        resource_type = v.type;
        resource->view_usages = v.usage;
    }
    resource->type = resource_type;
    SKR_ASSERT(resource->type != CGPU_RESOURCE_TYPE2_NONE);
    return resource_type;
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

// Blit pipeline and sampler utilities
static const char* blitVertexShader = 
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"\n"
"struct VertexOut {\n"
"    float4 position [[position]];\n"
"    float2 texCoord;\n"
"};\n"
"\n"
"vertex VertexOut blitVertex(uint vertexID [[vertex_id]]) {\n"
"    VertexOut out;\n"
"    \n"
"    // Generate a fullscreen triangle\n"
"    float2 pos;\n"
"    if (vertexID == 0) {\n"
"        pos = float2(-1.0, -1.0);\n"
"    } else if (vertexID == 1) {\n"
"        pos = float2(3.0, -1.0);\n"
"    } else {\n"
"        pos = float2(-1.0, 3.0);\n"
"    }\n"
"    \n"
"    out.position = float4(pos, 0.0, 1.0);\n"
"    out.texCoord = (pos + 1.0) * 0.5;\n"
"    out.texCoord.y = 1.0 - out.texCoord.y; // Flip Y coordinate\n"
"    \n"
"    return out;\n"
"}\n";

static const char* blitFragmentShader = 
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"\n"
"struct VertexOut {\n"
"    float4 position [[position]];\n"
"    float2 texCoord;\n"
"};\n"
"\n"
"fragment float4 blitFragment(VertexOut in [[stage_in]],\n"
"                            texture2d<float> sourceTexture [[texture(0)]],\n"
"                            sampler linearSampler [[sampler(0)]]) {\n"
"    return sourceTexture.sample(linearSampler, in.texCoord);\n"
"}\n";

id<MTLRenderPipelineState> MetalUtil_GetBlitPipeline(CGPUDevice_Metal* device, MTLPixelFormat format)
{
    static id<MTLRenderPipelineState> blitPipeline = nil;
    static MTLPixelFormat cachedFormat = MTLPixelFormatInvalid;
    
    if (blitPipeline && cachedFormat == format) {
        return blitPipeline;
    }
    
    @autoreleasepool {
        NSError* error = nil;
        
        // Compile shaders
        MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
        id<MTLLibrary> library = [device->pDevice newLibraryWithSource:[NSString stringWithUTF8String:blitVertexShader]
                                                              options:options
                                                                error:&error];
        if (error) {
            cgpu_error("Failed to compile blit vertex shader: %s", error.localizedDescription.UTF8String);
            return nil;
        }
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"blitVertex"];
        
        library = [device->pDevice newLibraryWithSource:[NSString stringWithUTF8String:blitFragmentShader]
                                               options:options
                                                 error:&error];
        if (error) {
            cgpu_error("Failed to compile blit fragment shader: %s", error.localizedDescription.UTF8String);
            return nil;
        }
        
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"blitFragment"];
        
        // Create pipeline descriptor
        MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDesc.vertexFunction = vertexFunction;
        pipelineDesc.fragmentFunction = fragmentFunction;
        pipelineDesc.colorAttachments[0].pixelFormat = format;
        
        // Create pipeline state
        blitPipeline = [device->pDevice newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
        if (error) {
            cgpu_error("Failed to create blit pipeline state: %s", error.localizedDescription.UTF8String);
            return nil;
        }
        
        cachedFormat = format;
    }
    
    return blitPipeline;
}

id<MTLSamplerState> MetalUtil_GetLinearSampler(CGPUDevice_Metal* device)
{
    static id<MTLSamplerState> linearSampler = nil;
    
    if (linearSampler) {
        return linearSampler;
    }
    
    MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
    samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    linearSampler = [device->pDevice newSamplerStateWithDescriptor:samplerDesc];
    
    return linearSampler;
}

id<MTLTexture> MetalUtil_CreateTextureView(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    CGPUTexture_Metal* texture = (CGPUTexture_Metal*)desc->texture;
    id<MTLTexture> view = nil;
    @autoreleasepool {
        // Determine view parameters
        ECGPUFormat viewFormat = desc->format != CGPU_FORMAT_UNDEFINED ? desc->format : texture->super.info->format;
        MTLPixelFormat mtlFormat = MetalUtil_TranslatePixelFormat(viewFormat);
        
        // Calculate mip and array ranges
        uint32_t baseMip = desc->base_mip_level;
        uint32_t mipCount = desc->mip_level_count ? desc->mip_level_count : (texture->super.info->mip_levels - baseMip);
        uint32_t baseLayer = desc->base_array_layer;
        uint32_t layerCount = desc->array_layer_count ? desc->array_layer_count : (texture->super.info->array_size - baseLayer);
        
        // Validate ranges
        cgpu_assert(baseMip < texture->super.info->mip_levels && "Base mip level out of range");
        cgpu_assert(baseMip + mipCount <= texture->super.info->mip_levels && "Mip range out of bounds");
        cgpu_assert(baseLayer < texture->super.info->array_size && "Base array layer out of range");
        cgpu_assert(baseLayer + layerCount <= texture->super.info->array_size && "Array range out of bounds");
        
        // Create texture view
        NSRange levelRange = NSMakeRange(baseMip, mipCount);
        NSRange sliceRange = NSMakeRange(baseLayer, layerCount);
        
        // Handle swizzle if needed (Metal doesn't support arbitrary swizzle, only specific patterns)
        MTLTextureSwizzleChannels swizzle = MTLTextureSwizzleChannelsDefault;
        
        // Check if we need a different view type
        MTLTextureType viewType = texture->pTexture.textureType;
        if (desc->dims != CGPU_TEXTURE_DIMENSION_UNDEFINED)
        {
            viewType = MetalUtil_TranslateTextureType(desc->dims, layerCount, false);
        }
        
        if (@available(macOS 10.15, iOS 13.0, *))
        {
            view = [texture->pTexture newTextureViewWithPixelFormat:mtlFormat
                                                                      textureType:viewType
                                                                           levels:levelRange
                                                                           slices:sliceRange
                                                                          swizzle:swizzle];
        }
        else
        {
            view = [texture->pTexture newTextureViewWithPixelFormat:mtlFormat
                                                                      textureType:viewType
                                                                           levels:levelRange
                                                                           slices:sliceRange];
        }
        
        if (!view)
        {
            cgpu_assert(false && "Failed to create Metal texture view");
            cgpu_free_aligned(view, _Alignof(CGPUTextureView_Metal));
            return NULL;
        }
        
        // Set debug name if provided
        if (device->adapter->instance->enable_set_name && desc->name)
        {
            view.label = [NSString stringWithUTF8String:(const char*)desc->name];
        }
    }
    return view;
}