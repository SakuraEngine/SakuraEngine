using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class YYJson
{
    static YYJson()
    {
        BuildSystem.Package("yyjson")
            .AddTarget("yyjson", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 9, 0))
                    throw new TaskFatalError("YYJson version mismatch!", "YYJson version mismatch, only v0.9.0 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Private, "port/yyjson")
                    .IncludeDirs(Visibility.Public, "port")
                    .FpModel(FpModel.Fast)
                    .AddCFiles("port/yyjson/**.c");
            });
    }
}