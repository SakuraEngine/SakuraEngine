using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class OpenEXR
{
    static OpenEXR()
    {
        BuildSystem.Package("OpenEXR")
            .AddTarget("OpenEXR", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(3, 3, 5))
                    throw new TaskFatalError("OpenEXR version mismatch!", "OpenEXR version mismatch, only v3.3.5 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .CppVersion("20")

                    .Require("LibDeflate", new PackageConfig { Version = new Version(1, 24, 0) })
                    .Depend(Visibility.Private, "LibDeflate@LibDeflate")

                    .RTTI(true)
                    .Exception(true)
                    .OptimizationLevel(OptimizationLevel.Fastest)

                    // Instead of #include <IexConfigInternal.h>
                    // .Defines(Visibility.Public, "HAVE_UCONTEXT_H")
                    // .Defines(Visibility.Public, "IEX_HAVE_CONTROL_REGISTER_SUPPORT")
                    // .Defines(Visibility.Public, "IEX_HAVE_SIGCONTEXT_CONTROL_REGISTER_SUPPORT")

                    // Instead of #include <OpenEXRConfigInternal.h>
                    // .Defines(Visibility.Public, "OPENEXR_IMF_HAVE_LINUX_PROCFS")
                    // .Defines(Visibility.Public, "OPENEXR_IMF_HAVE_DARWIN")
                    // .Defines(Visibility.Public, "OPENEXR_IMF_HAVE_COMPLETE_IOMANIP")
                    // .Defines(Visibility.Public, "OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN")
                    // .Defines(Visibility.Public, "OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX")
                    // .Defines(Visibility.Public, "OPENEXR_MISSING_ARM_VLD1")

                    .IncludeDirs(Visibility.Public, "./3.3.5/OpenEXR")
                    .IncludeDirs(Visibility.Public, "./3.3.5/OpenEXR/OpenEXRCore")
                    .IncludeDirs(Visibility.Public, "./3.3.5/OpenEXR/Iex")
                    .IncludeDirs(Visibility.Public, "./3.3.5/OpenEXR/Imath")
                    .IncludeDirs(Visibility.Private, "./3.3.5/OpenEXR/IlmThread")
                    .IncludeDirs(Visibility.Private, "./3.3.5/OpenEXR/OpenEXR")
                    .IncludeDirs(Visibility.Private, "./3.3.5/OpenEXR/OpenEXRUtil")

                    .AddCFiles("./3.3.5/OpenEXR/**.c")
                    .AddCppFiles("./3.3.5/OpenEXR/**.cpp")

                    .Clang_CXFlags(Visibility.Private,
                        "-Wno-deprecated-declarations",
                        "-Wno-parentheses",
                        "-Wno-pointer-sign"
                    );

                if (Engine.TargetArch == Architecture.ARM64)
                    Target.SIMD(SIMDArchitecture.Neon);
                else
                    Target.SIMD(SIMDArchitecture.AVX);
            });
    }
}