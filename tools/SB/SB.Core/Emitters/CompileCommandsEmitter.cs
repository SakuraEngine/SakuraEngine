using SB.Core;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public class CompileCommandsEmitter : TaskEmitter
    {
        public CompileCommandsEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppFileList>() || Target.HasFilesOf<CFileList>() || Target.HasFilesOf<ObjCppFileList>() || Target.HasFilesOf<ObjCFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppFileList>() || FileList.Is<CFileList>() || FileList.Is<ObjCppFileList>() || FileList.Is<ObjCFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            Stopwatch sw = new();
            sw.Start();

            CFamily Language = FileList.Is<ObjCppFileList>() ? CFamily.ObjCpp : FileList.Is<ObjCFileList>() ? CFamily.ObjC : FileList.Is<CppFileList>() ? CFamily.Cpp : CFamily.C;
            var SourceDependencies = Path.Combine(Target.GetStorePath(BS.DepsStore), BS.GetUniqueTempFileName(SourceFile, Target.Name + this.Name, "source.deps.json"));
            var ObjectFile = GetObjectFilePath(Target, SourceFile);
            var CLDriver = Toolchain.Compiler.CreateArgumentDriver(Language, false)
                .AddArguments(Target.Arguments)
                .MergeArguments(Options?.Arguments)
                .AddArgument("Source", SourceFile)
                .AddArgument("Object", ObjectFile)
                .AddArgument("SourceDependencies", SourceDependencies);

            var JSON = CLDriver.CompileCommands(Toolchain.Compiler, Target.Directory);
            CompileCommands.Add(JSON);

            sw.Stop();
            Time += (int)sw.ElapsedMilliseconds;
            return null;
        }

        public static void WriteToFile(string Path)
        {
            File.WriteAllText(Path, "[" + String.Join(",", CompileCommands) + "]");
        }

        private static string GetObjectFilePath(Target Target, string SourceFile) => Path.Combine(Target.GetStorePath(BuildSystem.ObjsStore), BuildSystem.GetUniqueTempFileName(SourceFile, Target.Name, "obj"));

        private static ConcurrentBag<string> CompileCommands = new();
        private IToolchain Toolchain { get; }
        public static volatile int Time = 0;
    }
}
