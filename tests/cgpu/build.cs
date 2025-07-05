using SB;
using SB.Core;
using System.Runtime.CompilerServices;

[TargetScript]
public static class CGPUTests
{
    static CGPUTests()
    {
        /*
        if (false)
        {
            var CGPUTests = Engine.UnitTest("CGPUTests")
                .EnableUnityBuild()
                .Depend(Visibility.Public, "SkrRT")

                .AddCppFiles(
                    "Common.cpp",
                    // Device Initialize
                    "DeviceInitialize/**.cpp",
                    // Device Extensions
                    "Exts/**.cpp",
                    // QueueOps
                    "QueueOperations/QueueOperations.cpp",
                    // RSPool
                    "RootSignaturePool/RootSignaturePool.cpp",
                    // ResourceCreation
                    "ResourceCreation/ResourceCreation.cpp",
                    // SwapChainCreation
                    "SwapChainCreation/SwapChainCreation.cpp"
                )
                .AddHLSLFiles(
                    "RootSignaturePool/shaders/**.hlsl"
                )

                .DXCOutputDirectory("resources/shaders/cgpu-rspool-test");

            if (BuildSystem.TargetOS == OSPlatform.Windows)
            {
                CGPUTests.Link(Visibility.Private, "User32");
            }
        }
        */
    }
}