using SB.Core;
using Serilog;

namespace SB
{
    [Doctor<DXCDoctor>]
    public class DXCEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<HLSLFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<HLSLFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            var HLSLFileList = FileList as HLSLFileList;
            var OutputDirectory = Path.Combine(Engine.BuildPath, ShaderOutputDirectories[Target.Name]);
            return CompileHLSL(Target, SourceFile, HLSLFileList!.Entry, OutputDirectory);
        }

        public static IArtifact? CompileHLSL(Target Target, string SourceFile, string Entry, string OutputDirectory)
        {
            bool AppendEntryInArtifactPath = false;
            var Parts = Path.GetFileName(SourceFile).Split('.');
            var HLSLBaseName = Parts[0];
            var TargetProfile = Parts[1];
            if (Parts.Length > 3)
            {
                Entry = Parts[1];
                TargetProfile = Parts[2];
                AppendEntryInArtifactPath = true;
            }
            Directory.CreateDirectory(OutputDirectory);

            // SPV
            bool Changed = Engine.ConfigureNotAwareDepend.OnChanged(Target.Name, SourceFile, "DXC.SPV", (Depend depend) => {
                var SpvFile = Path.Combine(OutputDirectory, AppendEntryInArtifactPath ? $"{HLSLBaseName}.{Entry}.spv" : $"{HLSLBaseName}.spv");
                var Arguments = new string[] {
                    "-HV 2021",
                    "-Wno-ignored-attributes",
                    "-spirv",
                    TargetProfile.StartsWith("lib") ? "-Vd" : $"-E {Entry}",
                    "-fspv-target-env=vulkan1.1",
                    $"-Fo{SpvFile}",
                    $"-T{TargetProfile}",
                    SourceFile
                };
                int ExitCode = BuildSystem.RunProcess(DXCDoctor.DXC!, string.Join(" ", Arguments), out var Output, out var Error);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile SPV for {SourceFile} failed with fatal error!", $"DXC.exe: {Error}");
                }
                depend.ExternalFiles.Add(SpvFile);
            }, new string [] { SourceFile }, null);

            // DXIL
            Changed |= Engine.ConfigureNotAwareDepend.OnChanged(Target.Name, SourceFile, "DXC.DXIL", (Depend depend) => {
                var DxilFile = Path.Combine(OutputDirectory, AppendEntryInArtifactPath? $"{HLSLBaseName}.{Entry}.dxil" : $"{HLSLBaseName}.dxil");
                var Arguments = new string[] {
                    "-HV 2021",
                    "-Wno-ignored-attributes",
                    "-all_resources_bound",
                    TargetProfile.StartsWith("lib") ? "-Vd" : $"-E {Entry}",
                    $"-Fo{DxilFile}",
                    $"-T{TargetProfile}",
                    SourceFile
                };
                int ExitCode = BuildSystem.RunProcess(DXCDoctor.DXC!, string.Join(" ", Arguments), out var Output, out var Error);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile DXIL for {SourceFile} failed with fatal error!", $"DXC.exe: {Error}");
                }
                depend.ExternalFiles.Add(DxilFile);
            }, new string [] { SourceFile }, null);

            return new PlainArtifact { IsRestored = !Changed };
        }

        public static Dictionary<string, string> ShaderOutputDirectories = new();
    }

    public class HLSLFileList : FileList
    {
        internal string Entry = "main";
    }

    public static partial class TargetExtensions
    {
        public static Target AddHLSLFiles(this Target @this, params string[] Files)
        {
            @this.FileList<HLSLFileList>().AddFiles(Files);
            DXCEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target AddHLSLFilesWithEntry(this Target @this, string Entry, params string[] Files)
        {
            if (Files.Length == 0)
            {
                Log.Fatal("No HLSL files provided for target {TargetName}!", @this.Name);
                throw new ArgumentException("No HLSL files provided.");
            }
            
            var FL = @this.FileList<HLSLFileList>();
            FL.Entry = Entry;
            FL.AddFiles(Files);
            if (!DXCEmitter.ShaderOutputDirectories.ContainsKey(@this.Name))
                DXCEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target DXCOutputDirectory(this Target @this, string Directory)
        {
            DXCEmitter.ShaderOutputDirectories[@this.Name] = Directory;
            return @this;
        }
    }

    public class DXCDoctor : IDoctor
    {
        public bool Check()
        {
            var Installation = (BuildSystem.TargetOS == OSPlatform.Windows) ? Install.Tool("dxc-2025_02_21") : Install.Tool("dxc");
            Installation.Wait();
            DXC = Path.Combine(Installation.Result, BuildSystem.HostOS == OSPlatform.Windows ? "dxc.exe" : "dxc");
            Directory.CreateDirectory(Path.Combine(Engine.BuildPath, "resources/shaders"));
            return true;
        }
        public bool Fix()
        {
            Log.Fatal("dxc sdks install failed!");
            return true; 
        }
        public static string? DXC { get; private set; }
    }
}