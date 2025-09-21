using SB.Core;

namespace SB;

public static class EngineDepends
{
    // depends
    private static DependDatabase? _Codegen;
    private static DependDatabase? _Misc;
    private static DependDatabase? _ShaderCompile;

    [AfterStage(EBuildStage.LoadedToolChain)]
    private static void _SetupDepends()
    {
        _Codegen ??= new DependDatabase(BuildDirs.BuildDir, $"Engine.Codegen.{Engine.GlobalConfiguration}");
        _ShaderCompile ??= new DependDatabase(BuildDirs.BuildDir, "Engine.ShaderCompileDepends");
        _Misc ??= new DependDatabase(BuildDirs.TempDir, "Engine.Misc");
    }

    // getters
    public static DependDatabase Codegen => _Codegen ?? throw new InvalidOperationException("EngineDepends not initialized");
    public static DependDatabase Misc => _Misc ?? throw new InvalidOperationException("EngineDepends not initialized");
    public static DependDatabase ShaderCompile => _ShaderCompile ?? throw new InvalidOperationException("EngineDepends not initialized");
}
