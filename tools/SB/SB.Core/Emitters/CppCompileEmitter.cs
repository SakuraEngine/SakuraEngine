using SB.Core;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public class CppCompileAttribute
    {
        public ConcurrentBag<string> ObjectFiles = new();
    }

    public class CppCompileEmitter : TaskEmitter
    {
        public CppCompileEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppFileList>() || Target.HasFilesOf<CFileList>() || Target.HasFilesOf<ObjCppFileList>() || Target.HasFilesOf<ObjCFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppFileList>() || FileList.Is<CFileList>() || FileList.Is<ObjCppFileList>() || FileList.Is<ObjCFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            Stopwatch sw = new();
            sw.Start();

            CFamily Language = FileList.Is<ObjCppFileList>() ? CFamily.ObjCpp : FileList.Is<ObjCFileList>() ? CFamily.ObjC : FileList.Is<CppFileList>() ? CFamily.Cpp : CFamily.C;
            var SourceDependencies = Path.Combine(Target.GetStorePath(BS.DepsStore), BS.GetUniqueTempFileName(SourceFile, Target.Name + this.Name, "source.deps.json"));
            var ObjectFile = GetObjectFilePath(Target, SourceFile);
            var CompilerDriver = Toolchain.Compiler.CreateArgumentDriver(Language, false)
                .AddArguments(Target.Arguments)
                .MergeArguments(Options?.Arguments)
                .AddArgument("Source", SourceFile)
                .AddArgument("Object", ObjectFile)
                .AddArgument("SourceDependencies", SourceDependencies);
            if (SourceFile.EndsWith(".cpp") || SourceFile.EndsWith(".cc"))
            {
                var UsePCH = Target.GetAttribute<UsePCHAttribute>();
                if (UsePCH?.CppPCHAST is not null)
                {
                    CompilerDriver.AddArgument("UsePCHAST", UsePCH.CppPCHAST);
                }
            }
            var R = Toolchain.Compiler.Compile(this, Target, CompilerDriver);
            var CompileAttribute = Target.GetAttribute<CppCompileAttribute>()!;
            CompileAttribute.ObjectFiles.Add(ObjectFile);

            sw.Stop();
            Time += (int)sw.ElapsedMilliseconds;
            return R;
        }

        public static string GetObjectFilePath(Target Target, string SourceFile) => Path.Combine(Target.GetStorePath(BS.ObjsStore), BS.GetUniqueTempFileName(SourceFile, Target.Name, "obj"));

        private IToolchain Toolchain { get; }
        public static volatile int Time = 0;
    }

    public class UnityBuildableFileOptions : FileOptions
    {
        public bool DisableUnityBuild = false;
        public string UnityGroup = "";
    }

    public class CFamilyFileOptions : UnityBuildableFileOptions {}
    public class CFileList : FileList {}
    public class CppFileList : FileList {}
    public class ObjCFileList : FileList {}
    public class ObjCppFileList : FileList {}

    public static partial class TargetExtensions
    {
        public static Target AddCppFiles(this Target @this, params string[] Files)
        {
            @this.FileList<CppFileList>().AddFiles(Files);
            return @this;
        }

        public static Target AddCFiles(this Target @this, params string[] Files)
        {
            @this.FileList<CFileList>().AddFiles(Files);
            return @this;
        }

        public static Target AddObjCppFiles(this Target @this, params string[] Files)
        {
            @this.FileList<ObjCppFileList>().AddFiles(Files);
            return @this;
        }

        public static Target AddObjCFiles(this Target @this, params string[] Files)
        {
            @this.FileList<ObjCFileList>().AddFiles(Files);
            return @this;
        }

        public static Target AddCppFiles(this Target @this, CFamilyFileOptions Options, params string[] Files)
        {
            @this.FileList<CppFileList>().AddFiles(Options, Files);
            return @this;
        }

        public static Target AddCFiles(this Target @this, CFamilyFileOptions Options, params string[] Files)
        {
            @this.FileList<CFileList>().AddFiles(Options, Files);
            return @this;
        }

        public static Target AddObjCppFiles(this Target @this, CFamilyFileOptions Options, params string[] Files)
        {
            @this.FileList<ObjCppFileList>().AddFiles(Options, Files);
            return @this;
        }

        public static Target AddObjCFiles(this Target @this, CFamilyFileOptions Options, params string[] Files)
        {
            @this.FileList<ObjCFileList>().AddFiles(Options, Files);
            return @this;
        }
    }
}