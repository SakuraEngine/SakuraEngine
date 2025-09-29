using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class RenderGraphSamples
{
    static RenderGraphSamples()
    {
        Engine.Program("RenderGraphTriangleSample")
            .Depend(Visibility.Public, "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("rg-triangle/*.cpp")
            .AddHLSLFiles("rg-triangle/**.hlsl")
            .DXCOutputDirectory("resources/shaders/rg-triangle");

        Engine.Program("RenderGraphDeferredSample")
            .Depend(Visibility.Public, "SkrImGui", "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("rg-deferred/*.cpp")
            .AddCppSLFiles("rg-deferred/**.cxx")
            .CppSLOutputDirectory("resources/shaders/rg-deferred");

        Engine.Program("RenderGraphSkyboxSample")
            .Depend(Visibility.Public, "SkrImGui", "SkrRenderGraph")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../common")
            .AddCppFiles("rg-skybox/*.cpp")
            .AddCppSLFiles("rg-skybox/**.cxx")
            .CppSLOutputDirectory("resources/shaders/rg-skybox");


        Engine.Program("RenderGraphRaytracingSample")
            .Depend(Visibility.Public, "SkrRenderGraph", "SkrSystem", "SkrActors")
            .Depend(Visibility.Private, "lodepng")
            .AddCppFiles("rg-raytracing/*.cpp")
            .AddCppSLFiles("rg-raytracing/**.cxx")
            .CppSLOutputDirectory("resources/shaders/rg-raytracing");

        Engine.Program("RenderGraphCrossProcessSample")
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