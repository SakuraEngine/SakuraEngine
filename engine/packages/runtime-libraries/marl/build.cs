using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Marl
{
    static Marl()
    {
        BuildSystem.Package("Marl")
            .AddTarget("Marl", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 0, 0))
                    throw new TaskFatalError("marl version mismatch!", "marl version mismatch, only v1.0.0 is supported in source.");

                Target.CppVersion("20")
                    .Exception(false)
                    .TargetType(TargetType.Objects)
                    .Depend(Visibility.Public, "SkrCore")
                    .IncludeDirs(Visibility.Public, "include")
                    .AddCppFiles("src/build.*.cpp")
                    .Clang_CXFlags(Visibility.Private, "-Wno-deprecated-declarations");

                if (BuildSystem.TargetOS != OSPlatform.Windows)
                {
                    Target.AddCFiles("src/**.c", "src/**.S");
                }

                if (!Engine.ShippingOneArchive)
                {
                    Target.TargetType(TargetType.Dynamic)
                        .Defines(Visibility.Public, "MARL_DLL")
                        .Defines(Visibility.Private, "MARL_BUILDING_DLL");
                }
                else
                {
                    Target.TargetType(TargetType.Objects);
                }
            });
    }
}