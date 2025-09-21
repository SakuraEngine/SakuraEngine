using SB;
using Cli = SB.Cli;

namespace SB;

public class CompileCommandsCommand : CommandBase
{

    [Cli.RegisterCmd(Name = "compile_commands", Help = "Generate Compile Commands for IDEs", Usage = "SB compile_commands [options]")]
    public static object RegisterCommand() => new CompileCommandsCommand();

    // with codegen
    [Cli.Option(Name = "with-codegen", Help = "Include codegen files in compile commands (will run codegen first)")]
    public bool WithCodegen { get; set; } = false;

    public CompileCommandsCommand()
    {
        CategoryString = "all";
    }

    public override int OnExecute()
    {
        if (WithCodegen)
        {
            Engine.AddCodegenEmitters(Toolchain);
        }

        Engine.AddCompileCommandsEmitter(Toolchain);
        Engine.RunBuild();

        var outDir = Path.Join(BuildDirs.TempDir, "compile_commands");
        var cppOutDir = Path.Join(outDir, "cpp");
        var shaderOutDir = Path.Join(outDir, "shaders");

        Directory.CreateDirectory(cppOutDir);
        CompileCommandsEmitter.WriteToFile(Path.Join(cppOutDir, "compile_commands.json"));

        Directory.CreateDirectory(shaderOutDir);
        CppSLCompileCommandsEmitter.WriteCompileCommandsToFile(Path.Join(shaderOutDir, "compile_commands.json"));

        return 0;
    }

}