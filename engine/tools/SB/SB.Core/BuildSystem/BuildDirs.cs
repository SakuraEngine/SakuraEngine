
using System.Runtime.InteropServices.Marshalling;
using SB.Core;
using Serilog;

namespace SB;

public static class BuildDirs
{
    // setup
    private static string? _EngineDir;
    private static string? _ProjectRoot;
    public static void SetupPaths(string EngineDir, string ProjectRoot)
    {
        // setup
        _EngineDir = EngineDir;
        _ProjectRoot = ProjectRoot;

        // check exists
        if (!Directory.Exists(EngineDir))
            throw new DirectoryNotFoundException($"EngineDir '{EngineDir}' not found");
        if (!Directory.Exists(ProjectRoot))
            throw new DirectoryNotFoundException($"ProjectRoot '{ProjectRoot}' not found");

        _FlushPathCache();

        // call stage callback
        BuildStage.UpdateStage(EBuildStage.SetupPaths);
    }

    // setup toolchain, used for build path
    public static void SetupToolchain(IToolchain toolchain)
    {
        _CachedBuildDir = Directory.CreateDirectory(Path.Combine(ProjectRoot, "build/.build", toolchain.Name)).FullName;
        _CachedPackageBuildDir = Directory.CreateDirectory(Path.Combine(ProjectRoot, "build/.pkgs", toolchain.Name)).FullName;

        // call stage callback
        BuildStage.UpdateStage(EBuildStage.LoadedToolChain);
    }

    // dump paths
    [AfterStage(EBuildStage.PrepareCommandline)]
    private static void _DumpPaths()
    {
        Log.Information("EngineDir: {EngineDir}", EngineDir);
        Log.Information("ProjectRoot: {ProjectRoot}", ProjectRoot);
    }

    // paths cache
    private static string? _CachedTempDir;
    private static string? _CachedBuildDir;
    private static string? _CachedPackageBuildDir;
    private static string? _CachedDownloadDir;
    private static string? _CachedToolDir;
    private static string? _CachedDBDir;
    private static void _FlushPathCache()
    {
        // setup cache
        _CachedTempDir = Directory.CreateDirectory(Path.Combine(ProjectRoot, "build/.sb")).FullName;
        _CachedDownloadDir = Directory.CreateDirectory(Path.Combine(_CachedTempDir, "downloads")).FullName;
        _CachedToolDir = Directory.CreateDirectory(Path.Combine(_CachedTempDir, "tools")).FullName;
        _CachedDBDir = Directory.CreateDirectory(Path.Combine(_CachedTempDir, "dbs")).FullName;
    }

    // getter
    public static string EngineDir => _EngineDir ?? throw new InvalidOperationException("BuildPath.EngineDir is not setup");
    public static string ProjectRoot => _ProjectRoot ?? throw new InvalidOperationException("BuildPath.ProjectRoot is not setup");
    public static string TempDir => _CachedTempDir ?? throw new InvalidOperationException("BuildPath.TempDir is not setup");
    public static string BuildDir => _CachedBuildDir ?? throw new InvalidOperationException("BuildPath.BuildDir is not setup");
    public static string PackageBuildDir => _CachedPackageBuildDir ?? throw new InvalidOperationException("BuildPath.PackageBuildDir is not setup");
    public static string DownloadDir => _CachedDownloadDir ?? throw new InvalidOperationException("BuildPath.DownloadDir is not setup");
    public static string ToolDir => _CachedToolDir ?? throw new InvalidOperationException("BuildPath.ToolDir is not setup");
    public static string DBDir => _CachedDBDir ?? throw new InvalidOperationException("BuildPath.DBDir is not setup");
}

public static class TargetBuildPathExtensions
{
    private static string _SolveUniqueBuildDir(string? mode = null)
    {
        mode ??= BuildSystem.GlobalConfiguration;
        return $"{BuildSystem.TargetOS}-{BuildSystem.TargetArch}-{mode}";
    }


    public static string GetBuildDir(this Target target)
    {
        return target.IsFromPackage ? BuildDirs.PackageBuildDir : BuildDirs.BuildDir;
    }
    public static string GetStoreDir(this Target target, string storeKind, string? mode = null)
    {
        string buildDir = GetBuildDir(target);
        string uniqueDir = _SolveUniqueBuildDir(mode);
        string fullPath = Path.Combine(buildDir, storeKind, uniqueDir, target.Name);
        return Directory.CreateDirectory(fullPath).FullName;
    }
    public static string GetBinaryDir(this Target target, string? mode = null)
    {
        string buildDir = GetBuildDir(target);
        string uniqueDir = _SolveUniqueBuildDir(mode);
        string fullPath = Path.Combine(buildDir, uniqueDir);
        return Directory.CreateDirectory(fullPath).FullName;
    }
    public static string GetBuildSrcDepsDir(this Target target, string? mode = null)
    {
        return target.GetStoreDir(".deps", mode);
    }
    public static string GetBuildObjsDir(this Target target, string? mode = null)
    {
        return target.GetStoreDir(".objs", mode);
    }
    public static string GetBuildGenDir(this Target target, string? mode = null)
    {
        return target.GetStoreDir(".gens", mode);
    }
}