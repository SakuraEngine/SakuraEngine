using SB.Core;
using Serilog;

namespace SB
{
    [DXCDoctor]
    public class DXCEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<HLSLFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<HLSLFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            var HLSLFileList = FileList as HLSLFileList;
            var Parts = Path.GetFileName(SourceFile).Split('.');
            var HLSLBaseName = Parts[0];
            var TargetProfile = Parts[1];
            var OutputDirectory = Path.Combine(Engine.BuildPath, ShaderOutputDirectories[Target.Name]);
            Directory.CreateDirectory(OutputDirectory);

            // SPV
            bool Changed = Depend.OnChanged(Target.Name, SourceFile, Name + ".SPV", (Depend depend) => {
                var SpvFile = Path.Combine(OutputDirectory, $"{HLSLBaseName}.spv");
                var Arguments = new string[] {
                    "-Wno-ignored-attributes",
                    "-all_resources_bound",
                    "-spirv",
                    $"-E {HLSLFileList!.Entry}",
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
            Changed |= Depend.OnChanged(Target.Name, SourceFile, Name + ".DXIL", (Depend depend) => {
                var DxilFile = Path.Combine(OutputDirectory, $"{HLSLBaseName}.dxil");
                var Arguments = new string[] {
                    "-Wno-ignored-attributes",
                    "-all_resources_bound",
                    $"-E {HLSLFileList!.Entry}",
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
            DXCEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target DXCOutputDirectory(this Target @this, string Directory)
        {
            DXCEmitter.ShaderOutputDirectories[@this.Name] = Directory;
            return @this;
        }
    }

    public class DXCDoctor : DoctorAttribute
    {
        public override bool Check()
        {
            var Installation = (BuildSystem.TargetOS == OSPlatform.Windows) ? Install.Tool("dxc-2025_02_21") : Install.Tool("dxc");
            Installation.Wait();
            DXC = Path.Combine(Installation.Result, BuildSystem.HostOS == OSPlatform.Windows ? "dxc.exe" : "dxc");
            Directory.CreateDirectory(Path.Combine(Engine.BuildPath, "resources/shaders"));
            return true;
        }
        public override bool Fix() 
        { 
            Log.Fatal("dxc sdks install failed!");
            return true; 
        }
        public static string? DXC { get; private set; }
    }
}