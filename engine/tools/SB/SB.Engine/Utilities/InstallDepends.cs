using SB.Core;

namespace SB;

public static class InstallDepends
{
    private static DependDatabase? _Download;
    private static DependDatabase? _SDK;
    [AfterStage(EBuildStage.SetupConfigure)]
    private static void _SetupDepends()
    {
        _Download ??= new DependDatabase(BuildDirs.TempDir, "Engine.Downloads");
        _SDK ??= new DependDatabase(BuildDirs.TempDir, $"Engine.SDKs.{Engine.GlobalConfiguration}");
    }

    public static DependDatabase Download => _Download ?? throw new InvalidOperationException("InstallDepends not initialized");
    public static DependDatabase SDK => _SDK ?? throw new InvalidOperationException("InstallDepends not initialized");
}