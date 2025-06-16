using SB.Core;
using Microsoft.Extensions.FileSystemGlobbing;

namespace SB
{
    using BS = BuildSystem;

    public class PCHEmitter : TaskEmitter
    {
        public PCHEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;
        public override bool EnableEmitter(Target Target) => Target.HasAttribute<CreateSharedPCHAttribute>() || Target.HasAttribute<CreatePrivatePCHAttribute>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            var CreateSharedPCH = Target.GetAttribute<CreateSharedPCHAttribute>();
            var CreatePrivatePCH = Target.GetAttribute<CreatePrivatePCHAttribute>();
            bool Changed = false;

            var CreatePCH = (CreatePCHAttribute CreatePCH, PCHMode Mode) => {
                var PCHFile = GetPCHFile(Target, Mode);
                var PCHASTFile = GetPCHASTFile(Target, Mode);

                Profiler.Message(Mode.ToString());

                if (CreatePCH.Globs.Count != 0)
                {
                    var GlobMatcher = new Matcher();
                    GlobMatcher.AddIncludePatterns(CreatePCH.Globs);
                    CreatePCH.Headers.AddRange(GlobMatcher.GetResultsInFullPath(Target.Directory));
                }

                Changed |= Depend.OnChanged(Target.Name, PCHFile, "PCHEmitter.CreatePCH", (Depend depend) =>
                {
                    var PCHIncludes = String.Join("\n", CreatePCH.Headers.Select(H => $"#include \"{H}\""));
                    var PCHFileContent = $"""
                    #pragma once
                    #ifdef __cplusplus
                    #ifdef _WIN32
                    #ifndef _CRT_SECURE_NO_WARNINGS
                    #define _CRT_SECURE_NO_WARNINGS 1
                    #endif
                    #include <intrin.h>
                    #endif
                    {PCHIncludes}
                    #endif // __cplusplus
                    """;
                    File.WriteAllText(PCHFile, PCHFileContent);
                    depend.ExternalFiles.Add(PCHFile);
                }, CreatePCH.Headers, null);

                var SourceDependencies = Path.Combine(Target.GetStorePath(BuildSystem.DepsStore), BuildSystem.GetUniqueTempFileName(PCHFile, Target.Name + this.Name, "source.deps.json"));
                var PCHArguments = Target.Arguments;
                if (Mode == PCHMode.Shared)
                {
                    var SharedPCHArgs = (PCHArguments as ArgumentDictionary)!.Copy();
                    // Remove all private defines & add all interface args
                    if (Target.Arguments.TryGetValue("Defines", out var defs))
                    {
                        ArgumentList<string> Defines = (defs as ArgumentList<string>)?.Copy() as ArgumentList<string> ?? new();
                        if (Target.PrivateArguments.TryGetValue("Defines", out var PrivateDefines))
                        {
                            Defines.Substract(PrivateDefines as ArgumentList<string>);
                        }
                        SharedPCHArgs["Defines"] = Defines;
                    }
                    SharedPCHArgs.Merge(Target.InterfaceArguments);
                    PCHArguments = SharedPCHArgs;
                }
                // Execute
                // TODO: SEPERATE C AND CPP PCH
                var CompilerDriver = Toolchain.Compiler.CreateArgumentDriver(CFamily.Cpp, true)
                    .AddArguments(PCHArguments)
                    .AddArgument("Source", PCHFile)
                    .AddArgument("Object", PCHASTFile)
                    .AddArgument("SourceDependencies", SourceDependencies);
                Toolchain.Compiler.Compile(this, Target, CompilerDriver, Path.GetDirectoryName(PCHASTFile));
            };

            if (CreateSharedPCH is not null)
                CreatePCH(CreateSharedPCH, PCHMode.Shared);
            if (CreatePrivatePCH is not null)
                CreatePCH(CreatePrivatePCH, PCHMode.Private);

            return new PlainArtifact { IsRestored = !Changed };
        }
        // 2025/3/24
        // under clang-cl, these two files must locate in the same directory
        // because the compiler has bugs for windows path
        internal static string GetPCHFile(Target Target, PCHMode Mode) => Path.Combine(Target.GetStorePath(BS.GeneratedSourceStore), $"{Mode}PCH.h");
        internal static string GetPCHASTFile(Target Target, PCHMode Mode) => Path.Combine(Target.GetStorePath(BS.GeneratedSourceStore), $"{Mode}PCH.pch");
        internal static bool ProvideSharedPCH(Target Target) => Target.GetAttribute<CreateSharedPCHAttribute>() is not null;
        private IToolchain Toolchain;
    }

    public enum PCHMode
    {
        Shared,
        Private
    }

    public class UsePCHAttribute
    {
        public PCHMode Mode { get; internal set; }
        public string? WantedSharedPCH { get; internal set; }
        public string? CppPCHAST { get; internal set; }
    }

    public abstract class CreatePCHAttribute
    {
        public List<string> Headers { get; init; } = new();
        public List<string> Globs { get; init; } = new();
    }

    public class CreateSharedPCHAttribute : CreatePCHAttribute {}
    
    public class CreatePrivatePCHAttribute : CreatePCHAttribute {}

    public static partial class TargetExtensions
    {
        public static Target UseSharedPCH(this Target @this, string? Wanted = null)
        {
            var UseAttribute = @this.GetAttribute<UsePCHAttribute>();
            if (UseAttribute is null)
            {
                @this.SetAttribute(@this.SetupUsePCHAttribute(PCHMode.Shared));
            }
            else
            {
                UseAttribute.Mode = PCHMode.Shared;
                UseAttribute.WantedSharedPCH = Wanted;
            }
            return @this;
        }

        public static Target UsePrivatePCH(this Target @this, params string[] Headers)
        {
            var UseAttribute = @this.GetAttribute<UsePCHAttribute>();
            if (UseAttribute is null)
            {
                @this.SetAttribute(@this.SetupUsePCHAttribute(PCHMode.Private));
            }
            else
            {
                UseAttribute.Mode = PCHMode.Private;
            }
                
            var CreateAttribute = new CreatePrivatePCHAttribute();
            @this.SetAttribute(CreateAttribute);
            foreach (var file in Headers)
            {
                if (file.Contains("*"))
                {
                    if (Path.IsPathFullyQualified(file))
                        CreateAttribute.Globs.Add(Path.GetRelativePath(@this.Directory, file));
                    else
                        CreateAttribute.Globs.Add(file);
                }
                else if (Path.IsPathFullyQualified(file))
                    CreateAttribute.Headers.Add(file);
                else
                    CreateAttribute.Headers.Add(Path.Combine(@this.Directory, file));
            }
            return @this;
        }

        public static Target CreateSharedPCH(this Target @this, params string[] Headers)
        {
            var CreateAttribute = new CreateSharedPCHAttribute();
            @this.SetAttribute(CreateAttribute);
            foreach (var file in Headers)
            {
                if (file.Contains("*"))
                {
                    if (Path.IsPathFullyQualified(file))
                        CreateAttribute.Globs.Add(Path.GetRelativePath(@this.Directory, file));
                    else
                        CreateAttribute.Globs.Add(file);
                }
                else if (Path.IsPathFullyQualified(file))
                    CreateAttribute.Headers.Add(file);
                else
                    CreateAttribute.Headers.Add(Path.Combine(@this.Directory, file));
            }
            return @this;
        }

        private static UsePCHAttribute SetupUsePCHAttribute(this Target @this, PCHMode Mode)
        {
            var UseAttribute = new UsePCHAttribute { Mode = Mode };
            @this.AfterLoad((@this) => {
                string? PCHProvider = null;
                if (UseAttribute.Mode == PCHMode.Shared && @this.Dependencies.Count > 0)
                {
                    if (UseAttribute.WantedSharedPCH is null)
                    {
                        var Providers = @this.Dependencies.Select(D => BS.GetTarget(D)!).Where(D => PCHEmitter.ProvideSharedPCH(D));
                        if (Providers.Any())
                        {
                            var Scores = Providers.Select(P => 1 + P.Dependencies.Count(PD => PCHEmitter.ProvideSharedPCH(BS.GetTarget(PD)!)));
                            var BestProvider = Providers.ElementAt(Scores.ToList().IndexOf(Scores.Max()));
                            PCHProvider = BestProvider.Name;
                        }
                    }
                    else
                        PCHProvider = UseAttribute.WantedSharedPCH!;
                }
                else if (UseAttribute.Mode == PCHMode.Private)
                {
                    PCHProvider = @this.Name;
                }
                
                if (PCHProvider is not null)
                {
                    // later we get it in cpp compile emitter
                    UseAttribute.CppPCHAST = PCHEmitter.GetPCHASTFile(BS.GetTarget(PCHProvider)!, UseAttribute.Mode);
                }
            });
            return UseAttribute;
        }
    }
}