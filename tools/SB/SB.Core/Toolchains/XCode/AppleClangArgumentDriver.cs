namespace SB.Core
{
    using ArgumentName = string;
    using BS = BuildSystem;
    public class AppleClangArgumentDriver : IArgumentDriver
    {
        public AppleClangArgumentDriver(string SDKDirectory, CFamily Language, bool isPCH)
        {
            this.Language = Language;
            this.isPCH = isPCH;
            RawArguments.Add($"-isysroot {SDKDirectory}");
            /*
            RawArguments.Add($"-isystem {SDKDirectory}/System/Library/Frameworks");
            RawArguments.Add($"-isystem {SDKDirectory}/usr/include/c++/v1");
            RawArguments.Add($"-isystem {SDKDirectory}/usr/include");
            RawArguments.Add("-isystem /Library/Developer/CommandLineTools/usr/lib/clang/16/include");
            RawArguments.Add("-isystem /Library/Developer/CommandLineTools/usr/include");
            */
        }

        [TargetProperty]
        public string Exception(bool Enable) => Enable ? "-fexceptions" : "-fno-exceptions";

        [TargetProperty]
        public string CppVersion(string what) => 
            Language == CFamily.Cpp || Language == CFamily.ObjCpp ?
            cppVersionMap.TryGetValue(what.Replace("c++", "").Replace("C++", ""), out var r) ? r : throw new TaskFatalError($"Invalid argument \"{what}\" for CppVersion!") :
            "";
        public static readonly Dictionary<string, string> cppVersionMap = new Dictionary<string, string> { { "11", "-std=c++11" }, { "14", "-std=c++14" }, { "17", "-std=c++17" }, { "20", "-std=c++20" }, { "23", "-std=c++23" }, { "latest", "-std=c++26" } };

        [TargetProperty]
        public string CVersion(string what) => 
            Language == CFamily.C || Language == CFamily.ObjC ?
            cVersionMap.TryGetValue(what.Replace("c", "").Replace("C", ""), out var r) ? r : throw new TaskFatalError($"Invalid argument \"{what}\" for CppVersion!") :
            "";
        public static readonly Dictionary<string, string> cVersionMap = new Dictionary<string, string> { { "11", "-std=c11" }, { "17", "-std=c17" }, { "23", "-std=c23" }, { "latest", "-std=c2y" } };

        [TargetProperty]
        public string SIMD(SIMDArchitecture simd) => simd switch
        {
            SIMDArchitecture.SSE2 => "-msse2",
            SIMDArchitecture.SSE4_2 => "-msse4.2",
            SIMDArchitecture.AVX => "-mavx",
            SIMDArchitecture.AVX512 => "-mavx512f -mavx512dq -mavx512bw -mavx512vl",
            SIMDArchitecture.AVX10_1 => "-mavx10.1",
            SIMDArchitecture.Neon => "-mfpu=neon",
            _ => throw new TaskFatalError($"Invalid argument \"{simd}\" for SIMD!")
        };

        [TargetProperty]
        public string WarningAsError(bool Enable) => Enable ? "-Werror" : "-Wno-error";

        [TargetProperty]
        public string OptimizationLevel(OptimizationLevel level) => level switch
        {
            SB.Core.OptimizationLevel.None => "-O0",
            SB.Core.OptimizationLevel.Fast => "-O1",
            SB.Core.OptimizationLevel.Faster => "-O2",
            SB.Core.OptimizationLevel.Fastest => "-O3",
            SB.Core.OptimizationLevel.Smallest => "-Oz",
            _ => throw new TaskFatalError($"Invalid argument \"{level}\" for OptimizationLevel!")
        };

        [TargetProperty] 
        public string FpModel(FpModel v) => $"-ffp-model={v}".ToLowerInvariant();

        [TargetProperty(InheritBehavior = true)] 
        public string[] CppFlags(ArgumentList<string> flags) => flags.Select(flag => flag).ToArray();
        
        [TargetProperty(InheritBehavior = true)] 
        public string[] Defines(ArgumentList<string> defines) => defines.Select(define => $"-D{define}").ToArray();

        [TargetProperty(InheritBehavior = true, PathBehavior = true)] 
        public string[]? IncludeDirs(ArgumentList<string> dirs) => dirs.All(x => BS.CheckPath(x, true) ? true : throw new TaskFatalError($"Invalid include dir {x}!")) ? dirs.Select(dir => $"-I{dir}").ToArray() : null;
        
        [TargetProperty] 
        public string RTTI(bool v) => v ? "-frtti" : "-fno-rtti";

        [TargetProperty]
        public virtual string DebugSymbols(bool Enable) => Enable ? "-g" : "";
        
        [TargetProperty] 
        public string Source(string path) => BS.CheckFile(path, true) ? GetLanguageArgString() + $" \"{path}\"" : throw new TaskFatalError($"Source value {path} is not an existed absolute path!");

        public string Arch(Architecture arch) => archMap.TryGetValue(arch, out var r) ? r : throw new TaskFatalError($"Invalid architecture \"{arch}\" for Apple clang!");
        static readonly Dictionary<Architecture, string> archMap = new Dictionary<Architecture, string> { { Architecture.X86, "" }, { Architecture.X64, "" }, { Architecture.ARM64, "" } };

        public string Object(string path) => BS.CheckFile(path, false) ? $"-o\"{path}\"" : throw new TaskFatalError($"Object value {path} is not a valid absolute path!");

        public string PDBMode(PDBMode mode) => "";

        public string PDB(string path) => "";

        public string SourceDependencies(string path) => BS.CheckFile(path, false) ? $"-MD -MF\"{path}\"" : throw new TaskFatalError($"SourceDependencies value {path} is not a valid absolute path!");

        public string UsePCHAST(string path) => BS.CheckFile(path, false) ? $"-include-pch \"{path}\"" : throw new TaskFatalError($"PCHObject value {path} is not a valid absolute path!");

        protected CFamily Language { get; }
        protected bool isPCH = false;
        protected string GetLanguageArgString() => Language switch
        {
            CFamily.C => isPCH ? "-xc-header" : "",
            CFamily.Cpp => isPCH ? "-xc++-header" : "-xc++",
            CFamily.ObjC => isPCH ? "-xobjective-c-header" : "-xobjective-c",
            CFamily.ObjCpp => isPCH ? "-xobjective-c++-header" : "-xobjective-c++",
            _ => throw new TaskFatalError($"Invalid language \"{Language}\" for Apple clang!")
        };
        public ArgumentDictionary Arguments { get; } = new ArgumentDictionary();
        public HashSet<string> RawArguments { get; } = new HashSet<string> { "-c" };
    }
}