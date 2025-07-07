using Serilog;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace SB.Core
{
    using BS = BuildSystem;

    public class ClangCLCompiler : ICompiler
    {
        public ClangCLCompiler(string ExePath, Dictionary<string, string?> Env)
        {
            VCEnvVariables = Env;
            this.ExecutablePath = ExePath;

            if (!File.Exists(ExePath))
                throw new ArgumentException($"ClangCLCompiler: ExePath: {ExePath} is not an existed absolute path!");

           ProcessOptions Options = new ProcessOptions
            {
                Environment = VCEnvVariables,
                WorkingDirectory = null,
                EnableTimeout = true,
                TimeoutMilliseconds = 20 * 60 * 1000 // 20 minutes
            };
            int ExitCode = BuildSystem.RunProcess(ExePath, "--version", out var Output, out var Error, Options);
            if (ExitCode == 0)
            {
                Log.Verbose("clang-cl.exe --version output: {Output}", Output);
                Regex pattern = new Regex(@"\d+(\.\d+)+");
                ClangCLVersion = Version.Parse(pattern.Match(Output).Value);
                Log.Information("clang-cl.exe version ... {ClangCLVersion}", ClangCLVersion);
            }
            else
            {
                throw new Exception($"Failed to get clang-cl.exe version! Exit code: {ExitCode}, Error: {Error}");
            }
        }

        public IArgumentDriver CreateArgumentDriver(CFamily Language, bool isPCH)
        {
            return new ClangCLArgumentDriver(Language, isPCH);
        }

        public CompileResult Compile(TaskEmitter Emitter, Target Target, IArgumentDriver Driver, string? WorkDirectory = null)
        {
            var CompilerArgsDict = Driver.CalculateArguments();
            var CompilerArgsList = CompilerArgsDict.Values.SelectMany(x => x).ToList();
            var DependArgsList = CompilerArgsList.ToList();
            DependArgsList.Add($"COMPILER:ID=ClangCL.exe");
            DependArgsList.Add($"COMPILER:VERSION={Version}");
            DependArgsList.Add($"ENV:VCToolsVersion={VCEnvVariables["VCToolsVersion"]}");
            DependArgsList.Add($"ENV:WindowsSDKVersion={VCEnvVariables["WindowsSDKVersion"]}");
            DependArgsList.Add($"ENV:WindowsSDKLibVersion={VCEnvVariables["WindowsSDKLibVersion"]}");
            DependArgsList.Add($"ENV:UCRTVersion={VCEnvVariables["UCRTVersion"]}");

            var FileToCompile = TryGet(Driver.Arguments, "Source") ?? TryGet(Driver.Arguments, "PCHHeader");
            var ObjectFile = TryGet(Driver.Arguments, "Object") ?? TryGet(Driver.Arguments, "PCHObject");
            var Changed = BS.CppCompileDepends(Target).OnChanged(Target.Name, FileToCompile!, Emitter.Name, (Depend depend) =>
            {
                ProcessOptions Options = new ProcessOptions
                {
                    Environment = VCEnvVariables,
                    WorkingDirectory = WorkDirectory
                };
                int ExitCode = BuildSystem.RunProcess(ExecutablePath, String.Join(" ", CompilerArgsList), out var OutputInfo, out var ErrorInfo, Options);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile {FileToCompile} failed with fatal error!", $"clang-cl.exe: {ErrorInfo}");
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

                if (OutputInfo != "")
                    Log.Warning("clang-cl.exe: {OutputInfo}", OutputInfo);
                if (ErrorInfo != "")
                    Log.Warning("clang-cl.exe: {ErrorInfo}", ErrorInfo);

                depend.ExternalFiles.Add(ObjectFile!);
            }, new List<string> { FileToCompile! }, DependArgsList);

            return new CompileResult
            {
                ObjectFile = ObjectFile!,
                PDBFile = Driver.Arguments.TryGetValue("PDB", out var args) ? (string)args! : "",
                IsRestored = !Changed
            };
        }

        public Version Version => ClangCLVersion;

        private string? TryGet(Dictionary<string, object?> Dict, string Name)
        {
            return Dict.TryGetValue(Name, out var Value) ? Value as string : null;
        }

        public readonly Dictionary<string, string?> VCEnvVariables;
        private readonly Version ClangCLVersion;
        public string ExecutablePath { get; }
    }
}