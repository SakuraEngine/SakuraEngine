using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class NanoVg
{
    static NanoVg()
    {
        BuildSystem.Package("nanovg")
            .AddTarget("nanovg", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 1, 0))
                    throw new TaskFatalError("nanovg version mismatch!", "nanovg version mismatch, only v0.1.0 is supported in source.");

                Target.CppVersion("20")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port/nanovg")
                    .AddCppFiles("port/nanovg/nanovg.cpp");
            });
    }
}