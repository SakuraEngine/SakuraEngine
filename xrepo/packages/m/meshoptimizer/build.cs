using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class MeshOptimizer
{
    static MeshOptimizer()
    {
        BuildSystem.Package("MeshOptimizer")
            .AddTarget("MeshOptimizer", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 1, 0))
                    throw new TaskFatalError("MeshOptimizer version mismatch!", "MeshOptimizer version mismatch, only v0.1.0 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .CppVersion("20")
                    .IncludeDirs(Visibility.Public, "port")
                    .IncludeDirs(Visibility.Private, "port/MeshOpt")
                    .AddCppFiles("port/MeshOpt/*.cpp");
            });
    }
}