using SB.Core;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace SB
{
    public class ModuleInfoEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasAttribute<ModuleAttribute>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            Stopwatch sw = new();
            sw.Start();

            var GenSourcePath = Target.GetStorePath(BuildSystem.GeneratedSourceStore);
            var GenFileName = Path.Combine(GenSourcePath, "module.configure.cpp");
            Target.AddCppFiles(GenFileName);
            var ModuleDependencies = Target.Dependencies.Where((Dependency) => BuildSystem.GetTarget(Dependency)!.GetAttribute<ModuleAttribute>() is not null);
            var DepArgs = ModuleDependencies.ToList();
            DepArgs.Add(Target.Name);
            DepArgs.Add(Target.GetTargetType()?.ToString() ?? "none");
            bool Changed = Engine.ConfigureNotAwareDepend.OnChanged(Target.Name, GenFileName, Name, (Depend depend) => {
                string DependenciesArray = "[" + String.Join(",", ModuleDependencies.Select(D => FormatDependency(D))) + "]";
                string JSON = $$"""
                {
                    "name": "{{Target.Name}}",
                    "prettyname": "{{Target.Name}}",
                    "api": "0.1.0",
                    "version": "0.1.0",
                    "linking": "{{Target.GetTargetType()}}",
                    "author": "official",
                    "license": "EngineDefault(MIT)",
                    "copyright": "EngineDefault",
                    "dependencies": {{DependenciesArray}}
                }
                """;
                string CppContent = $"#include \"SkrCore/module/module.hpp\"\n\nSKR_MODULE_METADATA(u8R\"__de___l___im__({JSON})__de___l___im__\", {Target.Name})";
                File.WriteAllText(GenFileName, CppContent);
                depend.ExternalFiles.Add(GenFileName);
            }, null, DepArgs);

            sw.Stop();
            Time += (int)sw.ElapsedMilliseconds;
            return new PlainArtifact { IsRestored = !Changed };
        }

        private static string FormatDependency(string Dependency)
        {
            var Target = BuildSystem.GetTarget(Dependency);
            return $$"""
            { "name": "{{Dependency}}", "version": "0.1.0", "kind": "shared" }
            """;
        }

        public static volatile int Time = 0;
    }
}
