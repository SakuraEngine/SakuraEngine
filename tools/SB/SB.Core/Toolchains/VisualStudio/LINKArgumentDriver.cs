using System.IO;

namespace SB.Core
{
    using ArgumentName = string;
    using VS = VisualStudio;
    using BS = BuildSystem;
    public class LINKArgumentDriver : IArgumentDriver
    {
        public LINKArgumentDriver(bool ArchiverMode) => this.ArchiverMode = ArchiverMode;

        [TargetProperty] 
        public string RuntimeLibrary(string what) => VS.IsValidRT(what) ? what.StartsWith("MT") ? "/NODEFAULTLIB:msvcrt.lib" : "" : throw new ArgumentException($"Invalid argument \"{what}\" for MSVC RuntimeLibrary!");

        [TargetProperty] 
        public string TargetType(TargetType type) => typeMap.TryGetValue(type, out var t) ? t : throw new ArgumentException($"Invalid target type \"{type}\" for MSVC Linker!");
        static readonly Dictionary<TargetType, string> typeMap = new Dictionary<TargetType, string> { { Core.TargetType.Static, "/LIB" }, { Core.TargetType.Dynamic, "/DLL" }, { Core.TargetType.Executable, "" }, { Core.TargetType.HeaderOnly, "" } };

        [TargetProperty(InheritBehavior = true, PathBehavior = true)] 
        public string[]? LinkDirs(ArgumentList<string> dirs) => dirs.All(x => BS.CheckPath(x, false) ? true : throw new ArgumentException($"Invalid link dir {x}!")) ? dirs.Select(dir => $"/LIBPATH:{dir}").ToArray() : null;
        
        [TargetProperty(InheritBehavior = true)] 
        public string[]? Link(ArgumentList<string> dirs) => dirs.Select(dir => Path.GetExtension(dir) != ".o" ?  $"{dir}.lib" : dir).ToArray();

        [TargetProperty(InheritBehavior = true)]
        public string[]? WholeArchive(ArgumentList<string> libs) => libs.Select(lib => $"/WHOLEARCHIVE:\"{lib}.lib\"").ToArray();

        [TargetProperty(InheritBehavior = true)]
        public string[]? MSVC_NoDefaultLibrary(ArgumentList<string> libs) => libs.Select(lib => lib.Length > 0 ? $"/NODEFAULTLIB:{lib}.lib" : "").ToArray();

        [TargetProperty(InheritBehavior = true)]
        public string[]? MSVC_LinkerArgs(ArgumentList<string> libs) => libs.ToArray();

        public string Arch(Architecture arch) => archMap.TryGetValue(arch, out var r) ? r : throw new ArgumentException($"Invalid architecture \"{arch}\" for LINK.exe!");
        static readonly Dictionary<Architecture, string> archMap = new Dictionary<Architecture, string> { { Architecture.X86, "/MACHINE:X86" }, { Architecture.X64, "/MACHINE:X64" }, { Architecture.ARM64, "/MACHINE:ARM64" } };

        [TargetProperty]
        public string DebugSymbols(bool Enable) => Enable && !ArchiverMode ? "/DEBUG:FULL" : "";

        [TargetProperty]
        public virtual string DynamicDebug(bool v) => v ? "/DEBUG:FULL /dynamicdeopt" : "";

        public string PDB(string path) => BS.CheckPath(path, false) ? $"/PDB:\"{path}\"" : throw new ArgumentException($"PDB value {path} is not a valid absolute path!");

        public string[] Inputs(ArgumentList<string> inputs) => inputs.Select(dir => $"\"{dir}\"").ToArray();

        public string Output(string output) => BS.CheckFile(output, false) ? $"/OUT:\"{output}\"" : throw new ArgumentException($"Invalid output file path {output}!");

        private readonly bool ArchiverMode;
        public ArgumentDictionary Arguments { get; } = new ArgumentDictionary();
        public HashSet<string> RawArguments { get; } = new HashSet<string> { "/NOLOGO" };
    }
}