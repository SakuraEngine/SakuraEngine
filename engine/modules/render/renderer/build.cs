using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrRenderer
{
    static SkrRenderer()
    {
        Engine.Module("SkrRenderer")
            .Depend(Visibility.Public, "SkrSystem", "SkrScene", "SkrRenderGraph")
            .IncludeDirs(Visibility.Public, "include")
            .EnableUnityBuild()
            .AddCppFiles("src/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "graphics" }, "src/graphics/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "resources" }, "src/resources/*.cpp")
            .AddCppSLFiles("shaders/**.cxx")
            .CppSLOutputDirectory("resources/shaders")
            .CppSL_IncludeDirs(Visibility.Public, "include", "shaders")
            .AddMetaHeaders("include/**.h", "include/**.hpp") // codegen
            .AddCodegenScript("meta/gpu_data_block.ts");
    }
}