using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Lemon
{
    static Lemon()
    {
        BuildSystem.Package("lemon")
            .AddTarget("lemon", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 3, 1))
                    throw new TaskFatalError("lemon version mismatch!", "lemon version mismatch, only v1.3.1 is supported in source.");

                Target.CppVersion("20")
                    .Exception(true)
                    .TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port/lemon")
                    .AddCppFiles(
                        "port/lemon/lemon/arg_parser.cc",
                        "port/lemon/lemon/base.cc",
                        "port/lemon/lemon/color.cc",
                        "port/lemon/lemon/lp_base.cc",
                        "port/lemon/lemon/lp_skeleton.cc",
                        "port/lemon/lemon/random.cc",
                        "port/lemon/lemon/bits/windows.cc"
                    );

                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    Target.Defines(Visibility.Private, "WIN32")
                        .Link(Visibility.Private, "ole32");
                }
                else
                {
                    Target.Link(Visibility.Private, "pthread");
                }
            });
    }
}