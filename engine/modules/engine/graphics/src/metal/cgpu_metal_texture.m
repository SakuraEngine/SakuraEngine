#include "../common/common_utils.h"
#include "SkrGraphics/backend/metal/cgpu_metal.h"
#include "SkrGraphics/backend/metal/cgpu_metal_types.h"
#include "SkrGraphics/flags.h"
#include "metal_utils.h"
#import <Metal/Metal.h>

// Texture usage flags mapping
static MTLTextureUsage MetalUtil_TranslateTextureUsage(CGPUTextureUsages descriptors)
{
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    
    if (descriptors & CGPU_TEXTURE_USAGE_SHADER_READWRITE)
    {
        usage |= MTLTextureUsageShaderWrite;
        usage |= MTLTextureUsagePixelFormatView;
    }
    
    if (descriptors & CGPU_TEXTURE_USAGE_RENDER_TARGET)
    {
        usage |= MTLTextureUsageRenderTarget;
    }
    
    if ((descriptors & CGPU_TEXTURE_USAGE_DEPTH_STENCIL))
    {
        usage |= MTLTextureUsageRenderTarget;
        usage &= ~MTLTextureUsageShaderWrite;
    }
    
    return usage;
}

// Determine texture dimension
static ECGPUTextureDimension MetalUtil_CalculateTextureDimension(const CGPUTextureDescriptor* desc)
{
    ECGPUTextureDimension dim = CGPU_TEXTURE_DIMENSION_2D;
    
    if (desc->flags & CGPU_TEXTURE_FLAG_FORCE_2D)
    {
        dim = CGPU_TEXTURE_DIMENSION_2D;
    }
    else if (desc->flags & CGPU_TEXTURE_FLAG_FORCE_3D)
    {
        dim = CGPU_TEXTURE_DIMENSION_3D;
    }
    else if (desc->depth > 1)
    {
        dim = CGPU_TEXTURE_DIMENSION_3D;
    }
    else if (desc->height == 1)
    {
        dim = CGPU_TEXTURE_DIMENSION_1D;
    }
    
    return dim;
}

CGPUTextureId cgpu_create_texture_metal(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc)
{
    cgpu_assert(device && desc && "Invalid parameters for texture creation");
    cgpu_assert(desc->width && desc->height && (desc->depth || desc->array_size) && "Invalid texture dimensions");
    
    CGPUDevice_Metal* mtlDevice = (CGPUDevice_Metal*)device;
    
    // Multi-sampled textures cannot have mip maps
    if (desc->sample_count > CGPU_SAMPLE_COUNT_1 && desc->mip_levels > 1)
    {
        cgpu_assert(false && "Multi-sampled textures cannot have mip maps");
        return NULL;
    }
    
    // Allocate texture with info structure
    const size_t totalSize = sizeof(CGPUTexture_Metal) + sizeof(CGPUTextureInfo);
    CGPUTexture_Metal* texture = (CGPUTexture_Metal*)cgpu_calloc_aligned(1, totalSize, _Alignof(CGPUTexture_Metal));
    if (!texture)
    {
        cgpu_assert(false && "Failed to allocate memory for texture");
        return NULL;
    }
    
    // Setup info structure
    CGPUTextureInfo* info = (CGPUTextureInfo*)(texture + 1);
    texture->super.info = info;
    texture->super.device = device;
    
    // Fill texture info
    info->unique_id = UINT64_MAX; // Not a shared texture by default
    info->width = desc->width;
    info->height = desc->height;
    info->depth = desc->depth;
    info->array_size = desc->array_size;
    info->mip_levels = desc->mip_levels;
    info->sample_count = desc->sample_count;
    info->format = desc->format;
    info->aspect_mask = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
    info->node_index = CGPU_SINGLE_GPU_NODE_INDEX;
    info->owns_image = true;
    info->is_cube = (CGPU_TEXTURE_USAGE_CUBEMAP == (desc->usages & CGPU_TEXTURE_USAGE_CUBEMAP)) ? 1 : 0;
    info->is_aliasing = (desc->flags & CGPU_TEXTURE_FLAG_ALIASING_RESOURCE) != 0;
    info->is_tiled = (desc->flags & CGPU_TEXTURE_FLAG_TILED_RESOURCE) != 0;
    info->is_imported = desc->native_handle != NULL;
    info->can_alias = false;
    info->can_export = (desc->flags & CGPU_TEXTURE_FLAG_EXPORT_BIT) != 0;
    
    // Set aspect mask based on format
    if (FormatUtil_IsDepthStencilFormat(desc->format))
    {
        if (FormatUtil_IsDepthOnlyFormat(desc->format))
            info->aspect_mask = CGPU_TEXTURE_VIEW_ASPECTS_DEPTH;
        else
            info->aspect_mask = CGPU_TEXTURE_VIEW_ASPECTS_DEPTH | CGPU_TEXTURE_VIEW_ASPECTS_STENCIL;
    }
    
    @autoreleasepool {
        // Handle native texture import
        if (desc->native_handle)
        {
            texture->pTexture = (__bridge id<MTLTexture>)desc->native_handle;
            info->owns_image = false;
            
            // Query texture properties from native handle
            if (texture->pTexture)
            {
                info->width = texture->pTexture.width;
                info->height = texture->pTexture.height;
                info->depth = texture->pTexture.depth;
                info->array_size = texture->pTexture.arrayLength;
                info->mip_levels = texture->pTexture.mipmapLevelCount;
                info->sample_count = (ECGPUSampleCount)texture->pTexture.sampleCount;
            }
        }
        else
        {
            // Create texture descriptor
            MTLTextureDescriptor* textureDesc = [[MTLTextureDescriptor alloc] init];
            
            // Set basic properties
            ECGPUTextureDimension dimension = MetalUtil_CalculateTextureDimension(desc);
            textureDesc.textureType = MetalUtil_TranslateTextureType(dimension, desc->array_size, info->is_cube);
            textureDesc.pixelFormat = MetalUtil_TranslatePixelFormat(desc->format);
            textureDesc.width = desc->width;
            textureDesc.height = desc->height;
            textureDesc.depth = (dimension == CGPU_TEXTURE_DIMENSION_3D) ? desc->depth : 1;
            textureDesc.mipmapLevelCount = desc->mip_levels;
            textureDesc.arrayLength = (dimension != CGPU_TEXTURE_DIMENSION_3D) ? desc->array_size : 1;
            textureDesc.sampleCount = desc->sample_count;
            
            // Set usage flags
            textureDesc.usage = MetalUtil_TranslateTextureUsage(desc->usages);
            
            // Set storage mode
            textureDesc.storageMode = MTLStorageModePrivate;
            // Metal doesn't have a direct host visible flag for textures
            // This would typically be handled through shared/managed storage modes
            // For now, we'll keep textures in private storage mode
            
            // Set CPU cache mode for managed textures
            if (textureDesc.storageMode == MTLStorageModeManaged)
            {
                textureDesc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
            }
            
            // Create texture
            if (desc->flags & CGPU_TEXTURE_FLAG_TILED_RESOURCE)
            {
                // Sparse textures are not yet supported
                cgpu_assert(false && "Tiled/sparse textures not yet implemented");
                cgpu_free_aligned(texture, _Alignof(CGPUTexture_Metal));
                return NULL;
            }
            else
            {
                texture->pTexture = [mtlDevice->pDevice newTextureWithDescriptor:textureDesc];
                if (!texture->pTexture)
                {
                    cgpu_assert(false && "Failed to create Metal texture");
                    cgpu_free_aligned(texture, _Alignof(CGPUTexture_Metal));
                    return NULL;
                }
                
                if (desc->name) {
                    NSString* textureName = [NSString stringWithUTF8String:(const char*)desc->name];
                    texture->pTexture.label = textureName;
                }
            }
            
            // Set debug name if provided
            if (device->adapter->instance->enable_set_name && desc->name)
            {
                texture->pTexture.label = [NSString stringWithUTF8String:(const char*)desc->name];
            }
        }
    }
    
    return &texture->super;
}

void cgpu_free_texture_metal(CGPUTextureId texture)
{
    if (!texture) return;
    
    CGPUTexture_Metal* mtlTexture = (CGPUTexture_Metal*)texture;
    
    // Release texture if we own it
    if (texture->info->owns_image)
    {
        mtlTexture->pTexture = nil;
    }
    
    cgpu_free_aligned(mtlTexture, _Alignof(CGPUTexture_Metal));
}

CGPUTextureViewId cgpu_create_texture_view_metal(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    cgpu_assert(device && desc && desc->texture && "Invalid parameters for texture view creation");
    
    CGPUTexture_Metal* texture = (CGPUTexture_Metal*)desc->texture;
    
    // Allocate texture view
    CGPUTextureView_Metal* view = (CGPUTextureView_Metal*)cgpu_calloc_aligned(1, sizeof(CGPUTextureView_Metal), _Alignof(CGPUTextureView_Metal));
    if (!view)
    {
        cgpu_assert(false && "Failed to allocate memory for texture view");
        return NULL;
    }
    view->pTextureView = MetalUtil_CreateTextureView(device, desc);
    return &view->super;
}

void cgpu_free_texture_view_metal(CGPUTextureViewId view)
{
    if (!view) return;
    
    CGPUTextureView_Metal* mtlView = (CGPUTextureView_Metal*)view;
    mtlView->pTextureView = nil;
    
    cgpu_free_aligned(mtlView, _Alignof(CGPUTextureView_Metal));
}