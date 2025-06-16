using Serilog;
using System.Text.RegularExpressions;

namespace SB.Core
{
    public class AppleClangCompiler : ICompiler, ILinker
    {
        public AppleClangCompiler(string ExePath, XCode Toolchain)
        {
            this.ExecutablePath = ExePath;
            this.XCodeToolchain = Toolchain!;

            if (!File.Exists(ExePath))
                throw new ArgumentException($"ClangCLCompiler: ExePath: {ExePath} is not an existed absolute path!");

            this.ClangVersionTask = Task.Run(() =>
            {
                int ExitCode = BuildSystem.RunProcess(ExePath, "--version", out var Output, out var Error);
                if (ExitCode == 0)
                {
                    Regex pattern = new Regex(@"\d+(\.\d+)+");
                    var ClangVersion = Version.Parse(pattern.Match(Output).Value);
                    Log.Information("clang version ... {ClangVersion}", ClangVersion);
                    return ClangVersion;
                }
                else
                {
                    throw new Exception($"Failed to get clang version! Exit code: {ExitCode}, Error: {Error}");
                }
            });
        }

        public CompileResult Compile(TaskEmitter Emitter, Target Target, IArgumentDriver Driver, string? WorkDirectory = null)
        {
            var CompilerArgsDict = Driver.CalculateArguments();
            var CompilerArgsList = CompilerArgsDict.Values.SelectMany(x => x).ToList();
            var DependArgsList = CompilerArgsList.ToList();
            DependArgsList.Add($"COMPILER:ID=AppleClang");
            DependArgsList.Add($"COMPILER:VERSION={Version}");
            DependArgsList.Add($"ENV:SDKVersion={XCodeToolchain.SDKVersion}");

            var SourceFile = Driver.Arguments["Source"] as string;
            var ObjectFile = Driver.Arguments["Object"] as string;
            var Changed = Depend.OnChanged(Target.Name, SourceFile!, Emitter.Name, (Depend depend) =>
            {
                int ExitCode = BuildSystem.RunProcess(ExecutablePath, String.Join(" ", CompilerArgsList), out var OutputInfo, out var ErrorInfo, null, WorkDirectory);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile {SourceFile} failed with fatal error!", $"apple-clang: {ErrorInfo}");
                }
                else
                {
                    var DepFilePath = Driver.Arguments["SourceDependencies"] as string;
                    // line0: {target}.o: {target}.cpp \
                    // line1~n: {include_file} \
                    var AllLines = File.ReadAllLines(DepFilePath!).Select(
                        x => x.Replace("\\ ", " ").Replace(" \\", "").Trim()
                    ).ToArray();
                    var DepIncludes = new Span<string>(AllLines, 1, AllLines.Length - 1);
                    depend.ExternalFiles.AddRange(DepIncludes);
                }

                if (OutputInfo.Contains("warning"))
                    Log.Warning("apple-clang: {OutputInfo}", OutputInfo);

                depend.ExternalFiles.Add(ObjectFile!);
            }, new List<string> { SourceFile! }, DependArgsList);

            return new CompileResult
            {
                ObjectFile = ObjectFile!,
                PDBFile = Driver.Arguments.TryGetValue("PDB", out var args) ? (string)args! : "",
                IsRestored = !Changed
            };
        }

        IArgumentDriver ICompiler.CreateArgumentDriver(CFamily Language, bool isPCH) => new AppleClangArgumentDriver(XCodeToolchain.PlatSDKDirectory!, Language, isPCH);

        public LinkResult Link(TaskEmitter Emitter, Target Target, IArgumentDriver Driver)
        {
            var LinkerArgsDict = Driver.CalculateArguments();
            var LinkerArgsList = LinkerArgsDict.Values.SelectMany(x => x).ToList();
            var DependArgsList = LinkerArgsList.ToList();
            DependArgsList.Add($"LINKER:ID=Clang++");
            DependArgsList.Add($"LINKER:VERSION={Version}");
            DependArgsList.Add($"ENV:SDKVersion={XCodeToolchain.SDKVersion}");

            var InputFiles = Driver.Arguments["Inputs"] as ArgumentList<string>;
            var OutputFile = Driver.Arguments["Output"] as string;
            bool Changed = Depend.OnChanged(Target.Name, OutputFile!, Emitter.Name, (Depend depend) =>
            {
                int ExitCode = BuildSystem.RunProcess(ExecutablePath, String.Join(" ", LinkerArgsList), out var OutputInfo, out var ErrorInfo);
                if (ExitCode != 0)
                    throw new TaskFatalError($"Link {OutputFile} failed with fatal error!", $"clang++: {ErrorInfo}");
                else if (OutputInfo.Contains("warning"))
                    Log.Warning("clang++ (as linker): {OutputInfo}", OutputInfo);

                depend.ExternalFiles.AddRange(OutputFile!);
            }, new List<string>(InputFiles!), DependArgsList);

            return new LinkResult
            {
                TargetFile = LinkerArgsDict["Output"][0],
                PDBFile = Driver.Arguments.TryGetValue("PDB", out var args) ? (string)args! : "",
                IsRestored = !Changed
            };
        }

        IArgumentDriver ILinker.CreateArgumentDriver() => new ClangLinkerArgumentDriver(XCodeToolchain.PlatSDKDirectory!);

        public Version Version
        {
            get
            {
                if (!ClangVersionTask.IsCompleted)
                    ClangVersionTask.Wait();
                return ClangVersionTask.Result;
            }
        }

        private readonly Task<Version> ClangVersionTask;
        public string ExecutablePath { get; }
        private XCode XCodeToolchain { get; init; }
    }
}