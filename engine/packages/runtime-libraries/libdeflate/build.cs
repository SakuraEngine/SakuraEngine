using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class LibDeflate
{
    static LibDeflate()
    {
        BuildSystem.Package("LibDeflate")
            .AddTarget("LibDeflate", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 24, 0))
                    throw new TaskFatalError("LibDeflate version mismatch!", "LibDeflate version mismatch, only v1.24.0 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .CVersion("11")
                    .OptimizationLevel(OptimizationLevel.Fastest)
                    .IncludeDirs(Visibility.Public, "./1.24.0/libdeflate")
                    .AddCFiles("./1.24.0/**.c");

                if (Engine.TargetArch == Architecture.ARM64)
                    Target.SIMD(SIMDArchitecture.Neon);
                else
                    Target.SIMD(SIMDArchitecture.AVX);
            });
    }
}