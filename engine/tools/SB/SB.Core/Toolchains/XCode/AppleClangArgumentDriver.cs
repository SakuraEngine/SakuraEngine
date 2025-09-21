namespace SB.Core
{
    using ArgumentName = string;
    using BS = BuildSystem;
    public class AppleClangArgumentDriver : IArgumentDriver
    {
        public AppleClangArgumentDriver(string DeveloperDirectory, string SDKDirectory, Version ClangVersion, CFamily Language, bool isPCH)
        {
            this.Language = Language;
            this.isPCH = isPCH;
            RawArguments.Add("-isysroot");
            RawArguments.Add($"{SDKDirectory}");

            // WE MUST KEEP THIS ORDER OTHER WISE IT WILL BREAK THE BUILD!
            RawArguments.Add($"-isystem {Path.Combine(SDKDirectory, "usr/include/c++/v1")}");
            RawArguments.Add($"-isystem {Path.Combine(DeveloperDirectory, $"usr/lib/clang/{ClangVersion.Major}/include")}");
            RawArguments.Add($"-isystem {Path.Combine(SDKDirectory, "usr/include")}");
            RawArguments.Add($"-isystem {Path.Combine(DeveloperDirectory, "usr/include")}");
            RawArguments.Add($"-isystem {Path.Combine(SDKDirectory, "System/Library/Frameworks")}");
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
            SIMDArchitecture.Neon => "", // -mfpu=neon is no longer needed from MacOS 11.0+ because Apple Silicon has arm64/Neon by default.
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
        public virtual string[] CppFlags(ArgumentList<string> flags) => (Language == CFamily.Cpp) ? CXFlags(flags) : new string[0];

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] CFlags(ArgumentList<string> flags) => (Language == CFamily.C) ? CXFlags(flags) : new string[0];

        [TargetProperty(InheritBehavior = true)] 
        public virtual string[] CXFlags(ArgumentList<string> flags) => flags.Select(flag => flag).ToArray();

        [TargetProperty(InheritBehavior = true)]
        public string[] Defines(ArgumentList<string> defines) => defines.Select(define => $"-D{define}").ToArray();

        [TargetProperty(InheritBehavior = true, PathBehavior = true)]
        public string[]? IncludeDirs(ArgumentList<string> dirs) => dirs.All(x => BS.CheckPath(x, true) ? true : throw new TaskFatalError($"Invalid include dir {x}!")) ? dirs.Select(dir => $"-I{dir}").ToArray() : null;

        [TargetProperty]
        public string RTTI(bool v) => v ? "-frtti" : "-fno-rtti";

        [TargetProperty]
        public virtual string DebugSymbols(bool Enable) => Enable ? "-g" : "";

        [TargetProperty]
        public string[] Source(string path) => BS.CheckFile(path, true) ? GetLanguageArgString($" \"{path}\"") : throw new TaskFatalError($"Source value {path} is not an existed absolute path!");

        public string Arch(Architecture arch) => archMap.TryGetValue(arch, out var r) ? r : throw new TaskFatalError($"Invalid architecture \"{arch}\" for Apple clang!");
        static readonly Dictionary<Architecture, string> archMap = new Dictionary<Architecture, string> { { Architecture.X86, "" }, { Architecture.X64, "" }, { Architecture.ARM64, "" } };

        public string Object(string path) => BS.CheckFile(path, false) ? $"-o\"{path}\"" : throw new TaskFatalError($"Object value {path} is not a valid absolute path!");

        public string PDBMode(PDBMode mode) => "";

        public string PDB(string path) => "";

        public string[] SourceDependencies(string path) => BS.CheckFile(path, false) ? new string[] { "-MD", "-MF", $"\"{path}\"" } : throw new TaskFatalError($"SourceDependencies value {path} is not a valid absolute path!");

        public string UsePCHAST(string path) => BS.CheckFile(path, false) ? $"-include-pch \"{path}\"" : throw new TaskFatalError($"PCHObject value {path} is not a valid absolute path!");

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] AppleClang_CppFlags(ArgumentList<string> flags) => CppFlags(flags);

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] AppleClang_CFlags(ArgumentList<string> flags) => CFlags(flags);

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] AppleClang_CXFlags(ArgumentList<string> flags) => CXFlags(flags);

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] Clang_CppFlags(ArgumentList<string> flags) => CppFlags(flags);

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] Clang_CFlags(ArgumentList<string> flags) => CFlags(flags);

        [TargetProperty(InheritBehavior = true)]
        public virtual string[] Clang_CXFlags(ArgumentList<string> flags) => CXFlags(flags);

        protected CFamily Language { get; }
        protected bool isPCH = false;
        protected string[] GetLanguageArgString(string p)
        {
            string lang;
            switch (Language)
            {
                case CFamily.C:
                    lang = isPCH ? "c-header" : "";
                    break;
                case CFamily.Cpp:
                    lang = isPCH ? "c++-header" : "c++";
                    break;
                case CFamily.ObjC:
                    lang = isPCH ? "objectivec-header" : "objective-c";
                    break;
                case CFamily.ObjCpp:
                    lang = isPCH ? "objectivec++-header" : "objective-c++";
                    break;
                default:
                    throw new TaskFatalError($"Invalid language \"{Language}\" for Apple clang!");
            }
            return string.IsNullOrEmpty(lang) ? new string[] { p } : new string[] { $"-x", lang, p };
        }
        public ArgumentDictionary Arguments { get; } = new ArgumentDictionary();
        public HashSet<string> RawArguments { get; } = new HashSet<string> { "-c" };
    }
}