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
        public static string DefaultMode = "debug";
        public static string DefaultToolchain = OperatingSystem.IsWindows() ? "clang-cl" : "clang";

        public static IToolchain Bootstrap(TargetCategory Categories)
        {
            using (Profiler.BeginZone("Bootstrap", color: (uint)Profiler.ColorType.WebMaroon))
            {
                Log.Verbose("Runs on {HostOS} with {ProcessorCount} logical processors", HostOS, Environment.ProcessorCount);

                IToolchain? Toolchain = null;
                if (BS.HostOS == OSPlatform.Windows)
                    Toolchain = VisualStudioSetup.VisualStudio;
                else if (BS.HostOS == OSPlatform.OSX)
                    Toolchain = XCodeSetup.XCode;
                else
                    throw new Exception("Unsupported Platform!");

                if (BS.HostOS == OSPlatform.Windows)
                {
                    char DriveLetter = SourceLocation.Directory()[0];
                    if (Char.IsLower(DriveLetter))
                        throw new Exception($"Drive letter {DriveLetter} from source location must be upper case! You might compiled SB in git bash environment, please recompile it in cmd.exe or powershell.exe!");
                }

                BS.LoadConfigurations();
                BuildDirs.SetupToolchain(Toolchain);

                Log.Verbose("Load Targets... ");
                LoadTargets(Categories);

                Stopwatch sw = new();
                sw.Start();
                Log.Verbose("Run Setups... ");
                Engine.RunSetups();
                sw.Stop();
                Log.Verbose($"Setups Finished... cost {sw.ElapsedMilliseconds / 1000.0f}s");
                return Toolchain!;
            }
        }

        public static void AddCodegenEmitters(IToolchain Toolchain)
        {
            Engine.AddTaskEmitter("Module.Info", new ModuleInfoEmitter());
            Engine.AddTaskEmitter("Cpp.UnityBuild", new UnityBuildEmitter())
                .AddDependency("Module.Info", DependencyModel.PerTarget);

            Engine.AddTaskEmitter("Codgen.Meta", new CodegenMetaEmitter(Toolchain));
            Engine.AddTaskEmitter("Codgen.Codegen", new CodegenRenderEmitter(Toolchain))
                .AddDependency("Cpp.UnityBuild", DependencyModel.PerTarget)
                .AddDependency("Codgen.Meta", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Meta", DependencyModel.PerTarget);
        }

        public static void AddEngineTaskEmitters(IToolchain Toolchain)
        {
            Log.Verbose("Add Engine Task Emitters... ");

            Engine.AddTaskEmitter("Utils.CopyFiles", new CopyFilesEmitter());
            Engine.AddTaskEmitter("ISPC.Compile", new ISPCEmitter());

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

            Engine.AddTaskEmitter("Install.Artifact", new InstallArtifactEmitter())
                .AddDependency("Cpp.Link", DependencyModel.PerTarget);
        }

        public static void AddCompileCommandsEmitter(IToolchain Toolchain)
        {
            Engine.AddTaskEmitter("Cpp.CompileCommands", new CompileCommandsEmitter(Toolchain))
                .AddDependency("Module.Info", DependencyModel.PerTarget);

            Engine.GetTaskEmitter("Cpp.UnityBuild")
                ?.AddDependency("Cpp.CompileCommands", DependencyModel.PerTarget);

            Engine.AddTaskEmitter("CppSL.CompileCommands", new CppSLCompileCommandsEmitter());
        }

        public static void AddShaderTaskEmitters(IToolchain Toolchain)
        {
            if (BuildSystem.TargetOS == OSPlatform.Windows)
            {
                Engine.AddTaskEmitter("DXC.Compile", new DXCEmitter());
            }
            if (BuildSystem.TargetOS == OSPlatform.OSX)
            {
                Engine.AddTaskEmitter("MSL.Compile", new MSLEmitter());
            }
            Engine.AddTaskEmitter("CppSL.Compile", new CppSLEmitter())
                .AddTargetDependency("CppSLCompiler", "Install.Artifact")
                .AddDependency("Codgen.Codegen", DependencyModel.ExternalTarget)
                .AddDependency("Codgen.Codegen", DependencyModel.PerTarget);  // 依赖 CppSLCompiler 目标的链接步骤
        }

        public static new int RunBuild(string? singleTargetName = null)
        {
            return BS.RunBuild(singleTargetName);
        }

        private static void LoadTargets(TargetCategory Categories)
        {
            BS.TargetDefaultSettings += (Target Target) =>
            {
                Target.CppVersion("20")
                    .Exception(false)
                    .RTTI(false)
                    .Clang_CppFlags(Visibility.Public, "-Wno-unknown-warning-option")
                    .Clang_CppFlags(Visibility.Public, "-Wno-character-conversion")
                    .LinkDirs(Visibility.Public, Target.GetBinaryDir());
                if (BS.TargetOS == OSPlatform.Windows)
                {
                    Target.RuntimeLibrary("MD")
                        .MSVC_LinkerArgs(Visibility.Public, "/NODEFAULTLIB:library");
                }

                if (Target.IsFromPackage)
                {
                    var BuildDirectory = Path.Combine(SB.BuildDirs.BuildDir, $"{BS.TargetOS}-{BS.TargetArch}-{BS.GlobalConfiguration}");
                    Target.LinkDirs(Visibility.Public, BuildDirectory)
                        .InstallArtifact();
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
    }
}
