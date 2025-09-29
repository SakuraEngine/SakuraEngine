using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrScene
{
    static SkrScene()
    {
        Engine.Module("SkrScene", "SKR_SCENE")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/*.cpp")
            .AddMetaHeaders("include/SkrScene/**.hpp");
            
    }
}