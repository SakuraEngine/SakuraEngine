using System.Collections.Concurrent;
using SB.Core;
using Serilog;

namespace SB
{
    public class CppSLEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppSLFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppSLFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? FileOptions, string SourceFile)
        {
            var OutputDirectory = Path.Combine(BuildDirs.BuildDir, ShaderOutputDirectories[Target.Name]);
            Directory.CreateDirectory(OutputDirectory);
            var FO = FileOptions as CppSLFileOptions;

            bool Changed = false;
            if (FO?.VariantTemplate is null)
            {
                KeyValuePair<string, Dictionary<CppSLVariantOption, string>> Permutation = new("", new());
                Changed |= CompileSinglePermutation(Target, Permutation, FileOptions, SourceFile);
            }
            else
            {
                var Variants = FO.VariantTemplate.GetAllVariants();
                lock (this)
                {
                    EngineDepends.ShaderCompile.OnChanged(Target.Name, SourceFile, "CPPSL.Variant", (Depend depend) =>
                    {
                        var JSONFile = FO.VariantTemplate.EmitVariantJSON(OutputDirectory);
                        depend.ExternalFiles.Add(JSONFile);
                    }, null, new string[] { FO.VariantTemplate.GetIdentityString() });
                }

                try
                {
                    Parallel.ForEach(Variants, (Variant) =>
                    {
                        Changed |= CompileSinglePermutation(Target, Variant, FileOptions, SourceFile);
                    });
                }
                catch (System.AggregateException E)
                {
                    throw E.InnerException!;
                }
            }

            return new PlainArtifact { IsRestored = !Changed };
        }
        internal bool CompileSinglePermutation(Target Target, KeyValuePair<string, Dictionary<CppSLVariantOption, string>> Permutation, FileOptions? FileOptions, string SourceFile)
        {
            string SourceName = Path.GetFileNameWithoutExtension(SourceFile);
            var PermutationSourceName = Permutation.Key == "" ? SourceName : $"{Permutation.Key}#{SourceName}";
            var OutputDirectory = Path.Combine(BuildDirs.BuildDir, ShaderOutputDirectories[Target.Name]);

            string Executable = CppSLCompiler;
            if (BuildSystem.TargetOS == OSPlatform.Windows)
                Executable += ".exe";

            ArgumentDictionary PermutationArgs = new();
            foreach (var Macro in Permutation.Value)
            {
                if (Macro.Key.Type == CppSLMacroType.Level)
                {
                    foreach (var LV in Macro.Key.ValueSelections)
                    {
                        PermutationArgs.CppSL_Defines(LV + "=1");
                        if (LV == Macro.Value)
                            break;
                    }
                }
                else if (Macro.Key.Type == CppSLMacroType.Select)
                {
                    PermutationArgs.CppSL_Defines(Macro.Value + "=1");
                }
                else if (Macro.Key.Type == CppSLMacroType.Value)
                {
                    if (Macro.Value == "on")
                        PermutationArgs.CppSL_Defines(Macro.Key.Key + "=1");
                    else if (Macro.Value != "off")
                        PermutationArgs.CppSL_Defines(Macro.Key.Key + "=" + Macro.Value);
                }
            }

            IArgumentDriver Driver = new CppSLArgumentDriver();
            var CompilerArgsDict = Driver.AddArguments(Target.Arguments)
                .MergeArguments(PermutationArgs, false)
                .MergeArguments(FileOptions?.Arguments, true)
                .CalculateArguments();

            var Arguments = CompilerArgsDict.Values.SelectMany(x => x).Select(x => $"--extra-arg={x}").ToList();
            // Add OutputDirectory To Arguments
            var DependArgs = Arguments.ToList();
            DependArgs.Add($"-I{OutputDirectory}");
            bool Changed = EngineDepends.ShaderCompile.OnChanged(Target.Name, SourceFile, "CPPSL.Compile." + Permutation.Key, (Depend depend) =>
            {
                var _Args = new string[]
                {
                    $"--permutation={Permutation.Key}",
                    $"--extra-arg=-I{BuildDirs.EngineDir}/engine/tools/shader_compiler/ShaderSTL",
                    SourceFile
                };
                Arguments.AddRange(_Args);

                ProcessOptions Options = new ProcessOptions
                {
                    WorkingDirectory = OutputDirectory
                };
                int ExitCode = BuildSystem.RunProcess(Executable, string.Join(" ", Arguments), out var Output, out var Error, Options);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile CppSL for {SourceFile} failed with fatal error!", $"CppSLCompiler.exe: {Error}");
                }
                else
                {
                    var DepFilePath = Path.Combine(OutputDirectory, $"{PermutationSourceName}.d");
                    // line0: {target}.o: {target}.cpp \
                    // line1~n: {include_file} \
                    var AllLines = File.ReadAllLines(DepFilePath!).Select(
                        x => x.Replace("\\ ", " ").Replace(" \\", "").Trim()
                    ).ToArray();
                    var DepIncludes = new Span<string>(AllLines, 1, AllLines.Length - 1);
                    depend.ExternalFiles.Add(DepFilePath);
                    depend.ExternalFiles.AddRange(DepIncludes);
                }

                // Get all files under output directory that matches '{SourceName}.*.*.hlsl' or '{SourceName}.*.*.metal'
                var OutputFiles = Directory.GetFiles(OutputDirectory, $"{PermutationSourceName}.*.*.hlsl")
                    .Concat(Directory.GetFiles(OutputDirectory, $"{PermutationSourceName}.*.*.metal"))
                    .ToArray();
                depend.ExternalFiles.AddRange(OutputFiles);
            }, new string[] { Executable, SourceFile }, DependArgs);

            try
            {
                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    var OutputFiles = Directory.GetFiles(OutputDirectory, $"{PermutationSourceName}.*.*.hlsl");
                    Parallel.ForEach(OutputFiles, (HLSL) =>
                    {
                        Changed |= !DXCEmitter.CompileHLSL(Target, HLSL, "", OutputDirectory)!.IsRestored;
                    });
                }
                else if (BuildSystem.TargetOS == OSPlatform.OSX)
                {
                    var OutputFiles = Directory.GetFiles(OutputDirectory, $"{PermutationSourceName}.*.*.metal");
                    Parallel.ForEach(OutputFiles, (Metal) =>
                    {
                        Changed |= !MSLEmitter.CompileMetal(Target, Metal, "main", OutputDirectory)!.IsRestored;
                    });
                }
            }
            catch (System.AggregateException E)
            {
                throw E.InnerException!;
            }

            return Changed;
        }

        public static string CppSLCompiler = Path.Combine(BuildDirs.TempDir, "tools", "CppSLCompiler");
        public static Dictionary<string, string> ShaderOutputDirectories = new();
    }

    public class CppSLCompileCommandsEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppSLFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppSLFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? FileOptions, string SourceFile)
        {
            var OutputDirectory = Path.Combine(BuildDirs.TempDir, CppSLEmitter.ShaderOutputDirectories[Target.Name]);
            Directory.CreateDirectory(OutputDirectory);

            IArgumentDriver Driver = new CppSLArgumentDriver();
            var CompilerArgsDict = Driver.AddArguments(Target.Arguments)
                .MergeArguments(FileOptions?.Arguments, true)
                .CalculateArguments();
            var CompilerArgsList = CompilerArgsDict.Values.SelectMany(x => x).ToList();
            var CMD = new CompileCommand
            {
                directory = OutputDirectory,
                file = SourceFile,
                arguments = new List<string>
                {
                    "clangd",
                    "-std=c++23",
                    "-x", "c++",
                    "-fsyntax-only",
                    "-D__CPPSL__",
                    "-Wno-unknown-attributes",
                    "-fms-extensions",
                    "-fms-compatibility-version=17.1.1",
                    "-Wno-microsoft-union-member-reference",
                    $"-I{BuildDirs.EngineDir}/engine/tools/shader_compiler/ShaderSTL"
                }
            };
            CMD.arguments.AddRange(CompilerArgsList);
            CMD.arguments.Add(SourceFile);
            CompileCommands.Add(SB.Core.Json.Serialize(CMD));

            return new PlainArtifact { IsRestored = false };
        }
        public static void WriteCompileCommandsToFile(string Path)
        {
            File.WriteAllText(Path, "[" + String.Join(",", CompileCommands) + "]");
        }
        public static ConcurrentBag<string> CompileCommands = new();
    }

    public class CppSLFileList : FileList
    {

    }

    public class CppSLArgumentDriver : IArgumentDriver
    {
        [TargetProperty(InheritBehavior = true)]
        public string[] CppSL_Defines(ArgumentList<string> defines) => defines.Select(define => $"-D{define}").ToArray();

        [TargetProperty(InheritBehavior = true, PathBehavior = true)]
        public virtual string[]? CppSL_IncludeDirs(ArgumentList<string> dirs) => dirs.All(x => BuildSystem.CheckPath(x, true) ? true : throw new TaskFatalError($"Invalid include dir {x}!")) ? dirs.Select(dir => $"-I{dir}").ToArray() : null;

        public ArgumentDictionary Arguments { get; } = new();
        public HashSet<string> RawArguments { get; } = new();
    }

    public class CppSLFileOptions : FileOptions
    {
        public CppSLVariantTemplate? VariantTemplate;
    }

    public static partial class TargetExtensions
    {
        public static Target AddCppSLFiles(this Target @this, params string[] Files)
        {
            @this.FileList<CppSLFileList>().AddFiles(Files);
            if (!CppSLEmitter.ShaderOutputDirectories.ContainsKey(@this.Name))
                CppSLEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target AddCppSLFiles(this Target @this, CppSLFileOptions Options, params string[] Files)
        {
            @this.FileList<CppSLFileList>().AddFiles(Options, Files);
            if (!CppSLEmitter.ShaderOutputDirectories.ContainsKey(@this.Name))
                CppSLEmitter.ShaderOutputDirectories.Add(@this.Name, "resources/shaders");
            return @this;
        }

        public static Target CppSLOutputDirectory(this Target @this, string Directory)
        {
            CppSLEmitter.ShaderOutputDirectories[@this.Name] = Directory;
            return @this;
        }
    }

}