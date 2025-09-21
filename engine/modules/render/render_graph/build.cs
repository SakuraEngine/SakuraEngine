using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrRenderGraph
{
    static SkrRenderGraph()
    {
        Engine.Module("SkrRenderGraph")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/*.cpp", "src/frontend/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "backend" }, "src/backend/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "backend" }, "src/phases_v2/*.cpp");
    }
}