using SB.Core;

namespace SB;

public static class BuildDepends
{
    // depends
    private static DependDatabase? _PkgCompile;
    private static DependDatabase? _TargetCompile;
    [AfterStage(EBuildStage.LoadedToolChain)]
    private static void _SetupDepends()
    {
        _PkgCompile ??= new DependDatabase(BuildDirs.PackageBuildDir, $"CppCompile.Paks.{BuildSystem.GlobalConfiguration}");
        _TargetCompile ??= new DependDatabase(BuildDirs.BuildDir, $"CppCompile.Targets.{BuildSystem.GlobalConfiguration}");
    }

    // getters
    public static DependDatabase PkgCompile => _PkgCompile ?? throw new InvalidOperationException("BuildDepends not initialized");
    public static DependDatabase TargetCompile => _TargetCompile ?? throw new InvalidOperationException("BuildDepends not initialized");

    // solve helper
    public static DependDatabase Solve(bool isPackage) => isPackage ? PkgCompile : TargetCompile;
    public static DependDatabase Solve(Target target) => Solve(target.IsFromPackage);
}