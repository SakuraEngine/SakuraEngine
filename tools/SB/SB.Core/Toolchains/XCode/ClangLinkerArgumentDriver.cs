using System.IO;

namespace SB.Core
{
    using ArgumentName = string;
    using VS = VisualStudio;
    using BS = BuildSystem;
    public class ClangLinkerArgumentDriver : IArgumentDriver
    {
        public ClangLinkerArgumentDriver(string SDKDirectory)
        {
            RawArguments.Add($"-isysroot {SDKDirectory}");
        }

        [TargetProperty] 
        public string TargetType(TargetType type) => typeMap.TryGetValue(type, out var t) ? t : throw new ArgumentException($"Invalid target type \"{type}\" for clang++ Linker!");
        static readonly Dictionary<TargetType, string> typeMap = new Dictionary<TargetType, string> { { Core.TargetType.Dynamic, "-fPIC -shared" }, { Core.TargetType.Executable, "" }};

        [TargetProperty(InheritBehavior = true, PathBehavior = true)]
        public string[]? LinkDirs(ArgumentList<string> dirs) => dirs.All(x => BS.CheckPath(x, false) ? true : throw new ArgumentException($"Invalid link dir {x}!")) ? dirs.Select(dir => $"-L{dir}").ToArray() : null;
        
        [TargetProperty(InheritBehavior = true)] 
        public string[]? Link(ArgumentList<string> dirs) => dirs.Select(dir => $"-l{dir}").ToArray();

        public string[] Inputs(ArgumentList<string> inputs) => inputs.Select(dir => $"\"{dir}\"").ToArray();

        public string Output(string output) => BS.CheckFile(output, false) ? $"-o \"{output}\"" : throw new ArgumentException($"Invalid output file path {output}!");
       
        [TargetProperty(InheritBehavior = true)]
        public string[]? AppleFramework(ArgumentList<string> frameworks) => frameworks.Select(framework => $"-framework {framework}").ToArray();

        [TargetProperty]
        public string BundleLoader(string loader) => $"-bundle -bundle_loader {loader}";

        public ArgumentDictionary Arguments { get; } = new ArgumentDictionary();
        public HashSet<string> RawArguments { get; } = new HashSet<string> { "-Wl,-rpath,." };
    }
}