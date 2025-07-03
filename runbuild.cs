using SB;
using SB.Core;
using Serilog;
using Serilog.Events;
using System.Diagnostics;

Stopwatch sw = new();
sw.Start();

TargetCategory Categories = TargetCategory.Runtime;
HashSet<string> AllArgs = args.ToHashSet();

BuildSystem.GlobalConfiguration = "debug";
if (AllArgs.Contains("verbose"))
{
    Engine.LogLevel = LogEventLevel.Verbose;
}
if (AllArgs.Contains("build"))
{
    Categories |= TargetCategory.Runtime | TargetCategory.DevTime;
}
if (AllArgs.Contains("tools"))
{
    Categories |= TargetCategory.Tool;
}
if (AllArgs.Contains("debug"))
{
    BuildSystem.GlobalConfiguration = "debug";
}
if (AllArgs.Contains("release"))
{
    BuildSystem.GlobalConfiguration = "release";
}
if (AllArgs.Contains("clang-cl"))
{
    VisualStudio.UseClangCl = true;
}
if (AllArgs.Contains("msvc"))
{
    VisualStudio.UseClangCl = false;
}

Engine.SetEngineDirectory(SourceLocation.Directory());
var Toolchain = Engine.Bootstrap(SourceLocation.Directory(), Categories); 

Engine.AddEngineTaskEmitters(Toolchain);
Engine.AddCompileCommandsEmitter(Toolchain);

Engine.SetTagsUnderDirectory("thirdparty", TargetTags.ThirdParty);
Engine.SetTagsUnderDirectory("modules/core", TargetTags.Engine);
Engine.SetTagsUnderDirectory("modules/engine", TargetTags.Engine);
Engine.SetTagsUnderDirectory("modules/render", TargetTags.Render);
Engine.SetTagsUnderDirectory("modules/gui", TargetTags.GUI);
Engine.SetTagsUnderDirectory("modules/dcc", TargetTags.DCC);
Engine.SetTagsUnderDirectory("modules/devtime", TargetTags.Tool);
Engine.SetTagsUnderDirectory("modules/tools", TargetTags.Tool);
Engine.SetTagsUnderDirectory("modules/experimental", TargetTags.Experimental);
Engine.SetTagsUnderDirectory("samples", TargetTags.Application);
Engine.SetTagsUnderDirectory("tests", TargetTags.Tests);

Engine.RunBuild();

sw.Stop();

Log.Information($"Total: {sw.ElapsedMilliseconds / 1000.0f}s");
Log.Information($"Execution Total: {sw.ElapsedMilliseconds / 1000.0f}s");
Log.Information($"Compile Commands Total: {CompileCommandsEmitter.Time / 1000.0f}s");
Log.Information($"Compile Total: {CppCompileEmitter.Time / 1000.0f}s");
Log.Information($"Link Total: {CppLinkEmitter.Time / 1000.0f}s");
Log.CloseAndFlush();


if (Categories.HasFlag(TargetCategory.Tool))
{
    Directory.CreateDirectory(".sb/compile_commands/tools");
    CompileCommandsEmitter.WriteToFile(".sb/compile_commands/tools/compile_commands.json");

    string ToolsDirectory = Path.Combine(SourceLocation.Directory(), ".sb", "tools");
    BuildSystem.Artifacts.AsParallel().ForAll(artifact =>
    {
        if (artifact is LinkResult Program)
        {
            if (!Program.IsRestored)
            {
                // copy to /.sb/tools
                if (File.Exists(Program.PDBFile))
                    File.Copy(Program.PDBFile, Path.Combine(ToolsDirectory, Path.GetFileName(Program.PDBFile)), true);
                if (File.Exists(Program.TargetFile))
                    File.Copy(Program.TargetFile, Path.Combine(ToolsDirectory, Path.GetFileName(Program.TargetFile)), true);
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