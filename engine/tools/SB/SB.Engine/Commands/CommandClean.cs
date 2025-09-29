using SB;
using Serilog;

namespace SB;

public class CleanCommand : CommandBase
{
    [Cli.Option(Name = "database", ShortName = 'd', Help = "Database to clean", IsRequired = false, Selections = ["all", "packages", "targets", "shaders", "sdks"])]
    public string Database { get; set; } = "targets";

    public override int OnExecute()
    {
        Log.Information("Cleaning build cache dependency databases for {Database}...", Database);

        // Clean dependency databases using API
        try
        {
            bool all = Database == "all";
            if (all || Database == "targets")
                BuildDepends.Solve(false).ClearDatabase();
            if (all || Database == "packages")
                BuildDepends.Solve(true).ClearDatabase();
            if (all || Database == "shaders")
                EngineDepends.ShaderCompile.ClearDatabase();
            if (all || Database == "misc")
                EngineDepends.Misc.ClearDatabase();
            if (all || Database == "codegen")
                EngineDepends.Codegen.ClearDatabase();
            if (Database == "sdks")
            {
                InstallDepends.Download.ClearDatabase();
                InstallDepends.SDK.ClearDatabase();
            }
        }
        catch (Exception ex)
        {
            Log.Error("Failed to clear databases: {Error}", ex.Message);
        }
        return 0;
    }

    [Cli.RegisterCmd(Name = "clean", ShortName = 'c', Help = "Clean build cache and dependency databases", Usage = "SB clean [options]\nSB c [options]")]
    public static object RegisterCommand() => new CleanCommand();
}