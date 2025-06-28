using SB;
using SB.Core;
using Serilog;
using System.Diagnostics;

Stopwatch sw = new();
sw.Start();

Engine.SetEngineDirectory(SourceLocation.Directory());
var Toolchain = Engine.Bootstrap(SourceLocation.Directory());
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

CompileCommandsEmitter.WriteToFile("compile_commands.json");