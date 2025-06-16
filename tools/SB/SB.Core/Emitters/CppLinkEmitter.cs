using SB.Core;
using Serilog;
using System.Diagnostics;

namespace SB
{
    using BS = BuildSystem;
    public class CppLinkAttribute
    {
        public List<string> LinkOnlys { get; init; } = new();
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
            if (TT != TargetType.HeaderOnly && TT != TargetType.Objects)
            {
                Stopwatch sw = new();
                sw.Start();

                var LinkedFileName = GetLinkedFileName(Target);
                var DependFile = Path.Combine(Target.GetStorePath(BS.DepsStore), BS.GetUniqueTempFileName(LinkedFileName, Target.Name + this.Name, "task.deps.json"));
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
                // Add dep obj files
                Inputs.AddRange(Target.Dependencies.Where(
                    Dep => BS.GetTarget(Dep)?.GetTargetType() == TargetType.Objects
                ).SelectMany(
                    Dep => BS.GetTarget(Dep)!.GetAttribute<CppCompileAttribute>()!.ObjectFiles
                ));
                if (TT != TargetType.Static)
                {
                    Inputs.AddRange(
                        Target.Dependencies.Select(Dependency => GetStubFileName(BS.GetTarget(Dependency)!)).Where(Stub => Stub != null).Select(Stub => Stub!)
                    );
                    // Collect links from static targets
                    Inputs.AddRange(
                        Target.Dependencies.Select(Dependency => BS.GetTarget(Dependency)!.GetAttribute<CppLinkAttribute>()!).SelectMany(A => A.LinkOnlys)
                    );
                    if (Inputs.Count != 0)
                    {
                        var LINKDriver = Toolchain.Linker.CreateArgumentDriver()
                            .AddArguments(Target.Arguments)
                            .AddArgument("Inputs", Inputs)
                            .AddArgument("Output", LinkedFileName);
                        if (Target.Arguments.TryGetValue("DebugSymbols", out var Enable) && (bool)Enable!)
                        {
                            if (BS.TargetOS == OSPlatform.Windows)
                            {
                                LINKDriver.AddArgument("PDB", LinkedFileName.Replace("dll", "pdb").Replace("exe", "pdb"));
                            }
                        }
                        return Toolchain.Linker.Link(this, Target, LINKDriver);
                    }
                }
                else
                {
                    // Add dep links.Static libraries don’t really have a link phase. 
                    // Very cruedly they are just an archive of object files that will be propagated to a real link line ( creation of a shared library or executable ).
                    CppLinkAttr.LinkOnlys.AddRange(
                        Target.Dependencies.Select(Dependency => GetStubFileName(BS.GetTarget(Dependency)!)).Where(Stub => Stub != null).Select(Stub => Stub!)
                    );
                    if (Inputs.Count != 0)
                    {
                        var ARDriver = Toolchain.Archiver.CreateArgumentDriver()
                            .AddArguments(Target.Arguments)
                            .AddArgument("Inputs", Inputs)
                            .AddArgument("Output", LinkedFileName);
                        return Toolchain.Archiver.Archive(this, Target, ARDriver);
                    }
                }
                sw.Stop();
                Time += (int)sw.ElapsedMilliseconds;
            }
            return null;
        }

        public static string GetLinkedFileName(Target Target)
        {
            var OutputType = Target.GetTargetType();
            var Extension = GetPlatformLinkedFileExtension(OutputType);
            var OutputFile = Path.Combine(Target.GetBinaryPath(), $"{Target.Name}{Extension}");
            return OutputFile;
        }

        public static string? GetStubFileName(Target Target)
        {
            var OutputType = Target.GetTargetType();
            var Extension = GetPlatformStubFileExtension(OutputType);
            if (Extension.Length == 0)
                return null;
            var OutputFile = Path.Combine(Target.GetBinaryPath(), $"{Target.Name}{Extension}");
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
