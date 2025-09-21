using SB;
using Cli = SB.Cli;

namespace SB;

public class BuildCommand : CommandBase
{
    [Cli.Option(Name = "shader-only", Help = "Build shaders only", IsRequired = false)]
    public bool ShaderOnly { get; set; }

    [Cli.Option(Name = "target", Help = "Build a single target", IsRequired = false)]
    public string? SingleTarget { get; set; }

    public override int OnExecute()
    {
        Engine.AddCodegenEmitters(Toolchain);
        Engine.AddShaderTaskEmitters(Toolchain);
        if (!ShaderOnly)
        {
            Engine.AddEngineTaskEmitters(Toolchain);
        }
        return Engine.RunBuild(SingleTarget);
    }

    [Cli.RegisterCmd(Name = "build", ShortName = 'b', Help = "Build the project", Usage = "SB build [options]\nSB b [options]")]
    public static object RegisterCommand() => new BuildCommand();
}
