using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrGuiRenderer
{
    static SkrGuiRenderer()
    {
        Engine.Module("SkrGuiRenderer")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrGui", "SkrRenderGraph", "SkrImageCoder")
            .Depend(Visibility.Private, "AppSampleCommon")
            .Defines(Visibility.Private, "SKR_GUI_RENDERER_USE_IMAGE_CODER")
            .IncludeDirs(Visibility.Public, "include")
            .IncludeDirs(Visibility.Private, "src")
            .AddMetaHeaders("include/**.hpp")
            .AddCppFiles("src/**.cpp")
            .AddHLSLFiles("shaders/*.hlsl")
            .DXCOutputDirectory("resources/shaders/GUI");
    }
}