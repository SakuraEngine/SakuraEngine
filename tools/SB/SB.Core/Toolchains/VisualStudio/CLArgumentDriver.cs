namespace SB.Core
{
    using ArgumentName = string;
    using VS = VisualStudio;
    using BS = BuildSystem;
    public class CLArgumentDriver : IArgumentDriver
    {
        public CLArgumentDriver(CFamily Language, bool isPCH)
        {
            this.Language = Language;
            this.isPCH = isPCH;
            if (Language == CFamily.C)
            {
                RawArguments.Add("/TC");
            }
            else if (Language == CFamily.Cpp)
            {
                RawArguments.Add("/TP");
                RawArguments.Add("/Zc:__cplusplus");
            }
        }

        [TargetProperty] 
        public virtual string[] Exception(bool Enable) => Enable ? new string[] { "/EHsc", "-D_HAS_EXCEPTIONS=1" } : new string[] { "/EHsc-", "-D_HAS_EXCEPTIONS=0" };

        [TargetProperty] 
        public virtual string RuntimeLibrary(string what) => VS.IsValidRT(what) ? $"/{what}" : throw new TaskFatalError($"Invalid argument \"{what}\" for MSVC RuntimeLibrary!");

        [TargetProperty] 
        public virtual string CppVersion(string what) => 
            Language == CFamily.Cpp ?
            cppVersionMap.TryGetValue(what.Replace("c++", "").Replace("C++", ""), out var r) ? r : throw new TaskFatalError($"Invalid argument \"{what}\" for CppVersion!") :
            "";
        public static readonly Dictionary<string, string> cppVersionMap = new Dictionary<string, string> { { "11", "/std:c++11" }, { "14", "/std:c++14" }, { "17", "/std:c++17" }, { "20", "/std:c++20" }, { "23", "/std:c++23" }, { "latest", "/std:c++latest" } };
        
        [TargetProperty] 
        public virtual string CVersion(string what) => 
            Language == CFamily.C ?
            cVersionMap.TryGetValue(what.Replace("c", "").Replace("C", ""), out var r) ? r : throw new TaskFatalError($"Invalid argument \"{what}\" for CVersion!") :
            "";
        public static readonly Dictionary<string, string> cVersionMap = new Dictionary<string, string> { { "11", "/std:c11" }, { "17", "/std:c17" }, { "latest", "/std:clatest" } };

        [TargetProperty]
        public virtual string SIMD(SIMDArchitecture simd) => $"/arch:{simd}".Replace("_", ".");

        [TargetProperty] 
        public virtual string WarningLevel(WarningLevel level) => level switch
        {
            SB.Core.WarningLevel.None => "/w",
            SB.Core.WarningLevel.All => "/W3",
            SB.Core.WarningLevel.AllExtra => "/W4",
            SB.Core.WarningLevel.Everything => "/Wall",
            _ => throw new TaskFatalError($"Invalid argument \"{level}\" for MSVC WarningLevel!")
        };

        [TargetProperty] 
        public virtual string WarningAsError(bool v) => v ? "/WX" : "";

        [TargetProperty] 
        public virtual string OptimizationLevel(SB.Core.OptimizationLevel opt) => opt switch
        {
            SB.Core.OptimizationLevel.None => "-Od",
            SB.Core.OptimizationLevel.Fast => "-Ox",
            SB.Core.OptimizationLevel.Faster => "-Ox",
            SB.Core.OptimizationLevel.Fastest => "-O2",
            SB.Core.OptimizationLevel.Smallest => "-O1 -GL",
            _ => throw new TaskFatalError($"Invalid argument \"{opt}\" for MSVC OptimizationLevel!")
        };

        [TargetProperty] 
        public virtual string FpModel(FpModel v) => $"/fp:{v}".ToLowerInvariant();

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] CppFlags(ArgumentList<string> flags) => (Language == CFamily.Cpp) ? flags.Select(flag => flag).ToArray() : new string[0];

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] CFlags(ArgumentList<string> flags) => (Language == CFamily.C) ? flags.Select(flag => flag).ToArray() : new string[0];

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] CXFlags(ArgumentList<string> flags) => (Language == CFamily.C || Language == CFamily.Cpp) ? flags.Select(flag => flag).ToArray() : new string[0];
        
        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] Defines(ArgumentList<string> defines) => defines.Select(define => $"-D{define}").ToArray();

        [TargetProperty(InheritBehavior = true, PathBehavior = true)] 
        public virtual string[]? IncludeDirs(ArgumentList<string> dirs) => dirs.All(x => BS.CheckPath(x, true) ? true : throw new TaskFatalError($"Invalid include dir {x}!")) ? dirs.Select(dir => $"/I{dir}").ToArray() : null;
        
        [TargetProperty] 
        public virtual string RTTI(bool v) => v ? "/GR" : "/GR-";
        
        [TargetProperty] 
        public virtual string[] Source(string path) => BS.CheckFile(path, true) ? new string[] { $"\"{path}\"" } : throw new TaskFatalError($"Source value {path} is not an existed absolute path!");

        public virtual string Arch(Architecture arch) => archMap.TryGetValue(arch, out var r) ? r : throw new TaskFatalError($"Invalid architecture \"{arch}\" for MSVC CL.exe!");
        static readonly Dictionary<Architecture, string> archMap = new Dictionary<Architecture, string> { { Architecture.X86, "" }, { Architecture.X64, "" }, { Architecture.ARM64, "" } };

        public virtual string Object(string path) => BS.CheckFile(path, false) ? $"/Fo\"{path}\"" : throw new TaskFatalError($"Object value {path} is not a valid absolute path!");

        [TargetProperty]
        public virtual string DebugSymbols(bool Enable) => Enable ? PDBMode(Core.PDBMode.Embed) : ""; // for compile speed, use /Z7 always

        public virtual string PDB(string path) => BS.CheckFile(path, false) ? $"/Fd\"{path}\"" : throw new TaskFatalError($"PDB value {path} is not a valid absolute path!");

        public virtual string SourceDependencies(string path) => BS.CheckFile(path, false) ? $"/sourceDependencies \"{path}\"" : throw new TaskFatalError($"SourceDependencies value {path} is not a valid absolute path!");

        public virtual string UsePCHAST(string path) => "";

        private string PDBMode(PDBMode mode) => (mode == SB.Core.PDBMode.Standalone) ? "/Zi" : (mode == SB.Core.PDBMode.Embed) ? "/Z7" : "";

        [TargetProperty]
        public virtual string DynamicDebug(bool v) => v ? "/dynamicdeopt /Z7" : "";

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] MSVC_CppFlags(ArgumentList<string> flags) => CppFlags(flags);

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] MSVC_CFlags(ArgumentList<string> flags) => CFlags(flags);

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] MSVC_CXFlags(ArgumentList<string> flags) => CXFlags(flags);

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] Cl_CppFlags(ArgumentList<string> flags) => CppFlags(flags);

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] Cl_CFlags(ArgumentList<string> flags) => CFlags(flags);

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] Cl_CXFlags(ArgumentList<string> flags) => CXFlags(flags);

        protected CFamily Language { get; }
        protected bool isPCH { get; }
        public ArgumentDictionary Arguments { get; } = new();
        public HashSet<string> RawArguments { get; } = new HashSet<string> { "/c", "/cgthreads1", "/nologo", "/FC" };
        // /c: dont link while compiling, https://learn.microsoft.com/zh-cn/cpp/build/reference/c-compile-without-linking?view=msvc-170
        // /logo: dont show info to output stream, https://learn.microsoft.com/zh-cn/cpp/build/reference/nologo-suppress-startup-banner-c-cpp?view=msvc-170
        // /FC use full path within compiler diagnostics
        // /GR- no runtime type infos
        // /cgthreads4: multi-thread count for CL to use to codegen, https://learn.microsoft.com/zh-cn/cpp/build/reference/cgthreads-code-generation-threads?view=msvc-170
        // /O: https://learn.microsoft.com/en-us/cpp/build/reference/ox-full-optimization?view=msvc-170
        // DumpDependencies: https://learn.microsoft.com/en-us/cpp/build/reference/sourcedependencies?view=msvc-170
    }
}