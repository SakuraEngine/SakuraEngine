using SB.Core;
using Serilog;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public class CppLinkAttribute
    {
        public List<string> BypassLinks { get; init; } = new();
        public List<string> BypassLinkDirs { get; init; } = new();
    }

    public class CppLinkEmitter : TaskEmitter
    {
        public CppLinkEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppFileList>() || Target.HasFilesOf<CFileList>() || Target.HasFilesOf<ObjCppFileList>() || Target.HasFilesOf<ObjCFileList>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            var CppLinkAttr = Target.GetAttribute<CppLinkAttribute>()!;
            var TT = Target.GetTargetType();
            Stopwatch sw = new();
            sw.Start();

            var LinkedFileName = GetLinkedFileName(Target);
            var DependFile = Path.Combine(Target.GetBuildSrcDepsDir(), BS.GetUniqueTempFileName(LinkedFileName, Target.Name + this.Name, "task.deps.json"));
            var Inputs = new ArgumentList<string>();
            // Add obj files
            var SourceFiles = Target.FileList<CppFileList>().Files.ToList();
            SourceFiles.AddRange(Target.FileList<CFileList>().Files.ToList());
            if (BuildSystem.TargetOS == OSPlatform.OSX)
            {
                SourceFiles.AddRange(Target.FileList<ObjCFileList>().Files.ToList());
                SourceFiles.AddRange(Target.FileList<ObjCppFileList>().Files.ToList());
            }
            Inputs.AddRange(SourceFiles.Select(F => CppCompileEmitter.GetObjectFilePath(Target, F)));

            if (TT == TargetType.Dynamic || TT == TargetType.Executable)
            {
                var TargetArguments = Target.Arguments.ToDictionary();
                // Add dep obj files
                Inputs.AddRange(Target.IgnoreVisibilityAllDependencies.Where(
                    Dep => BS.GetTarget(Dep)?.GetTargetType() == TargetType.Objects
                ).SelectMany(
                    Dep => BS.GetTarget(Dep)!.GetAttribute<CppCompileAttribute>()!.ObjectFiles
                ));
                // Add lib file of dep target
                Inputs.AddRange(
                    Target.IgnoreVisibilityAllDependencies.Select(Dependency => GetStubFileName(BS.GetTarget(Dependency)!)).Where(Stub => Stub != null).Select(Stub => Stub!)
                );
                // Collect 'Links' & 'LinkDirs' from static targets
                var Links = TargetArguments.GetOrAddNew<string, ArgumentList<string>>("Link");
                var LinkDirs = TargetArguments.GetOrAddNew<string, ArgumentList<string>>("LinkDirs");
                Links.AddRange(
                    Target.IgnoreVisibilityAllDependencies.Select(Dependency => BS.GetTarget(Dependency)!.GetAttribute<CppLinkAttribute>()!).SelectMany(A => A.BypassLinks)
                );
                LinkDirs.AddRange(
                    Target.IgnoreVisibilityAllDependencies.Select(Dependency => BS.GetTarget(Dependency)!.GetAttribute<CppLinkAttribute>()!).SelectMany(A => A.BypassLinkDirs)
                );
                // Link
                if (Inputs.Count != 0)
                {
                    var LINKDriver = Toolchain.Linker.CreateArgumentDriver()
                        .AddArguments(TargetArguments)
                        .AddArgument("Inputs", Inputs)
                        .AddArgument("Output", LinkedFileName);
                    if (TargetArguments.TryGetValue("DebugSymbols", out var Enable) && (bool)Enable!)
                    {
                        if (BS.TargetOS == OSPlatform.Windows)
                        {
                            LINKDriver.AddArgument("PDB", LinkedFileName.Replace("dll", "pdb").Replace("exe", "pdb"));
                        }
                    }
                    return Toolchain.Linker.Link(this, Target, LINKDriver);
                }
            }
            else if (TT == TargetType.Static)
            {
                // Archive only
                if (Inputs.Count != 0)
                {
                    var ARDriver = Toolchain.Archiver.CreateArgumentDriver()
                        .AddArguments(Target.Arguments)
                        .AddArgument("Inputs", Inputs)
                        .AddArgument("Output", LinkedFileName);
                    return Toolchain.Archiver.Archive(this, Target, ARDriver);
                }
            }
            if (TT == TargetType.Static || TT == TargetType.Objects || TT == TargetType.HeaderOnly)
            {
                // bypass 'Link' vars (include private)
                if (Target.Arguments.TryGetValue("Link", out var LinkArgs))
                {
                    var Links = LinkArgs as ArgumentList<string>;
                    CppLinkAttr.BypassLinks.AddRange(Links!.ToList());
                }
                // bypass 'LinkDir' vars
                if (Target.Arguments.TryGetValue("LinkDirs", out var LinkDirArgs))
                {
                    var LinkDirs = LinkDirArgs as ArgumentList<string>;
                    CppLinkAttr.BypassLinkDirs.AddRange(LinkDirs!.ToList());
                }
            }
            sw.Stop();
            Time += (int)sw.ElapsedMilliseconds;
            return null;
        }

        public static string GetLinkedFileName(Target Target)
        {
            var OutputType = Target.GetTargetType();
            var Extension = GetPlatformLinkedFileExtension(OutputType);
            var OutputFile = Path.Combine(Target.GetBinaryDir(), $"{Target.Name}{Extension}");
            return OutputFile;
        }

        public static string? GetStubFileName(Target Target)
        {
            var OutputType = Target.GetTargetType();
            var Extension = GetPlatformStubFileExtension(OutputType);
            if (Extension.Length == 0)
                return null;
            var OutputFile = Path.Combine(Target.GetBinaryDir(), $"{Target.Name}{Extension}");
            return OutputFile;
        }

        public static string GetPlatformLinkedFileExtension(TargetType? Type)
        {
            if (BS.TargetOS == OSPlatform.Windows)
                return Type switch
                {
                    TargetType.Static => ".lib",
                    TargetType.Dynamic => ".dll",
                    TargetType.Executable => ".exe",
                    _ => ""
                };
            else if (BS.TargetOS == OSPlatform.OSX)
                return Type switch
                {
                    TargetType.Static => ".a",
                    TargetType.Dynamic => ".dylib",
                    TargetType.Executable => "",
                    _ => ""
                };
            else if (BS.TargetOS == OSPlatform.Linux)
                return Type switch
                {
                    TargetType.Static => ".a",
                    TargetType.Dynamic => ".so",
                    TargetType.Executable => "",
                    _ => ""
                };
            return "";
        }

        private static string GetPlatformStubFileExtension(TargetType? Type)
        {
            if (BS.TargetOS == OSPlatform.Windows)
                return Type switch
                {
                    TargetType.Static => ".lib",
                    TargetType.Dynamic => ".lib",
                    _ => ""
                };
            else if (BS.TargetOS == OSPlatform.OSX)
                return Type switch
                {
                    TargetType.Static => ".a",
                    TargetType.Dynamic => ".dylib",
                    _ => ""
                };
            else if (BS.TargetOS == OSPlatform.Linux)
                return Type switch
                {
                    TargetType.Static => ".a",
                    TargetType.Dynamic => ".a",
                    _ => ""
                };
            return "";
        }

        private IToolchain Toolchain { get; }
        public static volatile int Time = 0;
    }
}
