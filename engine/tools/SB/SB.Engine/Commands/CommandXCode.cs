namespace SB;

using Serilog;
using XCode;

public class XCodeCommand : CommandBase
{
    [Cli.Option(Name = "xcodeproj-name", Help = "Name of the xcode project file (without extension)", IsRequired = false)]
    public string XCodeProjName { get; set; } = "SakuraEngine";

    [Cli.Option(Name = "output", Help = "Output directory for project files", IsRequired = false)]
    public string OutputDirectory { get; set; } = Path.Combine(BuildDirs.TempDir, "XCode");

    public override int OnExecute()
    {
        Log.Information("Generating XCode project...");

        XCodeEmitter emitter = new(Toolchain, OutputDirectory, XCodeProjName);
        BuildSystem.AddTaskEmitter("XCodeEmitter", emitter);
        Engine.RunBuild();
        XCodeEmitter.GenerateProjectFile(emitter.ProjectInfo);

        Log.Information("XCode project generated at: {Path}", Path.GetFullPath(Path.Combine(OutputDirectory, $"{XCodeProjName}.xcodeproj")));
        return 0;
    }

    [Cli.RegisterCmd(Name = "xcode", Help = "Generate XCode project", Usage = "SB xcode [options]")]
    public static object RegisterCommand() => new XCodeCommand();
}