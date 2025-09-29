using Serilog;

namespace SkrSharp;

public class GenIL : CommandBase
{
    public override int OnExecute()
    {
        return 0;
    }

    [Cli.RegisterCmd(Name = "il", Help = "Generate il cpp source codes", Usage = "SB il")]
    public static object RegisterCommand() => new GenIL();
}
