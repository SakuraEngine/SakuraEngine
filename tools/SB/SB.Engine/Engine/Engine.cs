using SB.Core;
using Serilog;
using Serilog.Events;
using Serilog.Sinks.SystemConsole.Themes;
using System.Reflection;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public partial class Engine : BuildSystem
    {
        public static void SetEngineDirectory(string Directory) => EngineDirectory = Directory;

        public static IToolchain Bootstrap(string ProjectRoot, TargetCategory Categories)
        {
            using (Profiler.BeginZone("Bootstrap", color: (uint)Profiler.ColorType.WebMaroon))
            {
                Log.Verbose("Runs on {HostOS} with {ProcessorCount} logical processors", HostOS, Environment.ProcessorCount);

                IToolchain? Toolchain = null;
                if (BS.HostOS == OSPlatform.Windows)
                    Toolchain = VisualStudioDoctor.VisualStudio;
                else if (BS.HostOS == OSPlatform.OSX)
                    Toolchain = XCodeDoctor.XCode;
                else
                    throw new Exception("Unsupported Platform!");
                
                if (BS.HostOS == OSPlatform.Windows)
                {
                    char DriveLetter = SourceLocation.Directory()[0];
                    if (Char.IsLower(DriveLetter))
                        throw new Exception($"Drive letter {DriveLetter} from source location must be upper case! You might compiled SB in git bash environment, please recompile it in cmd.exe or powershell.exe!");
                }

                BS.TempPath = Directory.CreateDirectory(Path.Combine(ProjectRoot, ".sb")).FullName;
                BS.BuildPath = Directory.CreateDirectory(Path.Combine(ProjectRoot, ".build", Toolchain.Name)).FullName;
                BS.PackageBuildPath = Directory.CreateDirectory(Path.Combine(ProjectRoot, ".pkgs/.build", Toolchain.Name)).FullName;
                BS.LoadConfigurations();

                Log.Verbose("Load Targets... ");
                LoadTargets(Categories);
                
                Stopwatch sw = new();
                sw.Start();
                Log.Verbose("Run Doctors... ");
                Engine.RunDoctors();
                sw.Stop();
                Log.Verbose($"Doctors Finished... cost {sw.ElapsedMilliseconds / 1000.0f}s");
                return Toolchain!;
            }
        }

        public static void AddEngineTaskEmitters(IToolchain Toolchain)
        {
            Log.Verbose("Add Engine Task Emitters... ");

            Engine.AddTaskEmitter("Codgen.Meta", new CodegenMetaEmitter(Toolchain));

            Engine.AddTaskEmitter("Module.Info", new ModuleInfoEmitter());

            Engine.AddTaskEmitter("ISPC.Compile", new ISPCEmitter());

            Engine.AddTaskEmitter("Cpp.UnityBuild", new UnityBuildEmitter())
                .AddDependency("Module.Info", DependencyModel.PerTarget);

            Engine.AddTaskEmitter("Codgen.Codegen", new CodegenRenderEmitter(Toolchain))
                .AddDependency("Cpp.UnityBuild", DependencyModel.PerTarget)
                .AddDependency("Codgen.Meta", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Meta", DependencyModel.PerTarget);

            Engine.AddTaskEmitter("Cpp.PCH", new PCHEmitter(Toolchain))
                .AddDependency("ISPC.Compile", DependencyModel.PerTarget)
                .AddDependency("Codgen.Codegen", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Codegen", DependencyModel.PerTarget);

            Engine.AddTaskEmitter("Cpp.Compile", new CppCompileEmitter(Toolchain))
                .AddDependency("ISPC.Compile", DependencyModel.PerTarget)
                .AddDependency("Module.Info", DependencyModel.PerTarget)
                .AddDependency("Cpp.UnityBuild", DependencyModel.PerTarget)
                .AddDependency("Cpp.PCH", DependencyModel.PerTarget)
                .AddDependency("Cpp.PCH", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Codegen", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Codegen", DependencyModel.PerTarget);
            
            Engine.AddTaskEmitter("Cpp.Link", new CppLinkEmitter(Toolchain))
                .AddDependency("Cpp.Link", DependencyModel.ExternalTarget)
                .AddDependency("Cpp.Compile", DependencyModel.PerTarget);
        }

        public static void AddCompileCommandsEmitter(IToolchain Toolchain)
        {
            Engine.AddTaskEmitter("Cpp.CompileCommands", new CompileCommandsEmitter(Toolchain))
                .AddDependency("Module.Info", DependencyModel.PerTarget);

            Engine.GetTaskEmitter("Cpp.UnityBuild")
                ?.AddDependency("Cpp.CompileCommands", DependencyModel.PerTarget);
        }

        public static void AddShaderTaskEmitters(IToolchain Toolchain)
        {
            if (BuildSystem.TargetOS == OSPlatform.Windows)
            {
                Engine.AddTaskEmitter("DXC.Compile", new DXCEmitter());
            }
            Engine.AddTaskEmitter("CppSL.Compile", new CppSLEmitter());
        }

        public static new void RunBuild(string? singleTargetName = null)
        {
            BS.RunBuild(singleTargetName);
        }

        private static void LoadTargets(TargetCategory Categories)
        {
            BS.TargetDefaultSettings += (Target Target) =>
            {
                Target.CppVersion("20")
                    .Exception(false)
                    .RTTI(false)
                    .LinkDirs(Visibility.Public, Target.GetBinaryPath());
                if (BS.TargetOS == OSPlatform.Windows)
                {
                    Target.RuntimeLibrary("MD");
                }
            };

            var Assemblies = AppDomain.CurrentDomain.GetAssemblies();
            var Types = Assemblies.AsParallel().SelectMany(A => A.GetTypes());
            var Scripts = Types.Where(Type => IsTargetOfCategory(Type, Categories));
            foreach (var Script in Scripts)
            {
                var PrevTargets = AllTargets.Values.ToHashSet();
                System.Runtime.CompilerServices.RuntimeHelpers.RunClassConstructor(Script.TypeHandle);
                var NewTargets = AllTargets.Values.Except(PrevTargets);
                foreach (var NewTarget in NewTargets)
                {
                    NewTarget.SetCategory(Script.GetCustomAttribute<TargetScript>()!.Category);
                }
            }
        }
        
        private static bool IsTargetOfCategory(Type Type, TargetCategory Category)
        {
            var TargetAttr = Type.GetCustomAttribute<TargetScript>();
            if (TargetAttr == null) return false;
            return (TargetAttr.Category & Category) != 0;
        }

        public static void InitializeLogger(LogEventLevel LogLevel = LogEventLevel.Information)
        {
            Console.OutputEncoding = System.Text.Encoding.UTF8;
            SystemConsoleTheme ConsoleLogTheme = new SystemConsoleTheme(
               new Dictionary<ConsoleThemeStyle, SystemConsoleThemeStyle>
               {
                   [ConsoleThemeStyle.Text] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.SecondaryText] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.TertiaryText] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.DarkGray },
                   [ConsoleThemeStyle.Invalid] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Yellow },
                   [ConsoleThemeStyle.Null] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Blue },
                   [ConsoleThemeStyle.Name] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.String] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.Number] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Green },
                   [ConsoleThemeStyle.Boolean] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Blue },
                   [ConsoleThemeStyle.Scalar] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Green },

                   [ConsoleThemeStyle.LevelVerbose] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.LevelDebug] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.LevelInformation] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.LevelWarning] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Yellow },
                   [ConsoleThemeStyle.LevelError] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White, Background = ConsoleColor.Red },
                   [ConsoleThemeStyle.LevelFatal] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White, Background = ConsoleColor.Red },
               });

            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Is(LogLevel)
                // .Enrich.WithThreadId()
                // .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Timestamp:yyyy-MM-dd HH:mm:ss.ffff zzz} {Message:lj}{NewLine}{Exception}")
                .WriteTo.Async(a => a.Logger(l => l
                    .Filter.ByIncludingOnly(e => e.Level == LogLevel)
                    .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Message:lj}{NewLine}{Exception}", theme: ConsoleLogTheme)
                ))
                .WriteTo.Async(a => a.Logger(l => l
                    .Filter.ByExcluding(e => e.Level == LogLevel)
                    .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Level:u}: {Message:lj}{NewLine}{Exception}", theme: ConsoleLogTheme)
                ))
                .CreateLogger();
        }

        public static DependDatabase ConfigureAwareDepend = new DependDatabase(Engine.TempPath, "Engine.ConfigureAwareDepends." + Engine.GlobalConfiguration);
        public static DependDatabase ConfigureNotAwareDepend = new DependDatabase(Engine.TempPath, "Engine.ConfigureNotAwareDepends");

        public static string EngineDirectory { get; private set; } = Directory.GetCurrentDirectory();
        public static string ToolDirectory => Path.Combine(TempPath, "tools");
        public static string DownloadDirectory => Path.Combine(TempPath, "downloads");
        public static bool EnableDebugInfo = true;
    }
}
