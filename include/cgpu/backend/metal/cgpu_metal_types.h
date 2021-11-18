#pragma once
#ifndef __OBJC__
    #include <assert.h>
static_assert(0, "This Header Should Only Be Included By OBJC SOURCES!!!!!");
#endif
#import <MetalKit/MetalKit.h>

// Types
typedef struct CGpuAdapter_Metal {
    CGpuAdapter super;
    id<MTLDevice> pDevice;
} CGpuAdapter_Metal;

typedef struct CGpuInstance_Metal {
    CGpuInstance super;
    CGpuAdapter_Metal adapter;
} CGpuInstance_Metal;