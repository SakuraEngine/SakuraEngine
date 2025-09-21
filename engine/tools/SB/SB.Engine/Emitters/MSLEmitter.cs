using SB.Core;
using Serilog;

namespace SB
{
    [Setup<MSLSetup>]
    public class MSLEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => BuildSystem.HostOS == OSPlatform.OSX && Target.HasFilesOf<MetalFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<MetalFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            var MetalFileList = FileList as MetalFileList;
            var OutputDirectory = Path.Combine(BuildDirs.BuildDir, ShaderOutputDirectories[Target.Name]);
            return CompileMetal(Target, SourceFile, MetalFileList!.Entry, OutputDirectory);
        }

        public static IArtifact? CompileMetal(Target Target, string SourceFile, string Entry, string OutputDirectory)
        {
            bool AppendEntryInArtifactPath = false;
            var Parts = Path.GetFileName(SourceFile).Split('.');
            var MetalBaseName = Parts[0];
            var ShaderType = Parts.Length > 1 ? Parts[1] : "vs"; // Default to vertex shader
            if (Parts.Length > 3)
            {
                Entry = Parts[1];
                ShaderType = Parts[2];
                AppendEntryInArtifactPath = true;
            }
            Directory.CreateDirectory(OutputDirectory);

            // Compile to AIR (Apple Intermediate Representation)
            bool Changed = EngineDepends.ShaderCompile.OnChanged(Target.Name, SourceFile, "MSL.AIR", (Depend depend) => {
                var AirFile = Path.Combine(OutputDirectory, AppendEntryInArtifactPath ? $"{MetalBaseName}.{Entry}.air" : $"{MetalBaseName}.air");
                
                // Determine Metal standard version based on macOS version
                var MetalStd = GetMetalStandard();
                
                var Arguments = new string[] {
                    "-sdk", "macosx",
                    "metal",
                    "-c",
                    $"-std={MetalStd}",
                    "-O3", // Optimization level
                    "-ffast-math",
                    "-o", AirFile,
                    SourceFile
                };
                
                int ExitCode = BuildSystem.RunProcess("xcrun", string.Join(" ", Arguments), out var Output, out var Error);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile Metal AIR for {SourceFile} failed with fatal error!", $"xcrun metal: {Error}");
                }
                depend.ExternalFiles.Add(AirFile);
            }, new string[] { SourceFile }, null);

            // Link AIR to create Metal library
            Changed |= EngineDepends.ShaderCompile.OnChanged(Target.Name, SourceFile, "MSL.METALLIB", (Depend depend) => {
                var AirFile = Path.Combine(OutputDirectory, AppendEntryInArtifactPath ? $"{MetalBaseName}.{Entry}.air" : $"{MetalBaseName}.air");
                var MetallibFile = Path.Combine(OutputDirectory, AppendEntryInArtifactPath ? $"{MetalBaseName}.{Entry}.metallib" : $"{MetalBaseName}.metallib");
                
                var Arguments = new string[] {
                    "-sdk", "macosx",
                    "metallib",
                    AirFile,
                    "-o", MetallibFile
                };
                
                int ExitCode = BuildSystem.RunProcess("xcrun", string.Join(" ", Arguments), out var Output, out var Error);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Link Metal library for {SourceFile} failed with fatal error!", $"xcrun metallib: {Error}");
                }
                depend.ExternalFiles.Add(MetallibFile);
            }, new string[] { SourceFile }, null);

            return new PlainArtifact { IsRestored = !Changed };
        }

        private static string GetMetalStandard()
        {
            return "metal3.2";
        }

        public static Dictionary<string, string> ShaderOutputDirectories = new();
    }

    public class MetalFileList : FileList
    {
        internal string Entry = "main";
    }

    public static partial class TargetExtensions
    {
        public static Target AddMetalFiles(this Target @this, params string[] Files)
        {
            @this.FileList<MetalFileList>().AddFiles(Files);
            MSLEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target AddMetalFilesWithEntry(this Target @this, string Entry, params string[] Files)
        {
            if (Files.Length == 0)
            {
                Log.Fatal("No Metal files provided for target {TargetName}!", @this.Name);
                throw new ArgumentException("No Metal files provided.");
            }
            
            var FL = @this.FileList<MetalFileList>();
            FL.Entry = Entry;
            FL.AddFiles(Files);
            if (!MSLEmitter.ShaderOutputDirectories.ContainsKey(@this.Name))
                MSLEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target MetalOutputDirectory(this Target @this, string Directory)
        {
            MSLEmitter.ShaderOutputDirectories[@this.Name] = Directory;
            return @this;
        }
    }

    public class MSLSetup : ISetup
    {
        public void Setup()
        {
            // xcrun is part of Xcode command line tools on macOS, no need to download
            if (BuildSystem.HostOS != OSPlatform.OSX)
                return;

            // Check if xcrun is available
            int ExitCode = BuildSystem.RunProcess("xcrun", "--version", out var Output, out var Error);
            if (ExitCode != 0)
            {
                throw new TaskFatalError("Xcode command line tools not found!", "Please install Xcode and run 'xcode-select --install'");
            }

            Directory.CreateDirectory(Path.Combine(BuildDirs.BuildDir, "resources/shaders"));
        }
    }
}