using SB.Core;
using Serilog;
using Serilog.Events;
using System.Diagnostics;

namespace SB;

public abstract class CommandBase
{
    [Cli.Option(Name = "verbose", ShortName = 'v', Help = "Enable verbose logging", IsRequired = false)]
    public bool Verbose { get; set; } = false;

    [Cli.Option(Name = "mode", ShortName = 'm', Help = "Build mode", IsRequired = false)]
    public string Mode { get; set; } = Engine.DefaultMode;
    [Cli.OptionSelectionProvider("mode")]
    public static IEnumerable<string> ModeSelections()
    {
        Engine.LoadConfigurations();
        return Engine.Configurations.Keys;
    }

    [Cli.Option(Name = "sha-depend", Help = "Use SHA instead of DateTime for dependency checking", IsRequired = false)]
    public bool UseShaDepend { get; set; } = false;

    [Cli.Option(Name = "category", ShortName = 'c', Help = "Build by category", IsRequired = false, Selections = ["all", "modules", "tools"])]
    public string CategoryString { get; set; } = "all";

    [Cli.Option(Name = "toolchain", Help = "Toolchain to use", IsRequired = false, Selections = ["msvc", "clang-cl", "clang"])]
    public string ToolchainName { get; set; } = Engine.DefaultToolchain;

    [Cli.Option(Name = "proxy", Help = "Set HTTP proxy for downloads")]
    public string Proxy { get; set; } = "";

    [Cli.Option(Name = "plat", ShortName = 'p', Help = "Set build platform")]
    public string Platform { get; set; } = "";

    [Cli.Option(Name = "arch", ShortName = 'a', Help = "Set build architecture")]
    public string Architecture { get; set; } = "";

    [Cli.ExecCmd]
    public int Exec()
    {
        Stopwatch timer = Stopwatch.StartNew();

        // setup log level
        Sakura.Logging.InitializeLogger(Verbose ? LogEventLevel.Verbose : LogEventLevel.Information);

        // notify prepare commandline stage for some basic setup
        BuildStage.UpdateStage(EBuildStage.PrepareCallCommandLine);

        // setup proxy
        if (!string.IsNullOrEmpty(Proxy))
        {
            Log.Information("Setting HTTP proxy to {Proxy}", Proxy);
            Download.HttpProxy = Proxy;
        }

        // use sha to check file dependency instead of using last write time 
        Depend.DefaultUseSHAInsteadOfDateTime = UseShaDepend;

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
        Log.Information("Build start with configuration: {Configuration}", BuildSystem.GlobalConfiguration);
        BuildStage.UpdateStage(EBuildStage.SetupConfigure);

        // Set categories
        Categories |= CategoryString switch
        {
            "modules" => TargetCategory.Runtime | TargetCategory.DevTime,
            "tools" => TargetCategory.Tool,
            "all" => TargetCategory.Tool | TargetCategory.Runtime | TargetCategory.DevTime,
            _ => throw new ArgumentException($"Invalid category: {CategoryString}"),
        };
        Log.Information("Build start with categories: {Categories}", Categories);

        // Bootstrap engine
        _toolchain = Engine.Bootstrap(Categories);

        // run custom exec
        var returnCode = OnExecuteAsync().GetAwaiter().GetResult();

        // stop and dump counters
        timer.Stop();
        if (DumpCounters)
        {
            Log.Information($"Total: {timer.ElapsedMilliseconds / 1000.0f}s");
            Log.Information($"Execution Total: {timer.ElapsedMilliseconds / 1000.0f}s");
            Log.Information($"Compile Commands Total: {CompileCommandsEmitter.Time / 1000.0f}s");
            Log.Information($"Compile Total: {CppCompileEmitter.Time / 1000.0f}s");
            Log.Information($"Link Total: {CppLinkEmitter.Time / 1000.0f}s");
        }

        Log.CloseAndFlush();

        return returnCode;
    }

    public virtual Task<int> OnExecuteAsync()
    {
        return Task.FromResult(OnExecute());
    }

    public virtual int OnExecute()
    {
        return 0;
    }
    public IToolchain Toolchain => _toolchain!;
    private IToolchain? _toolchain;
    protected TargetCategory Categories = TargetCategory.Package;
    protected virtual bool DumpCounters => true;
}