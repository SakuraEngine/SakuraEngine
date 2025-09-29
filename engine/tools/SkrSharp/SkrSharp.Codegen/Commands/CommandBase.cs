using Serilog;
using Serilog.Events;
using System.Diagnostics;

namespace SkrSharp;

public abstract class CommandBase
{
    [Cli.Option(Name = "verbose", ShortName = 'v', Help = "Enable verbose logging", IsRequired = false)]
    public bool Verbose { get; set; } = false;

    [Cli.ExecCmd]
    public int Exec()
    {
        Sakura.Logging.InitializeLogger(Verbose ? LogEventLevel.Verbose : LogEventLevel.Information);

        var returnCode = OnExecuteAsync().GetAwaiter().GetResult();
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
}