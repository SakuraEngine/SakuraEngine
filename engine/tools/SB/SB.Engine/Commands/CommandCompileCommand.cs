using SB;
using Serilog;

namespace SB;

public class CompileCommandsCommand : CommandBase
{

    [Cli.RegisterCmd(Name = "compile_commands", Help = "Generate Compile Commands for IDEs", Usage = "SB compile_commands [options]")]
    public static object RegisterCommand() => new CompileCommandsCommand();

    // with codegen
    [Cli.Option(Name = "with-codegen", Help = "Include codegen files in compile commands (will run codegen first)")]
    public bool WithCodegen { get; set; } = false;

    [Cli.Option(Name = "clean-clangd-cache", Help = "Clean up clangd cache after generating compile commands", Selections = ["none", "cpp", "shader", "all"])]
    public string CleanClangdCache { get; set; } = "none";

    public CompileCommandsCommand()
    {
        CategoryString = "all";
    }

    public override int OnExecute()
    {
        // collect compile commands
        if (WithCodegen)
        {
            Engine.AddCodegenEmitters(Toolchain);
        }
        Engine.AddCompileCommandsEmitter(Toolchain);
        Engine.RunBuild();

        // solve paths
        var outDir = Path.Join(BuildDirs.TempDir, "compile_commands");
        var cppOutDir = Path.Join(outDir, "cpp");
        var shaderOutDir = Path.Join(outDir, "shaders");

        // output compile commands
        Directory.CreateDirectory(cppOutDir);
        CompileCommandsEmitter.WriteToFile(Path.Join(cppOutDir, "compile_commands.json"));
        Directory.CreateDirectory(shaderOutDir);
        CppSLCompileCommandsEmitter.WriteCompileCommandsToFile(Path.Join(shaderOutDir, "compile_commands.json"));

        // clean up clangd cache if needed
        if (CleanClangdCache != "none")
        {
            bool cleanCpp = CleanClangdCache == "cpp" || CleanClangdCache == "all";
            bool cleanShaders = CleanClangdCache == "shader" || CleanClangdCache == "all";
            var cppClangdCacheDir = Path.Join(cppOutDir, ".cache");
            var shaderClangdCacheDir = Path.Join(shaderOutDir, ".cache");

            if (cleanCpp && Directory.Exists(cppClangdCacheDir))
            {
                Directory.Delete(cppClangdCacheDir, true);
                Log.Information("Cleaned up clangd cache for C++ at {Dir}", cppClangdCacheDir);
            }
            if (cleanShaders && Directory.Exists(shaderClangdCacheDir))
            {
                Directory.Delete(shaderClangdCacheDir, true);
                Log.Information("Cleaned up clangd cache for Shaders at {Dir}", shaderClangdCacheDir);
            }
        }

        return 0;
    }

}