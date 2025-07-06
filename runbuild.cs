using SB;
using SB.Core;
using Serilog;
using Serilog.Events;
using System.Diagnostics;

// Main entry point
var parser = new CommandParser();
var mainCmd = new MainCommand();
// Check if arguments look like they're for the default build command
// This allows "SB --mode=release" to work as "SB build --mode=release"
if (args.Length > 0 && !args[0].Equals("build", StringComparison.OrdinalIgnoreCase) && 
    !args[0].Equals("test", StringComparison.OrdinalIgnoreCase) &&
    (args[0].StartsWith("-") || args[0].StartsWith("--")))
{
    // Insert "build" at the beginning
    var newArgs = new List<string> { "build" };
    newArgs.AddRange(args);
    args = newArgs.ToArray();
}
else if (args.Length == 0)
{
    // No arguments means default to build
    args = new[] { "build" };
}
// Configure the parser
parser.MainCmd(mainCmd, "SB", "Sakura Build System", "SB [options] <command> [command-options]");
return parser.ParseSync(args);

// Main command that holds global options
public class MainCommand
{
    [CmdSub(Name = "build", ShortName = 'b', Help = "Build the project")]
    public BuildCommand Build { get; set; } = new BuildCommand();

    [CmdSub(Name = "test", ShortName = 't', Help = "Run tests")]
    public TestCommand Test { get; set; } = new TestCommand();
}

public abstract class CommandBase
{
    [CmdOption(Name = "verbose", ShortName = 'v', Help = "Enable verbose logging", IsRequired = false)]
    public bool Verbose { get; set; } = false;

    [CmdOption(Name = "mode", ShortName = 'm', Help = "Build mode (debug/release)", IsRequired = false)]
    public string Mode { get; set; } = "debug";

    [CmdOption(Name = "sha-depend", ShortName = 's', Help = "Use SHA instead of DateTime for dependency checking", IsRequired = false)]
    public bool UseShaDepend { get; set; } = false;

    [CmdOption(Name = "category", ShortName = 'c', Help = "Build tools", IsRequired = false)]
    public string Category { get; set; } = "modules";

    [CmdOption(Name = "toolchain", Help = "Toolchain to use", IsRequired = false)]
    public string ToolchainName { get; set; } = OperatingSystem.IsWindows() ? "clang-cl" : "clang";

    [CmdExec]
    public void Exec()
    {
        Stopwatch timer = Stopwatch.StartNew();

        // Use global verbose setting if available
        LogEventLevel LogLevel = LogEventLevel.Information;
        if (Verbose)
        {
            LogLevel = LogEventLevel.Verbose;
        }
        Engine.InitializeLogger(LogLevel);
        Engine.SetEngineDirectory(SourceLocation.Directory());

        // use sha to check file dependency instead of using last write time 
        if (UseShaDepend)
            Depend.DefaultUseSHAInsteadOfDateTime = true;

        // Set compiler
        if (ToolchainName == "clang-cl")
            VisualStudio.UseClangCl = true;
        else if (ToolchainName == "msvc")
            VisualStudio.UseClangCl = false;
        else if (ToolchainName == "clang")
        {
            ; //XCode.
        }

        // Set configuration based on mode
        BuildSystem.GlobalConfiguration = Mode.ToLower();
        if (Mode.ToLower() != "debug" && Mode.ToLower() != "release")
        {
            Log.Warning($"Unknown build mode '{Mode}', defaulting to debug");
            BuildSystem.GlobalConfiguration = "debug";
        }
        Log.Information("Build start with configuration: {Configuration}", BuildSystem.GlobalConfiguration);

        // Set categories
        if (Category == "modules")
            Categories |= TargetCategory.Runtime | TargetCategory.DevTime;
        if (Category == "tools")
            Categories |= TargetCategory.Tool;
        Log.Information("Build start with categories: {Categories}", Categories);

        // Bootstrap engine
        _toolchain = Engine.Bootstrap(SourceLocation.Directory(), Categories);

        // run subcmd exec
        OnExecute();

        // stop and dump counters
        timer.Stop();
        Log.Information($"Total: {timer.ElapsedMilliseconds / 1000.0f}s");
        Log.Information($"Execution Total: {timer.ElapsedMilliseconds / 1000.0f}s");
        Log.Information($"Compile Commands Total: {CompileCommandsEmitter.Time / 1000.0f}s");
        Log.Information($"Compile Total: {CppCompileEmitter.Time / 1000.0f}s");
        Log.Information($"Link Total: {CppLinkEmitter.Time / 1000.0f}s");
        Log.CloseAndFlush();
    }

    public abstract void OnExecute();
    public IToolchain Toolchain => _toolchain!;
    private IToolchain? _toolchain;
    protected TargetCategory Categories = TargetCategory.Package;
}

// Build subcommand
public class BuildCommand : CommandBase
{
    [CmdOption(Name = "shader-only", Help = "Build shaders only", IsRequired = false)]
    public bool ShaderOnly { get; set; }

    [CmdOption(Name = "target", Help = "Build a single target", IsRequired = false)]
    public string? SingleTarget { get; set; }

    public override void OnExecute()
    {
        Engine.AddShaderTaskEmitters(Toolchain);
        if (!ShaderOnly)
        {
            Engine.AddEngineTaskEmitters(Toolchain);
            Engine.AddCompileCommandsEmitter(Toolchain);
        }

        Engine.RunBuild(SingleTarget);

        // Handle post-build tasks
        if (Categories.HasFlag(TargetCategory.Tool))
        {
            Directory.CreateDirectory(".sb/compile_commands/tools");
            CompileCommandsEmitter.WriteToFile(".sb/compile_commands/tools/compile_commands.json");

            string ToolsDirectory = Path.Combine(SourceLocation.Directory(), ".sb", "tools");
            BuildSystem.Artifacts.AsParallel().ForAll(artifact =>
            {
                if (artifact is LinkResult Program)
                {
                    if (!Program.IsRestored && Program.Target.IsCategory(TargetCategory.Tool))
                    {
                        // copy to /.sb/tools
                        if (File.Exists(Program.PDBFile))
                        {
                            Log.Verbose("Copying PDB file {PDBFile} to {ToolsDirectory}", Program.PDBFile, ToolsDirectory);
                            File.Copy(Program.PDBFile, Path.Combine(ToolsDirectory, Path.GetFileName(Program.PDBFile)), true);
                        }
                        if (File.Exists(Program.TargetFile))
                        {
                            Log.Verbose("Copying target file {TargetFile} to {ToolsDirectory}", Program.TargetFile, ToolsDirectory);
                            File.Copy(Program.TargetFile, Path.Combine(ToolsDirectory, Path.GetFileName(Program.TargetFile)), true);
                        }
                    }
                }
            });
        }
        else
        {
            Directory.CreateDirectory(".sb/compile_commands/modules");
            CompileCommandsEmitter.WriteToFile(".sb/compile_commands/modules/compile_commands.json");

            Directory.CreateDirectory(".sb/compile_commands/shaders");
            CppSLEmitter.WriteCompileCommandsToFile(".sb/compile_commands/shaders/compile_commands.json");
        }
    }
}

// Test subcommand
public class TestCommand : CommandBase
{
    public override void OnExecute()
    {
        // Add necessary emitters for test execution
        Engine.AddEngineTaskEmitters(Toolchain);
        Engine.RunBuild();

        // This assumes build has already been done
        var Programs = BuildSystem.Artifacts.Where(a => a is LinkResult)
            .Select(a => (LinkResult)a)
            .Where(p => p.Target.IsCategory(TargetCategory.Tests) && p.Target.GetTargetType() == TargetType.Executable)
            .ToList();

        if (!Programs.Any())
        {
            Log.Warning("No test programs found. Make sure to build tests first.");
            return;
        }

        Programs.AsParallel().ForAll(program =>
        {
            Stopwatch sw = Stopwatch.StartNew();
            Log.Information("Running test target {TargetName}", program.Target.Name);
            var result = BuildSystem.RunProcess(program.TargetFile, "", out var output, out var error, null, Path.GetDirectoryName(program.TargetFile));
            sw.Stop();
            float Seconds = sw.ElapsedMilliseconds / 1000.0f;
            if (result != 0)
            {
                Log.Error("Test target {TargetName} failed with error: {Error}", program.Target.Name, error);
            }
            else
            {
                Log.Information("Test target {TargetName} passed, cost {Seconds}s", program.Target.Name, Seconds);
            }
        });
    }
}