using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrLive2DViewer
{
    static SkrLive2DViewer()
    {
        Engine.Program("Live2DViewer", "LIVE2D_VIEWER")
            .Depend(Visibility.Public, "SkrLive2D", "SkrImGui")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common", "include")
            .AddCppFiles("src/main.cpp", "src/viewer_module.cpp");
    }
}