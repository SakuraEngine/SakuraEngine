using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrSceneCore
{
    static SkrSceneCore()
    {
        Engine.Module("SkrSceneCore", "SKR_SCENE_CORE")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/*.cpp")
            .AddMetaHeaders("include/SkrSceneCore/**.h");
            
    }
}