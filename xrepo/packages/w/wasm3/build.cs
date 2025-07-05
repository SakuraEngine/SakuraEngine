using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Wasm3
{
    static Wasm3()
    {
        BuildSystem
            .Package("wasm3")
            .AddTarget("wasm3", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 5, 0))
                    throw new TaskFatalError("wasm3 version mismatch!", "wasm3 version mismatch, only v0.5.0 is supported in source.");

                Target
                    .CVersion("11")
                    .TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port/wasm3/source")
                    .FpModel(FpModel.Fast)
                    .AddCFiles("port/wasm3/source/**.c");
            });
    }
}