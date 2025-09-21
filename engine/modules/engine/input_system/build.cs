using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrInputSystem
{
    static SkrInputSystem()
    {
        Engine
            .Module("SkrInputSystem")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Public, "SkrSystem")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/*.cpp");
    }
}