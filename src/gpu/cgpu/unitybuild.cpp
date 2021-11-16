#include "cgpu/cgpu_config.h"
#ifdef CGPU_USE_VULKAN
#include "vulkan/cgpu_vulkan_instance.cpp"
#endif
#ifdef CGPU_USE_D3D12
#include "d3d12/cgpu_d3d12_surfaces.cpp"
#include "d3d12/cgpu_d3d12.cpp"
#endif