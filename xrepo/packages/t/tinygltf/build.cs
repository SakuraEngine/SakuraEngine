using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class TinyGltf
{
    static TinyGltf()
    {
        BuildSystem.Package("tinygltf")
            .AddTarget("tinygltf", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(2, 8, 14))
                    throw new TaskFatalError("tinygltf version mismatch!", "tinygltf version mismatch, only v2.8.14 is supported in source.");

                Target.CppVersion("20")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port")
                    .AddCppFiles("port/tinygltf/**.cc");
            });
    }
}