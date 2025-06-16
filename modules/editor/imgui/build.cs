using SB;
using SB.Core;

[TargetScript]
public static class SkrImGui
{
    static SkrImGui()
    {
        Engine.Module("SkrImGui", "SKR_IMGUI")
            .Depend(Visibility.Private, "SDL3")
            .Depend(Visibility.Public, "SkrInput", "SkrRenderGraph")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/**.cpp")
            .AddHLSLFiles("shaders/*.hlsl")
            .IncludeDirs(Visibility.Public, "imgui/include")
            .IncludeDirs(Visibility.Private, "imgui/src")
            .AddCppFiles("imgui/src/**.cpp")
            .Defines(Visibility.Public, "IMGUI_DEFINE_MATH_OPERATORS");
    }
}