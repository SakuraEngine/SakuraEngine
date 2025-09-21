using SB;
using SB.Core;

// load all assemblies for commands
AppDomain.CurrentDomain.Load("SB.Core");
AppDomain.CurrentDomain.Load("SB.Engine");

// get engine dir
var engineDir = Path.GetFullPath(Path.Join(SourceLocation.Directory(), "../"));

// setup engine directory
BuildDirs.SetupPaths(
    EngineDir: engineDir,
    ProjectRoot: Directory.GetCurrentDirectory()
);

// filter SB args passed by dotnet run
string[] filteredArgs = args;
if (args.Length > 0 && args[0] == "SB")
{
    filteredArgs = args[1..];
}

return SB.Cli.ReflCommand.InvokeDefaultCommandFromDomain(AppDomain.CurrentDomain, filteredArgs);
