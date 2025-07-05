using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class ICU
{
    static ICU()
    {
        BuildSystem.Package("icu")
            .AddTarget("icu", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(72, 1, 0))
                    throw new TaskFatalError("icu version mismatch!", "icu version mismatch, only v72.1.0 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .OptimizationLevel(OptimizationLevel.Fastest)
                    .RTTI(true)
                    .CppVersion("17") // Compiles much more faster with C++17 than C++20
                    .IncludeDirs(Visibility.Public, "port/icu4c/source/common")
                    .IncludeDirs(Visibility.Public, "port/icu4c/source/i18n")
                    .Defines(Visibility.Private, "U_I18N_IMPLEMENTATION")
                    .Defines(Visibility.Private, "U_COMMON_IMPLEMENTATION")
                    .Defines(Visibility.Private, "U_STATIC_IMPLEMENTATION")
                    .AddCppFiles(
                        "port/icu4c/source/i18n/**.cpp",
                        "port/icu4c/source/common/**.cpp",
                        "port/icu4c/source/stubdata/**.cpp"
                    );
                    
                    if (BuildSystem.TargetOS == OSPlatform.Windows)
                    {
                        Target.CXFlags(Visibility.Private, "/wd4804", "/wd4805", "/wd4267", "/wd4244", "/utf-8")
                            .ClangCl_CXFlags(Visibility.Private, "-Wno-deprecated-declarations")
                            .Defines(Visibility.Private, "_CRT_SECURE_NO_WARNINGS");
                    }
            });
    }
}