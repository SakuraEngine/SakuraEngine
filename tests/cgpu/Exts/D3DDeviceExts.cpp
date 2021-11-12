#define RUNTIME_DLL

#include "cgpu/api.h"
#ifdef CGPU_USE_D3D12
#include "cgpu/extensions/cgpu_d3d12_exts.h"
#include "gtest/gtest.h"
#include <vector>

class D3D12DeviceExtsTest : public testing::Test 
{
protected:
  static void SetUpTestCase() {
    
  }
  static void TearDownTestCase() {

  }

};

TEST_F(D3D12DeviceExtsTest, EnableAndDisableDRED)
{
    auto DRED = cgpu_d3d12_enable_DRED();
    EXPECT_NE(DRED, CGPU_NULLPTR);
    EXPECT_NE(DRED, nullptr);
    cgpu_d3d12_disable_DRED(DRED);
}
#else
int main() {return 0;}
#endif
