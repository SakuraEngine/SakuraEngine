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

// notify prepare commandline stage for some basic setup
BuildStage.UpdateStage(EBuildStage.PrepareCommandline);

// now, invoke command
Cli.Command cmd = new Cli.Command
{
    Name = "SB",
    Help = "Sakura Build System (SB) - A fast, modern build system for C++ projects",
    Usage = "SB [sub-commands] [options]"
};
var banner =
@"

    _____         _                        ____          _  _      _ 
   / ____|       | |                      |  _ \        (_)| |    | |
  | (___    __ _ | | __ _   _  _ __  __ _ | |_) | _   _  _ | |  __| |
   \___ \  / _` || |/ /| | | || '__|/ _` ||  _ < | | | || || | / _` |
   ____) || (_| ||   < | |_| || |  | (_| || |_) || |_| || || || (_| |
  |_____/  \__,_||_|\_\ \__,_||_|   \__,_||____/  \__,_||_||_| \__,_|



";
return Cli.ReflCommand.InvokeDefaultCommandFromDomain(AppDomain.CurrentDomain, cmd, filteredArgs, banner);