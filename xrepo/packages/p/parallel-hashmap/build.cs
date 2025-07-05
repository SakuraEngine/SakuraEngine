using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class ParallelHashMap
{
    static ParallelHashMap()
    {
        BuildSystem
            .Package("phmap")
            .AddTarget("phmap", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 3, 11))
                    throw new TaskFatalError("phmap version mismatch!", "phmap version mismatch, only v1.3.11 is supported in source.");

                Target
                    .CppVersion("17")
                    .TargetType(TargetType.HeaderOnly)
                    .IncludeDirs(Visibility.Public, "port");
            });
    }
}