using SB;
using Serilog;

namespace SB;

public class VSCommand : CommandBase
{
    [Cli.Option(Name = "solution-name", Help = "Name of the solution file (without extension)", IsRequired = false)]
    public string SolutionName { get; set; } = "SakuraEngine";

    [Cli.Option(Name = "output", Help = "Output directory for solution files", IsRequired = false)]
    public string OutputDirectory { get; set; } = Path.Combine(BuildDirs.TempDir, "VisualStudio");


    public override int OnExecute()
    {
        Log.Information("Generating Visual Studio solution...");

        // Set output directory for VS emitter
        VSEmitter.RootDirectory = !string.IsNullOrEmpty(BuildDirs.EngineDir) ? BuildDirs.EngineDir : Directory.GetCurrentDirectory();
        VSEmitter.OutputDirectory = OutputDirectory;

        // Add VS emitter to generate project files
        BuildSystem.AddTaskEmitter("VSEmitter", new VSEmitter(Toolchain));

        // Run the build to process all targets
        Engine.RunBuild();

        // Generate solution file
        var solutionPath = Path.Combine(OutputDirectory, $"{SolutionName}.sln");
        VSEmitter.GenerateSolution(solutionPath, SolutionName);

        Log.Information("Visual Studio solution generated at: {Path}", Path.GetFullPath(solutionPath));

        return 0;
    }

    [Cli.RegisterCmd(Name = "vs", Help = "Generate Visual Studio solution", Usage = "SB vs [options]")]
    public static object RegisterCommand() => new VSCommand();
}
