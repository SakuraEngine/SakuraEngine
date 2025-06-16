using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class RenderGraphSamples
{
    static RenderGraphSamples()
    {
        Engine.Program("RenderGraphTriangle")
            .Depend(Visibility.Public, "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("rg-triangle/*.cpp")
            .AddHLSLFiles("rg-triangle/**.hlsl")
            .DXCOutputDirectory("resources/shaders/rg-triangle");

        Engine.Program("RenderGraphDeferred")
            .Depend(Visibility.Public, "SkrImGui", "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("rg-deferred/*.cpp")
            .AddHLSLFiles("rg-deferred/**.hlsl")
            .DXCOutputDirectory("resources/shaders/rg-deferred");

        Engine.Program("RenderGraphCrossProcess")
            .Require("lmdb", new PackageConfig { Version = new(0, 9, 29) })
            .Depend(Visibility.Private, "lmdb@lmdb")
            .Depend(Visibility.Public, "SkrImGui", "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("cross-process/*.cpp")
            .AddHLSLFiles("cross-process/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cross-process");
    }
}