#include "SkrGraphics/api.h"
#ifdef CGPU_USE_D3D12
    #include "SkrGraphics/extensions/cgpu_d3d12_exts.h"
    #include "SkrTestFramework/framework.hpp"
    #include <vector>

class D3D12DeviceExtsTest
{

};

TEST_CASE_METHOD(D3D12DeviceExtsTest, "EnableAndDisableDRED")
{
    auto DRED = cgpu_d3d12_enable_DRED();
    EXPECT_NE(DRED, CGPU_NULLPTR);
    EXPECT_NE(DRED, nullptr);
    cgpu_d3d12_disable_DRED(DRED);
}
#endif
