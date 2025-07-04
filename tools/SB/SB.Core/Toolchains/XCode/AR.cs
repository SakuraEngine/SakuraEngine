using Serilog;
using System.Text.RegularExpressions;

namespace SB.Core
{
    using BS = BuildSystem;
    public class AR : IArchiver
    {
        public AR(string ExePath)
        {
            this.ExecutablePath = ExePath;
        }

        public ArchiveResult Archive(TaskEmitter Emitter, Target Target, IArgumentDriver Driver)
        {
            var LinkerArgsDict = Driver.CalculateArguments();
            var DependArgsList = LinkerArgsDict.Values.SelectMany(x => x).ToList();
            DependArgsList.Add($"LINKER:ID=AR");
            DependArgsList.Add($"LINKER:VERSION={Version}");

            var InputFiles = Driver.Arguments["Inputs"] as ArgumentList<string>;
            var OutputFile = Driver.Arguments["Output"] as string;
            bool Changed = BS.CppCompileDepends(Target).OnChanged(Target.Name, OutputFile!, Emitter.Name, (Depend depend) =>
            {
                // AR output must be the first argument
                var OutputArg = LinkerArgsDict["Output"];
                LinkerArgsDict.Remove("Output");
                var LinkerArgsList = LinkerArgsDict.Values.SelectMany(x => x).ToList();
                var ArgsString = OutputArg[0] + " " + String.Join(" ", LinkerArgsList);

                int ExitCode = BuildSystem.RunProcess(ExecutablePath, ArgsString, out var OutputInfo, out var ErrorInfo);
                if (ExitCode != 0)
                    throw new TaskFatalError($"Archive {OutputFile} failed with fatal error!", $"ar: {ErrorInfo}");
                else if (OutputInfo.Contains("warning"))
                    Log.Warning("Archive: {OutputInfo}", OutputInfo);

                depend.ExternalFiles.AddRange(OutputFile!);
            }, new List<string>(InputFiles!), DependArgsList);

            return new ArchiveResult
            {
                TargetFile = OutputFile!,
                IsRestored = !Changed
            };
        }

        public IArgumentDriver CreateArgumentDriver() => new ARArgumentDriver();
        public Version Version => new Version(0, 0, 0);
        public string ExecutablePath { get; }
    }
}