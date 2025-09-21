using SB;
using SB.Core;

[TargetScript]
public static class GraphicsTests
{
    static GraphicsTests()
    {
        // CGPU Memory Pool Test
        Engine.Program("CGPUMemoryPoolTest")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")           // 包含 SkrGraphics/CGPU
            .AddCppFiles("memory_pool_test.cpp");
    }
}