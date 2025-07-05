using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class CGltf
{
    static CGltf()
    {
        BuildSystem.Package("cgltf")
            .AddTarget("cgltf", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 13, 0))
                    throw new TaskFatalError("cgltf version mismatch!", "cgltf version mismatch, only v1.13.0 is supported in source.");

                var @this = Target.TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port/cgltf/include")
                    .FpModel(FpModel.Fast)
                    .AddCFiles("port/cgltf/src/cgltf.c");

                if (BuildSystem.TargetOS == OSPlatform.Windows)
                    @this.Defines(Visibility.Private, "_CRT_SECURE_NO_WARNINGS");
            });
    }
}