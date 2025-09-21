using SB;
using Serilog;
using Cli = SB.Cli;

namespace SB;

public class VSCodeCommand : CommandBase
{
    [Cli.Option(Name = "debugger", Help = "debugger to use", IsRequired = false, Selections = ["cppdbg", "lldb-dap", "codelldb", "cppvsdbg"])]
    public string Debugger { get; set; } = "";

    [Cli.Option(Name = "workspace", Help = "Workspace root directory", IsRequired = false)]
    public string WorkspaceRoot { get; set; } = BuildDirs.ProjectRoot;

    [Cli.Option(Name = "temp-output", Help = "Temp files output dir", IsRequired = false)]
    public string TempFilesDir { get; set; } = Path.Combine(BuildDirs.TempDir, "vscode");

    [Cli.Option(Name = "preserve-user", Help = "Preserve user-created debug configurations", IsRequired = false)]
    public bool PreserveUser { get; set; } = true;

    [Cli.Option(Name = "clear", Help = "Preserve user-created debug configurations", IsRequired = false)]
    public bool ClearMode { get; set; } = false;

    public override int OnExecute()
    {
        Log.Information("Generating VSCode debug configurations...");

        if (ClearMode)
        {
            var emitter = new VSCodeDebugEmitter();
            emitter.WorkspaceRoot = !string.IsNullOrEmpty(WorkspaceRoot) ? WorkspaceRoot :
                                    (!string.IsNullOrEmpty(BuildDirs.EngineDir) ? BuildDirs.EngineDir : Directory.GetCurrentDirectory());
            emitter.CmdFilesOutputDir = Path.Combine(TempFilesDir, "task_cmds");
            emitter.MergedNatvisOutputDir = Path.Combine(TempFilesDir, "natvis");
            // emitter.Debugger = Debugger.ToLower();
            emitter.Mode = Mode;
            emitter.Toolchain = Toolchain;
            emitter.PreserveUserConfig = PreserveUser;
            emitter.Clear();
            Log.Information("Cleared generated VSCode debug configurations in: {Path}", Path.GetFullPath(Path.Combine(emitter.WorkspaceRoot, ".vscode")));
        }
        else
        {
            // Add VSCode emitter
            var emitter = new VSCodeDebugEmitter();
            emitter.WorkspaceRoot = !string.IsNullOrEmpty(WorkspaceRoot) ? WorkspaceRoot :
                                    (!string.IsNullOrEmpty(BuildDirs.EngineDir) ? BuildDirs.EngineDir : Directory.GetCurrentDirectory());
            emitter.CmdFilesOutputDir = Path.Combine(TempFilesDir, "task_cmds");
            emitter.MergedNatvisOutputDir = Path.Combine(TempFilesDir, "natvis");
            emitter.Debugger = Debugger;
            emitter.Mode = Mode;
            emitter.Toolchain = Toolchain;
            emitter.PreserveUserConfig = PreserveUser;
            BuildSystem.AddTaskEmitter("VSCodeDebugEmitter", emitter);

            // Run the build to process all targets
            Engine.RunBuild();

            // Generate the debug configurations
            emitter.Generate();
            Log.Information("VSCode debug configurations generated in: {Path}", Path.GetFullPath(Path.Combine(emitter.WorkspaceRoot, ".vscode")));
        }

        return 0;
    }

    [Cli.RegisterCmd(Name = "vscode", Help = "Generate VSCode debug configurations", Usage = "SB vscode [options]")]
    public static object RegisterCommand() => new VSCodeCommand();
}
