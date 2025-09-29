using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrActors
{
    static SkrActors()
    {
        Engine.Module("SkrActors", "SKR_ACTORS")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRenderer")
            .Depend(Visibility.Public, "SkrAnim")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/*.cpp")
            .AddMetaHeaders("include/SkrActors/**.hpp");
    }
}