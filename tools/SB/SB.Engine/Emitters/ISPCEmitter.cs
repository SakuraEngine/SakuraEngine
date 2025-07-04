using SB.Core;
using Serilog;

namespace SB
{
    using BS = BuildSystem;
    [Doctor<ISPCDoctor>]
    public class ISPCEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<ISPCFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<ISPCFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            var GeneratedObj = GetObjectFile(SourceFile);
            GeneratedObj = Path.Combine(GetObjectDirectory(Target), GeneratedObj);
            var GeneratedHeader = GetGeneratedHeaderFile(Target, SourceFile);
            var Arguments = new string[] {
                "-O2",
                SourceFile,
                $"-o {GeneratedObj}",
                $"-h {GeneratedHeader}",
                "--target=host",
                "--opt=fast-math"
            };
            bool Changed = Engine.ConfigureAwareDepend.OnChanged(Target.Name, SourceFile, Name, (Depend depend) => {
                int ExitCode = BuildSystem.RunProcess(ISPCDoctor.ISPC!, string.Join(" ", Arguments), out var Output, out var Error);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile ISPC for {SourceFile} failed with fatal error!", $"ISPC.exe: {Error}");
                }
                depend.ExternalFiles.Add(GeneratedObj);
                depend.ExternalFiles.Add(GeneratedHeader);
            }, new string [] { SourceFile }, Arguments);
            return new PlainArtifact { IsRestored = !Changed };
        }
        public static string GetObjectDirectory(Target Target) => Path.Combine(Target.GetStorePath(BS.ObjsStore));
        public static string GetObjectFile(string SourceFile) => Path.GetFileNameWithoutExtension(SourceFile) + ".o";
        public static string GetGeneratedHeadersDir(Target Target) => Path.Combine(Target.GetCodegenDirectory(), "ispc-includes");
        public static string GetGeneratedHeaderFile(Target Target, string SourceFile) => Path.Combine(GetGeneratedHeadersDir(Target), Path.GetFileName(SourceFile) + ".h");
    }

    public class ISPCFileList : FileList {}

    public static partial class TargetExtensions
    {
        public static Target AddISPCFiles(this Target @this, params string[] Files)
        {
            var EarlyResolveList = new ISPCFileList { Target = @this };
            EarlyResolveList.AddFiles(Files);
            EarlyResolveList.GlobFiles();
            var ResolvedFiles = EarlyResolveList.Files;

            var HeadersDir = ISPCEmitter.GetGeneratedHeadersDir(@this);
            var ObjectsDir = @this.GetStorePath(BS.ObjsStore);
            Directory.CreateDirectory(HeadersDir);
            @this.IncludeDirs(Visibility.Private, HeadersDir);
            @this.FileList<ISPCFileList>().AddFiles(ResolvedFiles.ToArray());
            @this.LinkDirs(Visibility.Private, ISPCEmitter.GetObjectDirectory(@this));
            @this.Link(Visibility.Private, ResolvedFiles.Select(f => ISPCEmitter.GetObjectFile(f)).ToArray());
            return @this;
        }
    }

    public class ISPCDoctor : IDoctor
    {
        public bool Check()
        {
            var Installation = Install.Tool("ispc-1.26.0");
            Installation.Wait();
            ISPC = Path.Combine(Installation.Result, BuildSystem.HostOS == OSPlatform.Windows ? "ispc.exe" : "ispc");
            return true;
        }
        public bool Fix() 
        { 
            Log.Fatal("ispc sdks install failed!");
            return true; 
        }
        public static string? ISPC { get; private set; }
    }
}