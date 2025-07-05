using System.Collections.Concurrent;
using SB.Core;
using Serilog;

namespace SB
{
    public class CppSLEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CppSLFileList>();
        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppSLFileList>();

        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            var CppSLFileList = FileList as CppSLFileList;
            string SourceName = Path.GetFileNameWithoutExtension(SourceFile);
            var OutputDirectory = Path.Combine(Engine.BuildPath, ShaderOutputDirectories[Target.Name]);
            Directory.CreateDirectory(OutputDirectory);

            string Executable = CppSLCompiler;
            if (BuildSystem.TargetOS == OSPlatform.Windows)
                Executable += ".exe";

            bool Changed = Engine.ConfigureNotAwareDepend.OnChanged(Target.Name, SourceFile, "CPPSL", (Depend depend) =>
            {
                var Arguments = new string[]
                {
                    $"--extra-arg=-I{Engine.EngineDirectory}/tools/shader_compiler/LLVM/test_shaders",
                    SourceFile
                };

                int ExitCode = BuildSystem.RunProcess(Executable, string.Join(" ", Arguments), out var Output, out var Error,
                    WorkingDirectory: OutputDirectory);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Compile CppSL for {SourceFile} failed with fatal error!", $"CppSLCompiler.exe: {Error}");
                }
                else
                {
                    var DepFilePath = Path.Combine(OutputDirectory, $"{SourceName}.d");
                    // line0: {target}.o: {target}.cpp \
                    // line1~n: {include_file} \
                    var AllLines = File.ReadAllLines(DepFilePath!).Select(
                        x => x.Replace("\\ ", " ").Replace(" \\", "").Trim()
                    ).ToArray();
                    var DepIncludes = new Span<string>(AllLines, 1, AllLines.Length - 1);
                    depend.ExternalFiles.AddRange(DepIncludes);
                }

                // Get all files under output directory that matches '{SourceName}.*.*.hlsl'
                var OutputFiles = Directory.GetFiles(OutputDirectory, $"{SourceName}.*.*.hlsl");
                depend.ExternalFiles.AddRange(OutputFiles);
            }, new string[] { Executable, SourceFile }, null);

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
                    "-fms-extensions", 
                    "-Wno-microsoft-union-member-reference",
                    $"-I{Engine.EngineDirectory}/tools/shader_compiler/LLVM/test_shaders",
                    SourceFile
                }
            };
            CompileCommands.Add(SB.Core.Json.Serialize(CMD));

            if (BuildSystem.TargetOS == OSPlatform.Windows)
            {
                var OutputFiles = Directory.GetFiles(OutputDirectory, $"{SourceName}.*.*.hlsl");
                foreach (var HLSL in OutputFiles)
                {
                    Changed |= !DXCEmitter.CompileHLSL(Target, HLSL, "", OutputDirectory)!.IsRestored;
                }
            }

            return new PlainArtifact { IsRestored = !Changed };
        }

        public static void WriteCompileCommandsToFile(string Path)
        {
            File.WriteAllText(Path, "[" + String.Join(",", CompileCommands) + "]");
        }

        public static ConcurrentBag<string> CompileCommands = new();
        public static string CppSLCompiler = Path.Combine(Engine.TempPath, "tools", "CppSLCompiler");
        public static Dictionary<string, string> ShaderOutputDirectories = new();
    }

    public class CppSLFileList : FileList
    {

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

        public static Target CppSLOutputDirectory(this Target @this, string Directory)
        {
            CppSLEmitter.ShaderOutputDirectories[@this.Name] = Directory;
            return @this;
        }
    }

}