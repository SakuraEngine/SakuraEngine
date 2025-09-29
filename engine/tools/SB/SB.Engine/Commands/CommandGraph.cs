using SB;
using Serilog;
using System.Diagnostics;

namespace SB;

public class GraphCommand : CommandBase
{
    [Cli.Option(Name = "format", ShortName = 'f', Help = "Output format: dot, mermaid, plantuml", IsRequired = false)]
    public string Format { get; set; } = "dot";

    [Cli.Option(Name = "color", ShortName = 's', Help = "Color scheme: type, module, size, deps", IsRequired = false)]
    public string ColorScheme { get; set; } = "type";

    [Cli.Option(Name = "output", ShortName = 'o', Help = "Output directory for graph files", IsRequired = false)]
    public string OutputDirectory { get; set; } = Path.Combine(BuildDirs.TempDir, "graphs");

    [Cli.Option(Name = "external", ShortName = 'e', Help = "Include external dependencies", IsRequired = false)]
    public bool IncludeExternalDeps { get; set; } = false;

    [Cli.Option(Name = "transitive", ShortName = 't', Help = "Show transitive dependencies (default: false, only direct deps)", IsRequired = false)]
    public bool ShowTransitiveDeps { get; set; } = false;

    [Cli.Option(Name = "depth", ShortName = 'd', Help = "Maximum dependency depth (-1 for unlimited)", IsRequired = false)]
    public int MaxDepth { get; set; } = -1;

    [Cli.Option(Name = "open", Help = "Open generated graph file after creation", IsRequired = false)]
    public bool OpenAfterGeneration { get; set; } = false;

    public override int OnExecute()
    {
        Log.Information("Generating dependency graph...");

        // Clear any previous data
        DependencyGraphEmitter.Clear();

        // Configure the emitter
        DependencyGraphEmitter.Format = ParseFormat(Format);
        DependencyGraphEmitter.ColorScheme = ParseColorScheme(ColorScheme);
        DependencyGraphEmitter.OutputDirectory = OutputDirectory;
        DependencyGraphEmitter.IncludeExternalDeps = IncludeExternalDeps;
        DependencyGraphEmitter.ShowTransitiveDeps = ShowTransitiveDeps;
        DependencyGraphEmitter.MaxDepth = MaxDepth;

        // Add the emitter to the build system
        var emitter = new DependencyGraphEmitter();
        BuildSystem.AddTaskEmitter("DependencyGraph", emitter);

        // Run the build to collect dependency information
        Engine.RunBuild();

        // Generate the dependency graph after build
        DependencyGraphEmitter.GenerateDependencyGraph();

        Log.Information("Dependency graph generated in: {Path}", Path.GetFullPath(OutputDirectory));

        // List generated files
        var graphFiles = Directory.GetFiles(OutputDirectory, "dependency_graph_*.*", SearchOption.TopDirectoryOnly)
            .OrderByDescending(f => File.GetCreationTime(f))
            .Take(2)  // Get the latest graph and stats file
            .ToList();

        foreach (var file in graphFiles)
        {
            Log.Information("  - {File}", Path.GetFileName(file));
        }

        // Optionally open the generated file
        if (OpenAfterGeneration && graphFiles.Any())
        {
            var latestGraph = graphFiles.FirstOrDefault(f => !f.EndsWith("_stats.txt"));
            if (latestGraph != null)
            {
                try
                {
                    if (OperatingSystem.IsWindows())
                    {
                        Process.Start(new ProcessStartInfo
                        {
                            FileName = latestGraph,
                            UseShellExecute = true
                        });
                    }
                    else if (OperatingSystem.IsMacOS())
                    {
                        Process.Start("open", latestGraph);
                    }
                    else if (OperatingSystem.IsLinux())
                    {
                        Process.Start("xdg-open", latestGraph);
                    }
                }
                catch (Exception ex)
                {
                    Log.Warning("Failed to open graph file: {Error}", ex.Message);
                }
            }
        }

        // Provide rendering hints based on format
        if (Format.ToLower() == "dot")
        {
            Log.Information("To render the graph, you can use:");
            Log.Information("  dot -Tpng {OutputDirectory}/dependency_graph_*.dot -o graph.png", OutputDirectory);
            Log.Information("  dot -Tsvg {OutputDirectory}/dependency_graph_*.dot -o graph.svg", OutputDirectory);
        }
        else if (Format.ToLower() == "mermaid")
        {
            Log.Information("To view the Mermaid graph:");
            Log.Information("  - Copy the content to https://mermaid.live/");
            Log.Information("  - Or include in a Markdown file with Mermaid support");
        }
        else if (Format.ToLower() == "plantuml")
        {
            Log.Information("To render the PlantUML graph:");
            Log.Information("  java -jar plantuml.jar {OutputDirectory}/dependency_graph_*.puml", OutputDirectory);
        }

        return 0;
    }

    private DependencyGraphEmitter.GraphFormat ParseFormat(string format)
    {
        return format.ToLower() switch
        {
            "dot" => DependencyGraphEmitter.GraphFormat.DOT,
            "mermaid" => DependencyGraphEmitter.GraphFormat.Mermaid,
            "plantuml" => DependencyGraphEmitter.GraphFormat.PlantUML,
            _ => DependencyGraphEmitter.GraphFormat.DOT
        };
    }

    private DependencyGraphEmitter.NodeColorScheme ParseColorScheme(string scheme)
    {
        return scheme.ToLower() switch
        {
            "type" => DependencyGraphEmitter.NodeColorScheme.ByType,
            "module" => DependencyGraphEmitter.NodeColorScheme.ByModule,
            "size" => DependencyGraphEmitter.NodeColorScheme.BySize,
            "deps" => DependencyGraphEmitter.NodeColorScheme.ByDependencyCount,
            _ => DependencyGraphEmitter.NodeColorScheme.ByType
        };
    }

    [Cli.RegisterCmd(Name = "graph", Help = "Generate dependency graph visualization", Usage = "SB graph [options]")]
    public static object RegisterCommand() => new GraphCommand();
}
