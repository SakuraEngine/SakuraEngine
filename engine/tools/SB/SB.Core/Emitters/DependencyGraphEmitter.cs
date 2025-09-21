using SB.Core;
using System;
using System.Text;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Serilog;

namespace SB
{
    public class DependencyGraphEmitter : TaskEmitter
    {
        public enum GraphFormat { DOT, Mermaid, PlantUML }
        public enum NodeColorScheme { ByType, ByModule, BySize, ByDependencyCount }

        public static GraphFormat Format { get; set; } = GraphFormat.DOT;
        public static NodeColorScheme ColorScheme { get; set; } = NodeColorScheme.ByType;
        public static bool IncludeExternalDeps { get; set; } = false;
        public static bool ShowTransitiveDeps { get; set; } = false;
        public static int MaxDepth { get; set; } = -1;

        private static readonly Dictionary<string, TargetInfo> targetInfos = new();
        private static readonly HashSet<(string from, string to)> edges = new();
        private static readonly object syncLock = new object();

        private class TargetInfo
        {
            public Target Target { get; set; } = null!;
            public long CodeSize { get; set; }
            public int SourceFileCount { get; set; }
            public int DirectDependencyCount { get; set; }
            public string ModulePath { get; set; } = "";
        }

        public override bool EnableEmitter(Target Target) => true;
        public override bool EmitTargetTask(Target Target) => true;


        public static string? OutputDirectory { get; set; }


        public static void Clear()
        {
            lock (syncLock)
            {
                targetInfos.Clear();
                edges.Clear();
            }
        }

        public override IArtifact? PerTargetTask(Target Target)
        {
            var info = new TargetInfo
            {
                Target = Target,
                ModulePath = Path.GetRelativePath(Environment.CurrentDirectory, Target.Directory).Replace('\\', '/'),
                SourceFileCount = Target.FileLists.Sum(fl => fl.Files.Count(f =>
                    f.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase) ||
                    f.EndsWith(".c", StringComparison.OrdinalIgnoreCase))),
                DirectDependencyCount = Target.PublicDependencies.Count +
                                      Target.PrivateDependencies.Count +
                                      Target.InterfaceDependencies.Count
            };
            info.CodeSize = info.SourceFileCount * 5000;

            var deps = new HashSet<string>();
            if (ShowTransitiveDeps)
                foreach (var depName in Target.Dependencies) deps.Add(depName);
            else
            {
                foreach (var depName in Target.PublicDependencies) deps.Add(depName);
                foreach (var depName in Target.PrivateDependencies) deps.Add(depName);
                foreach (var depName in Target.InterfaceDependencies) deps.Add(depName);
            }

            lock (syncLock)
            {
                targetInfos[Target.Name] = info;
                foreach (var depName in deps)
                    edges.Add((Target.Name, depName));
            }

            return null;
        }

        public static void GenerateDependencyGraph()
        {
            if (targetInfos.Count == 0)
            {
                Log.Warning("No targets collected. Make sure to run build before generating graph.");
                return;
            }

            Directory.CreateDirectory(OutputDirectory!);
            var timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            var filename = $"dependency_graph_{timestamp}";

            Dictionary<string, TargetInfo> targetsCopy;
            HashSet<(string from, string to)> edgesCopy;
            lock (syncLock)
            {
                targetsCopy = new Dictionary<string, TargetInfo>(targetInfos);
                edgesCopy = new HashSet<(string from, string to)>(edges);
            }

            var outputPath = Path.Combine(OutputDirectory!, filename);
            switch (Format)
            {
                case GraphFormat.DOT:
                    GenerateDot(targetsCopy, edgesCopy, $"{outputPath}.dot");
                    break;
                case GraphFormat.Mermaid:
                    GenerateMermaid(targetsCopy, edgesCopy, $"{outputPath}.mmd");
                    break;
                case GraphFormat.PlantUML:
                    GeneratePlantUML(targetsCopy, edgesCopy, $"{outputPath}.puml");
                    break;
            }

            GenerateStats(targetsCopy, edgesCopy, $"{outputPath}_stats.txt");
            Log.Information($"Generated dependency graph with {targetsCopy.Count} targets and {edgesCopy.Count} edges");
        }

        private static void GenerateDot(Dictionary<string, TargetInfo> targets, HashSet<(string from, string to)> edges, string path)
        {
            var sb = new StringBuilder();
            sb.AppendLine("digraph DependencyGraph {");
            sb.AppendLine("    rankdir=LR;");
            sb.AppendLine("    node [shape=box, style=\"rounded,filled\", fontname=\"Arial\"];");
            sb.AppendLine("    edge [fontname=\"Arial\", fontsize=10];");
            sb.AppendLine();

            foreach (var (name, info) in targets)
            {
                var color = GetNodeColor(info);
                var label = GetNodeLabel(name, info);
                var shape = info.Target.GetTargetType() switch
                {
                    TargetType.Executable => "box3d",
                    TargetType.Dynamic => "component",
                    _ => "box"
                };
                sb.AppendLine($"    \"{name}\" [label=\"{label}\", fillcolor=\"{color}\", shape={shape}];");
            }

            sb.AppendLine();
            foreach (var (from, to) in edges)
                sb.AppendLine($"    \"{from}\" -> \"{to}\" [color=\"darkgray\"];");

            // Legend
            sb.AppendLine("\n    subgraph cluster_legend {");
            sb.AppendLine("        label=\"Legend\";");
            sb.AppendLine("        style=dashed;");
            if (ColorScheme == NodeColorScheme.ByType)
            {
                sb.AppendLine("        \"Static Library\" [fillcolor=\"lightblue\"];");
                sb.AppendLine("        \"Dynamic Library\" [fillcolor=\"lightgreen\"];");
                sb.AppendLine("        \"Executable\" [fillcolor=\"lightyellow\"];");
            }
            sb.AppendLine("    }");
            sb.AppendLine("}");

            File.WriteAllText(path, sb.ToString());
            Log.Information($"Generated DOT file: {path}");
        }

        private static void GenerateMermaid(Dictionary<string, TargetInfo> targets, HashSet<(string from, string to)> edges, string path)
        {
            var sb = new StringBuilder();
            sb.AppendLine("graph LR");
            sb.AppendLine();

            var moduleGroups = targets.GroupBy(t => t.Value.ModulePath);
            foreach (var module in moduleGroups)
            {
                if (!string.IsNullOrEmpty(module.Key))
                    sb.AppendLine($"    subgraph {SanitizeId(module.Key)}[\"{module.Key}\"]");

                foreach (var (name, info) in module)
                {
                    var label = GetNodeLabel(name, info);
                    var style = info.Target.GetTargetType() switch
                    {
                        TargetType.Executable => "executable",
                        TargetType.Dynamic => "dynamic",
                        _ => "static"
                    };
                    sb.AppendLine($"        {SanitizeId(name)}[\"{label}\"]:::{style}");
                }

                if (!string.IsNullOrEmpty(module.Key))
                    sb.AppendLine("    end");
            }

            sb.AppendLine();
            foreach (var (from, to) in edges)
                sb.AppendLine($"    {SanitizeId(from)} --> {SanitizeId(to)}");

            sb.AppendLine("\n    classDef executable fill:#ffffcc,stroke:#333,stroke-width:2px;");
            sb.AppendLine("    classDef static fill:#ccccff,stroke:#333,stroke-width:2px;");
            sb.AppendLine("    classDef dynamic fill:#ccffcc,stroke:#333,stroke-width:2px;");

            File.WriteAllText(path, sb.ToString());
            Log.Information($"Generated Mermaid file: {path}");
        }

        private static void GeneratePlantUML(Dictionary<string, TargetInfo> targets, HashSet<(string from, string to)> edges, string path)
        {
            var sb = new StringBuilder();
            sb.AppendLine("@startuml");
            sb.AppendLine("!define RECTANGLE class");
            sb.AppendLine("skinparam componentStyle rectangle");
            sb.AppendLine();

            foreach (var (name, info) in targets)
            {
                var stereotype = info.Target.GetTargetType() switch
                {
                    TargetType.Executable => "<<executable>>",
                    TargetType.Dynamic => "<<dynamic>>",
                    _ => "<<static>>"
                };
                var label = GetNodeLabel(name, info);
                sb.AppendLine($"component \"{label}\" as {name.Replace('-', '_')} {stereotype}");
            }

            sb.AppendLine();
            foreach (var (from, to) in edges)
                sb.AppendLine($"{from.Replace('-', '_')} --> {to.Replace('-', '_')}");

            sb.AppendLine("@enduml");

            File.WriteAllText(path, sb.ToString());
            Log.Information($"Generated PlantUML file: {path}");
        }

        private static void GenerateStats(Dictionary<string, TargetInfo> targets, HashSet<(string from, string to)> edges, string path)
        {
            var sb = new StringBuilder();
            sb.AppendLine("=== Dependency Graph Statistics ===");
            sb.AppendLine($"Generated: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
            sb.AppendLine($"\nTotal Targets: {targets.Count}");
            sb.AppendLine($"Total Dependencies: {edges.Count}");

            sb.AppendLine("\nTarget Types:");
            var typeGroups = targets.GroupBy(t => t.Value.Target.GetTargetType());
            foreach (var group in typeGroups)
                sb.AppendLine($"  {group.Key}: {group.Count()}");

            sb.AppendLine("\nTop 10 Targets by Direct Dependencies:");
            foreach (var (name, info) in targets.OrderByDescending(t => t.Value.DirectDependencyCount).Take(10))
                sb.AppendLine($"  {name}: {info.DirectDependencyCount} dependencies");

            sb.AppendLine("\nTop 10 Most Depended Upon Targets:");
            var dependedUpon = edges.GroupBy(e => e.to).OrderByDescending(g => g.Count()).Take(10);
            foreach (var group in dependedUpon)
                sb.AppendLine($"  {group.Key}: {group.Count()} dependents");

            // Circular dependencies detection
            var cycles = FindCircularDependencies(targets.Keys, edges);
            if (cycles.Any())
            {
                sb.AppendLine("\nWARNING: Circular Dependencies Detected:");
                foreach (var cycle in cycles)
                    sb.AppendLine($"  {string.Join(" -> ", cycle)} -> {cycle.First()}");
            }
            else
                sb.AppendLine("\nNo circular dependencies detected ✓");

            File.WriteAllText(path, sb.ToString());
            Log.Information($"Generated statistics report: {path}");
        }

        private static List<List<string>> FindCircularDependencies(IEnumerable<string> targets, HashSet<(string from, string to)> edges)
        {
            var cycles = new List<List<string>>();
            var visited = new HashSet<string>();
            var recursionStack = new HashSet<string>();
            var path = new List<string>();

            void FindCycles(string node)
            {
                visited.Add(node);
                recursionStack.Add(node);
                path.Add(node);

                foreach (var neighbor in edges.Where(e => e.from == node).Select(e => e.to))
                {
                    if (!visited.Contains(neighbor))
                        FindCycles(neighbor);
                    else if (recursionStack.Contains(neighbor))
                    {
                        var cycleStart = path.IndexOf(neighbor);
                        if (cycleStart >= 0)
                            cycles.Add(path.Skip(cycleStart).ToList());
                    }
                }

                path.RemoveAt(path.Count - 1);
                recursionStack.Remove(node);
            }

            foreach (var target in targets)
                if (!visited.Contains(target))
                    FindCycles(target);

            return cycles;
        }

        private static string GetNodeLabel(string name, TargetInfo info)
        {
            var lines = new List<string> { name };
            lines.Add($"Type: {info.Target.GetTargetType()}");
            lines.Add($"Files: {info.SourceFileCount}");
            if (info.CodeSize > 0)
                lines.Add($"Size: {FormatSize(info.CodeSize)}");
            lines.Add($"Deps: {info.DirectDependencyCount}");
            return string.Join("\\n", lines);
        }

        private static string GetNodeColor(TargetInfo info)
        {
            return ColorScheme switch
            {
                NodeColorScheme.ByType => info.Target.GetTargetType() switch
                {
                    TargetType.Static => "lightblue",
                    TargetType.Dynamic => "lightgreen",
                    TargetType.Executable => "lightyellow",
                    _ => "lightgray"
                },
                NodeColorScheme.ByModule => info.ModulePath.Contains("core") ? "lightcoral" :
                                           info.ModulePath.Contains("engine") ? "lightblue" :
                                           info.ModulePath.Contains("game") ? "lightgreen" :
                                           info.ModulePath.Contains("tools") ? "lightyellow" : "lightgray",
                NodeColorScheme.BySize => info.CodeSize < 10000 ? "#e6f3ff" :
                                         info.CodeSize < 50000 ? "#cce6ff" :
                                         info.CodeSize < 100000 ? "#99ccff" :
                                         info.CodeSize < 500000 ? "#66b2ff" : "#3399ff",
                NodeColorScheme.ByDependencyCount => info.DirectDependencyCount == 0 ? "#f0f0f0" :
                                                    info.DirectDependencyCount < 3 ? "#ffe6e6" :
                                                    info.DirectDependencyCount < 5 ? "#ffcccc" :
                                                    info.DirectDependencyCount < 10 ? "#ffb3b3" : "#ff9999",
                _ => "lightgray"
            };
        }

        private static string FormatSize(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB" };
            int order = 0;
            double size = bytes;
            while (size >= 1024 && order < sizes.Length - 1)
            {
                order++;
                size /= 1024;
            }
            return $"{size:0.##} {sizes[order]}";
        }

        private static string SanitizeId(string id) => id.Replace('-', '_').Replace('/', '_');
    }
}