using SB.Core;
using Serilog;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public class CodegenMetaAttribute
    {
        public required string RootDirectory { get; set; }
        public string? MetaDirectory { get; internal set; }
        public string[]? AllGeneratedMetaFiles { get; internal set; }
    }

    [MetaDoctor]
    public class CodegenMetaEmitter : TaskEmitter
    {
        public CodegenMetaEmitter(IToolchain Toolchain)
        {
            this.Toolchain = Toolchain;
        }
        public override bool EnableEmitter(Target Target) => Target.HasAttribute<CodegenMetaAttribute>() && Target.HasFilesOf<MetaHeaderList>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            // Ensure output file
            var MetaAttribute = Target.GetAttribute<CodegenMetaAttribute>()!;
            MetaAttribute.MetaDirectory = Path.Combine(Target.GetStorePath(BS.GeneratedSourceStore), "meta_database");
            Directory.CreateDirectory(MetaAttribute.MetaDirectory);
            var BatchDirectory = Path.Combine(Target.GetStorePath(BS.GeneratedSourceStore), "ReflectionBatch");
            Directory.CreateDirectory(BatchDirectory);

            // Batch headers to a source file
            var Headers = Target.FileList<MetaHeaderList>().Files;
            var BatchFile = Path.Combine(BatchDirectory, "ReflectionBatch.cpp");
            Depend.OnChanged(Target.Name, BatchFile, Name, (Depend depend) => {
                Directory.CreateDirectory(BatchDirectory);
                File.WriteAllLines(BatchFile, Headers.Select(H => $"#include \"{H}\""));
                depend.ExternalFiles.Add(BatchFile);
            }, Headers, null);

            // Set meta args
            var MetaArgs = new List<string>{
                BatchFile,
                $"--output={MetaAttribute.MetaDirectory}",
                $"--root={MetaAttribute.RootDirectory}",
                "--"
            };
            // Compiler arguments
            var ArgsWithoutPCH = Target.Arguments.ToDictionary();
            ArgsWithoutPCH.Remove("UsePCHAST");
            var CompilerArgs = Toolchain.Compiler.CreateArgumentDriver(CFamily.Cpp, false)
                .AddArguments(ArgsWithoutPCH)
                .CalculateArguments()
                .Values.SelectMany(x => x).ToList();
            // Set addon flags            
            CompilerArgs.Add("-Wno-abstract-final-class");
            if (BS.TargetOS == OSPlatform.Windows)
                CompilerArgs.Add("--driver-mode=cl");
            MetaArgs.AddRange(CompilerArgs);
            MetaArgs.AddRange(
                "-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include/c++/v1",
                "-isystem /Library/Developer/CommandLineTools/usr/lib/clang/17/include",
                "-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include",
                "-isystem /Library/Developer/CommandLineTools/usr/include",
                "-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/System/Library/Frameworks"
            );
            // Run meta.exe
            bool Changed = Depend.OnChanged(Target.Name, MetaAttribute.MetaDirectory, Name, (Depend depend) =>
            {
                var EXE = Path.Combine(MetaDoctor.Installation!.Result, BS.HostOS == OSPlatform.Windows ? "meta.exe" : "meta");

                int ExitCode = BS.RunProcess(EXE, string.Join(" ", MetaArgs), out var OutputInfo, out var ErrorInfo);
                if (ExitCode != 0)
                    throw new TaskFatalError($"meta.exe {BatchFile} failed with fatal error!", $"meta.exe: {ErrorInfo}");
                else if (OutputInfo.Contains("warning LNK"))
                    Log.Warning("meta.exe: {OutputInfo}", OutputInfo);

                MetaAttribute.AllGeneratedMetaFiles = Directory.GetFiles(MetaAttribute.MetaDirectory, "*.meta", SearchOption.AllDirectories);
                depend.ExternalFiles.AddRange(MetaAttribute.AllGeneratedMetaFiles);
            }, new string[] { BatchFile }, MetaArgs);

            if (!Changed)
                MetaAttribute.AllGeneratedMetaFiles = Directory.GetFiles(MetaAttribute.MetaDirectory, "*.meta", SearchOption.AllDirectories);
            return new PlainArtifact { IsRestored = !Changed };
        }

        private IToolchain Toolchain { get; }
        public static volatile int Time = 0;
    }

    public class MetaDoctor : DoctorAttribute
    {
        public override bool Check()
        {
            Installation = Install.Tool("meta_v1.0.3-llvm_19.1.7");
            Installation!.Wait();
            return true;
        }
        public override bool Fix() 
        { 
            Log.Fatal("meta install failed!");
            return true; 
        }
        public static Task<string>? Installation;
    }

    public class MetaHeaderList : FileList {}

    public static partial class TargetExtensions
    {
        public static Target AddMetaHeaders(this Target @this, params string[] Files)
        {
            @this.FileList<MetaHeaderList>().AddFiles(Files);
            return @this;
        }
    }
}