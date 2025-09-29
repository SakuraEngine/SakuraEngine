using SB.Core;
using System;
using System.Text;
using System.Xml.Linq;
using System.Collections.Concurrent;
using System.Linq;
using System.Collections.Generic;
using System.IO;

namespace SB
{
    using BS = BuildSystem;

    public class VSProjectInfo
    {
        public string ProjectPath { get; set; } = "";
        public string ProjectGuid { get; set; } = "";
        public string TargetDirectory { get; set; } = "";
        public string FolderPath { get; set; } = "";
        public HashSet<string> SourceFiles { get; } = new();
        public HashSet<string> HeaderFiles { get; } = new();
        public Dictionary<string, object?> CompileArguments { get; } = new();
        public List<string> IncludePaths { get; } = new();
        public List<string> Defines { get; } = new();
        public List<string> CompilerFlags { get; } = new();
    }

    public class VSEmitter : TaskEmitter
    {
        #region Constants

        // XML Namespaces and GUIDs
        private const string VS_NAMESPACE = "http://schemas.microsoft.com/developer/msbuild/2003";
        private const string CPP_PROJECT_GUID = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}";
        private const string SOLUTION_FOLDER_GUID = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}";

        // Build Configurations
        private static readonly string[] Configurations = { "Debug", "Release" };
        private static readonly string[] Platforms = { "x64", "Win32" };

        // File Extensions
        private static readonly string[] SourceExtensions = { ".cpp", ".cc", ".c", ".m", ".mm" };
        private static readonly string[] HeaderExtensions = { ".h", ".hpp", ".hxx", ".inl", ".inc" };

        // Compiler Settings
        private static readonly string[] CompilerSettingKeys = { "CppVersion", "WarningLevel", "OptimizationLevel", "RTTI", "Exception" };

        #endregion

        public VSEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;

        public static string RootDirectory { get; set; } = "./";
        public static int SolutionFolderDepth { get; set; } = 2;

        public static string? OutputDirectory { get; set; }

        public override bool EnableEmitter(Target Target) =>
            Target.HasFilesOf<CppFileList>() ||
            Target.HasFilesOf<CFileList>() ||
            Target.HasFilesOf<ObjCppFileList>() ||
            Target.HasFilesOf<ObjCFileList>();

        public override bool EmitTargetTask(Target Target) => true;

        public override IArtifact? PerTargetTask(Target Target)
        {
            // Create output directory structure
            var relativeTargetPath = Path.GetRelativePath(RootDirectory, Target.Directory);
            var projectOutputDir = Path.Combine(OutputDirectory!, relativeTargetPath);
            Directory.CreateDirectory(projectOutputDir);

            // Collect all source files and generate compile arguments
            var projectInfo = new VSProjectInfo
            {
                ProjectPath = Path.Combine(projectOutputDir, $"{Target.Name}.vcxproj"),
                ProjectGuid = GenerateGuid(Target.Name),
                TargetDirectory = Target.Directory,
                FolderPath = GetTargetFolderPath(Target)
            };

            // Process all file lists
            ProcessFileList<CppFileList>(Target, projectInfo, CFamily.Cpp);
            ProcessFileList<CFileList>(Target, projectInfo, CFamily.C);
            ProcessFileList<ObjCppFileList>(Target, projectInfo, CFamily.ObjCpp);
            ProcessFileList<ObjCFileList>(Target, projectInfo, CFamily.ObjC);

            // Extract compile arguments using ArgumentDriver
            ExtractCompileArguments(Target, projectInfo);

            // Scan include directories for header files
            ScanIncludeDirectories(Target, projectInfo);

            // Store project info for solution generation (must be done before generating project files)
            ProjectInfos.TryAdd(Target.Name, projectInfo);

            // Generate project files
            GenerateVcxproj(Target, projectInfo);
            GenerateVcxprojFilters(Target, projectInfo);

            return null;
        }

        private void ProcessFileList<T>(Target Target, VSProjectInfo projectInfo, CFamily language)
            where T : FileList, new()
        {
            var fileList = Target.FileList<T>();
            if (fileList == null) return;

            foreach (var file in fileList.Files)
            {
                var ext = Path.GetExtension(file).ToLower();
                var fullPath = GetAbsolutePath(file, Target.Directory);

                if (SourceExtensions.Contains(ext))
                    projectInfo.SourceFiles.Add(fullPath);
                else if (HeaderExtensions.Contains(ext))
                    projectInfo.HeaderFiles.Add(fullPath);
            }
        }

        private void ScanIncludeDirectories(Target Target, VSProjectInfo projectInfo)
        {
            var targetDir = Target.Directory;
            var relevantIncludes = projectInfo.IncludePaths
                .Select(path => Path.IsPathFullyQualified(path) ? path : Path.GetFullPath(Path.Combine(targetDir, path)))
                .Where(path => IsSubdirectory(targetDir, path))
                .ToList();

            // Create a normalized set of existing headers to avoid duplicates
            var normalizedExistingHeaders = new HashSet<string>(
                projectInfo.HeaderFiles.Select(h => Path.GetFullPath(h).ToLowerInvariant()),
                StringComparer.OrdinalIgnoreCase);

            // Scan each relevant include directory for header files
            foreach (var includeDir in relevantIncludes)
            {
                if (Directory.Exists(includeDir))
                {
                    try
                    {
                        var headers = Directory.GetFiles(includeDir, "*.*", SearchOption.AllDirectories)
                            .Where(file => HeaderExtensions.Contains(Path.GetExtension(file).ToLower()));

                        foreach (var header in headers)
                        {
                            var normalizedPath = Path.GetFullPath(header).ToLowerInvariant();
                            if (!normalizedExistingHeaders.Contains(normalizedPath))
                            {
                                projectInfo.HeaderFiles.Add(header);
                                normalizedExistingHeaders.Add(normalizedPath);
                            }
                        }
                    }
                    catch (Exception)
                    {
                        // Ignore directories that can't be accessed
                    }
                }
            }
        }

        private void ExtractCompileArguments(Target Target, VSProjectInfo projectInfo)
        {
            // Create ArgumentDriver to extract compile settings
            var driver = Toolchain.Compiler.CreateArgumentDriver(CFamily.Cpp, false)
                .AddArguments(Target.Arguments);

            // Calculate all arguments from the driver
            var calculatedArgs = driver.CalculateArguments();

            // Store raw arguments for later use
            foreach (var kvp in Target.Arguments)
            {
                projectInfo.CompileArguments[kvp.Key] = kvp.Value;
            }

            // Extract include directories and defines
            ExtractFromArgs(calculatedArgs, "IncludeDirs", projectInfo.IncludePaths, "/I", "-I");
            ExtractFromArgs(calculatedArgs, "Defines", projectInfo.Defines, "/D", "-D");

            // Extract compiler flags
            var allFlags = ExtractCompilerFlags(calculatedArgs);

            projectInfo.CompilerFlags.AddRange(allFlags);
        }

        private void GenerateVcxproj(Target Target, VSProjectInfo projectInfo)
        {
            var ns = XNamespace.Get(VS_NAMESPACE);

            var project = new XElement(ns + "Project",
                new XAttribute("DefaultTargets", "Build"),
                new XAttribute("ToolsVersion", "15.0"));

            // Project configurations
            project.Add(CreateProjectConfigurations(ns));

            // Globals
            project.Add(new XElement(ns + "PropertyGroup",
                new XAttribute("Label", "Globals"),
                new XElement(ns + "ProjectGuid", projectInfo.ProjectGuid),
                new XElement(ns + "RootNamespace", Target.Name),
                new XElement(ns + "WindowsTargetPlatformVersion", "10.0")));

            // Import default props
            project.Add(CreateImport(ns, @"$(VCTargetsPath)\Microsoft.Cpp.Default.props"));

            // Configuration properties
            ForEachConfiguration((config, platform) =>
            {
                project.Add(CreatePropertyGroup(ns, config, platform, new[]
                {
                    ("ConfigurationType", "Makefile"),
                    ("UseDebugLibraries", config == "Debug" ? "true" : "false"),
                    ("PlatformToolset", "v143"),
                    ("CharacterSet", "Unicode"),
                    // 确保项目总是被认为是过期的，需要重新构建
                    ("BuildInParallel", "false")
                }, "Configuration"));
            });

            project.Add(CreateImport(ns, @"$(VCTargetsPath)\Microsoft.Cpp.props"));

            // Build customizations
            AddBuildCustomizations(project, ns, Target);

            // Compiler settings
            AddCompilerSettings(project, ns, Target, projectInfo);

            // Source files - Use ClCompile for IntelliSense but with ExcludedFromBuild=true
            // This provides better IntelliSense while preventing actual compilation
            if (projectInfo.SourceFiles.Any())
            {
                project.Add(new XElement(ns + "ItemGroup",
                    projectInfo.SourceFiles.Select(file =>
                        CreateClCompileElement(ns, file, projectInfo))));
            }

            // Header files
            if (projectInfo.HeaderFiles.Any())
            {
                project.Add(new XElement(ns + "ItemGroup",
                    projectInfo.HeaderFiles.Select(file =>
                        new XElement(ns + "ClInclude",
                            new XAttribute("Include", GetFileProjectPath(file, projectInfo))))));
            }

            // 注意：不需要添加 None 项，因为 ClCompile 和 ClInclude 项已经足够让 Visual Studio 跟踪依赖关系
            // Visual Studio 会根据这些文件的修改时间来决定是否需要重新构建

            project.Add(CreateImport(ns, @"$(VCTargetsPath)\Microsoft.Cpp.targets"));

            // 添加自定义 MSBuild target 来强制 Rider 每次都执行构建
            project.Add(new XElement(ns + "Target",
                new XAttribute("Name", "AlwaysBuild"),
                new XAttribute("BeforeTargets", "Build"),
                new XElement(ns + "Message",
                    new XAttribute("Text", "Forcing build check for Rider compatibility"),
                    new XAttribute("Importance", "low")),
                // 创建一个虚拟的输出文件，让 MSBuild 认为总是需要重新构建
                new XElement(ns + "Touch",
                    new XAttribute("Files", Path.Combine("$(IntDir)", "force_rebuild.timestamp")),
                    new XAttribute("AlwaysCreate", "true"))));

            SaveXmlDocument(project, projectInfo.ProjectPath);
        }

        private void GenerateVcxprojFilters(Target Target, VSProjectInfo projectInfo)
        {
            var ns = XNamespace.Get(VS_NAMESPACE);
            var project = new XElement(ns + "Project", new XAttribute("ToolsVersion", "4.0"));

            // Create filter structure based on target directory hierarchy
            var filters = new HashSet<string>();
            var sourceFilters = new Dictionary<string, string>();
            var headerFilters = new Dictionary<string, string>();

            // Helper to collect all unique directory paths including parents
            void CollectAllPaths(string path)
            {
                if (string.IsNullOrEmpty(path)) return;

                // Normalize to use backslashes for VS
                path = path.Replace('/', '\\');
                filters.Add(path);

                // Add parent path
                var parentPath = Path.GetDirectoryName(path);
                if (!string.IsNullOrEmpty(parentPath))
                {
                    CollectAllPaths(parentPath);
                }
            }

            // Process source and header files
            ProcessFiles(projectInfo.SourceFiles, projectInfo, sourceFilters, CollectAllPaths);
            ProcessFiles(projectInfo.HeaderFiles, projectInfo, headerFilters, CollectAllPaths);

            // Create filters sorted by depth to ensure parents are created first
            if (filters.Any())
            {
                var sortedFilters = filters
                    .OrderBy(f => f.Count(c => c == '\\'))
                    .ThenBy(f => f);

                project.Add(new XElement(ns + "ItemGroup",
                    sortedFilters.Select(filter =>
                        new XElement(ns + "Filter",
                            new XAttribute("Include", filter),
                            new XElement(ns + "UniqueIdentifier", $"{{{Guid.NewGuid()}}}")
                        ))));
            }

            // Add source files with filters
            if (sourceFilters.Any())
            {
                project.Add(new XElement(ns + "ItemGroup",
                    sourceFilters.Select(kvp =>
                        new XElement(ns + "ClCompile",
                            new XAttribute("Include", GetFileProjectPath(kvp.Key, projectInfo)),
                            new XElement(ns + "Filter", kvp.Value)))));
            }

            // Add header files with filters
            if (headerFilters.Any())
            {
                project.Add(new XElement(ns + "ItemGroup",
                    headerFilters.Select(kvp =>
                        new XElement(ns + "ClInclude",
                            new XAttribute("Include", GetFileProjectPath(kvp.Key, projectInfo)),
                            new XElement(ns + "Filter", kvp.Value)))));
            }

            SaveXmlDocument(project, projectInfo.ProjectPath + ".filters");
        }

        private void AddBuildCustomizations(XElement project, XNamespace ns, Target Target)
        {
            if (!ProjectInfos.TryGetValue(Target.Name, out var projectInfo))
                throw new InvalidOperationException($"Project info for target '{Target.Name}' not found.");

            var outDir = Path.GetDirectoryName(GetTargetOutputPath(Target));
            ForEachConfiguration((config, platform) =>
            {
                project.Add(CreatePropertyGroup(ns, config, platform, GetNMakeProperties(Target, config, platform, outDir!)));
            });
        }

        private void AddCompilerSettings(XElement project, XNamespace ns, Target Target, VSProjectInfo projectInfo)
        {
            var projectDir = Path.GetDirectoryName(projectInfo.ProjectPath) ?? "";
            var relativeIncludes = GetRelativeIncludes(projectInfo.IncludePaths, projectDir);
            var forcedIncludes = ExtractForcedIncludes(projectInfo.CompilerFlags);

            ForEachConfiguration((config, platform) =>
            {
                // IntelliSense configuration
                var intelliSenseProps = new List<(string, string)>();
                if (relativeIncludes.Any())
                    intelliSenseProps.Add(("IncludePath", string.Join(";", relativeIncludes) + ";$(IncludePath)"));
                if (projectInfo.Defines.Any())
                    intelliSenseProps.Add(("NMakePreprocessorDefinitions", string.Join(";", projectInfo.Defines) + ";$(NMakePreprocessorDefinitions)"));
                if (forcedIncludes.Any())
                    intelliSenseProps.Add(("NMakeForcedIncludes", string.Join(";", forcedIncludes)));

                project.Add(CreatePropertyGroup(ns, config, platform, intelliSenseProps));

                // ClCompile settings
                var itemDefGroup = new XElement(ns + "ItemDefinitionGroup", new XAttribute("Condition", $"'$(Configuration)|$(Platform)'=='{config}|{platform}'"));
                itemDefGroup.Add(CreateClCompileSettings(ns, projectInfo, config, projectDir));
                project.Add(itemDefGroup);
            });
        }

        private List<string> ExtractForcedIncludes(List<string> compilerFlags)
        {
            var forcedIncludes = new List<string>();

            for (int i = 0; i < compilerFlags.Count; i++)
            {
                var flag = compilerFlags[i];
                if (flag == "/FI" || flag == "-include")
                {
                    // Next argument is the include file
                    if (i + 1 < compilerFlags.Count)
                    {
                        forcedIncludes.Add(compilerFlags[i + 1].Trim('"'));
                        i++; // Skip the include file argument
                    }
                }
                else if (flag.StartsWith("/FI") && flag.Length > 3)
                {
                    // Include file is in the same argument
                    forcedIncludes.Add(flag.Substring(3).Trim('"'));
                }
                else if (flag.StartsWith("-include="))
                {
                    // Include file is in the same argument with equals
                    forcedIncludes.Add(flag.Substring(9).Trim('"'));
                }
            }

            return forcedIncludes;
        }

        private XElement CreateClCompileSettings(XNamespace ns, VSProjectInfo projectInfo, string config, string projectDir)
        {
            var clCompile = new XElement(ns + "ClCompile");
            var relativeIncludes = GetRelativeIncludes(projectInfo.IncludePaths, projectDir);
            var forcedIncludes = ExtractForcedIncludes(projectInfo.CompilerFlags);

            if (relativeIncludes.Any())
                clCompile.Add(new XElement(ns + "AdditionalIncludeDirectories", string.Join(";", relativeIncludes) + ";%(AdditionalIncludeDirectories)"));
            if (projectInfo.Defines.Any())
                clCompile.Add(new XElement(ns + "PreprocessorDefinitions", string.Join(";", projectInfo.Defines) + ";%(PreprocessorDefinitions)"));
            if (projectInfo.CompileArguments.TryGetValue("CppVersion", out var cppVersion) && cppVersion != null)
                clCompile.Add(new XElement(ns + "LanguageStandard", MapCppStandard(cppVersion.ToString() ?? "")));
            if (forcedIncludes.Any())
            {
                var relativeForced = forcedIncludes.Select(inc => Path.IsPathFullyQualified(inc) ? GetRelativePath(projectDir, inc) : inc);
                clCompile.Add(new XElement(ns + "ForcedIncludeFiles", string.Join(";", relativeForced) + ";%(ForcedIncludeFiles)"));
            }

            AddCompilerFlagSettings(clCompile, ns, projectInfo.CompilerFlags, config);
            return clCompile;
        }

        private void AddCompilerFlagSettings(XElement clCompile, XNamespace ns, List<string> compilerFlags, string config)
        {
            var settings = new Dictionary<string, string>
            {
                ["WarningLevel"] = "Level3",
                ["Optimization"] = config == "Debug" ? "Disabled" : "MaxSpeed",
                ["RuntimeLibrary"] = config == "Debug" ? "MultiThreadedDebugDLL" : "MultiThreadedDLL"
            };

            foreach (var flag in compilerFlags)
            {
                switch (flag)
                {
                    case "/GR-" or "-fno-rtti": settings["RuntimeTypeInfo"] = "false"; break;
                    case "/GR" or "-frtti": settings["RuntimeTypeInfo"] = "true"; break;
                    case "/EHsc" or "-fexceptions": settings["ExceptionHandling"] = "Sync"; break;
                    case "/EHa": settings["ExceptionHandling"] = "Async"; break;
                    case "/EH-" or "-fno-exceptions": settings["ExceptionHandling"] = "false"; break;
                    case var f when f.StartsWith("/W") && f.Length == 3 && char.IsDigit(f[2]):
                        settings["WarningLevel"] = $"Level{f[2]}"; break;
                    case "/O1": settings["Optimization"] = "MinSpace"; break;
                    case "/O2": settings["Optimization"] = "MaxSpeed"; break;
                }
            }

            foreach (var (key, value) in settings)
                clCompile.Add(new XElement(ns + key, value));
        }

        private XElement CreateClCompileElement(XNamespace ns, string file, VSProjectInfo projectInfo)
        {
            var projectDir = Path.GetDirectoryName(projectInfo.ProjectPath) ?? "";
            var clCompile = CreateClCompileSettings(ns, projectInfo, "Debug", projectDir);
            clCompile.SetAttributeValue("Include", GetFileProjectPath(file, projectInfo));
            clCompile.Add(new XElement(ns + "ExcludedFromBuild", "true"));

            var compileAs = GetCompileAsType(Path.GetExtension(file));
            if (compileAs != null)
                clCompile.Add(new XElement(ns + "CompileAs", compileAs));

            return clCompile;
        }


        private string GetTargetOutputPath(Target Target)
        {
            var targetType = Target.GetTargetType();
            var extension = targetType switch
            {
                TargetType.Executable => ".exe",
                TargetType.Dynamic => ".dll",
                TargetType.Static => ".lib",
                _ => ""
            };
            var relativePath = Path.Combine(Target.GetBinaryDir(), Target.Name + extension);
            // 确保返回绝对路径，这对 Visual Studio 检测文件状态很重要
            return Path.IsPathFullyQualified(relativePath) ? relativePath : Path.GetFullPath(Path.Combine(RootDirectory, relativePath));
        }

        private string MapCppStandard(string version)
        {
            return version switch
            {
                "c++11" or "11" => "stdcpp11",
                "c++14" or "14" => "stdcpp14",
                "c++17" or "17" => "stdcpp17",
                "c++20" or "20" => "stdcpp20",
                "c++23" or "23" => "stdcpp23",
                "latest" => "stdcpplatest",
                _ => "stdcpp17" // Default
            };
        }

        // Helper methods
        private void ForEachConfiguration(Action<string, string> action)
        {
            foreach (var config in Configurations)
                foreach (var platform in Platforms)
                    action(config, platform);
        }

        private XElement CreatePropertyGroup(XNamespace ns, string config, string platform, IEnumerable<(string key, string value)> properties, string? label = null)
        {
            var group = new XElement(ns + "PropertyGroup", new XAttribute("Condition", $"'$(Configuration)|$(Platform)'=='{config}|{platform}'"));
            if (label != null)
                group.SetAttributeValue("Label", label);
            foreach (var (key, value) in properties)
                group.Add(new XElement(ns + key, value));
            return group;
        }

        private IEnumerable<string> GetRelativeIncludes(List<string> includePaths, string projectDir)
        {
            return includePaths.Select(inc => Path.IsPathFullyQualified(inc) ? GetRelativePath(projectDir, inc) : inc);
        }

        private string GetFileDisplayPath(string filePath, VSProjectInfo projectInfo)
        {
            // Calculate path relative to target directory for display in VS
            return Path.IsPathFullyQualified(filePath)
                ? Path.GetRelativePath(projectInfo.TargetDirectory, filePath)
                : filePath;
        }

        private string GetFileProjectPath(string filePath, VSProjectInfo projectInfo)
        {
            // Calculate path relative to project file for VS project references
            var projectDir = Path.GetDirectoryName(projectInfo.ProjectPath) ?? "";
            return Path.IsPathFullyQualified(filePath)
                ? GetRelativePath(projectDir, filePath)
                : filePath;
        }

        private string? GetCompileAsType(string extension)
        {
            return extension.ToLower() switch
            {
                ".c" => "CompileAsC",
                ".cpp" or ".cc" or ".cxx" => "CompileAsCpp",
                ".mm" => "CompileAsObjCpp",
                ".m" => "CompileAsObjC",
                _ => null
            };
        }

        private string GetTargetFolderPath(Target target)
        {
            // Get relative path from root to target directory
            var relativePath = Path.GetRelativePath(RootDirectory, target.Directory);

            // Split path and take up to SolutionFolderDepth levels
            var parts = relativePath.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                .Where(p => !string.IsNullOrEmpty(p) && p != ".")
                .Take(SolutionFolderDepth)
                .ToArray();

            // Return the folder path with backslashes for VS
            return parts.Length > 0 ? string.Join("\\", parts) : "";
        }

        private static string GenerateGuid(string name)
        {
            using var md5 = System.Security.Cryptography.MD5.Create();
            var hash = md5.ComputeHash(Encoding.UTF8.GetBytes(name));
            return "{" + new Guid(hash).ToString().ToUpper() + "}";
        }

        private string GetRelativePath(string basePath, string fullPath)
        {
            var relativePath = GetSafeRelativePath(basePath, fullPath);

            // Handle case where paths are on different drives (Windows)
            if (!string.IsNullOrEmpty(basePath) && !string.IsNullOrEmpty(fullPath))
            {
                basePath = Path.GetFullPath(basePath);
                fullPath = Path.GetFullPath(fullPath);
                if (Path.GetPathRoot(basePath) != Path.GetPathRoot(fullPath))
                    return fullPath;
            }

            // Convert forward slashes to backslashes for consistency
            return relativePath.Replace('/', Path.DirectorySeparatorChar);
        }

        public static void GenerateSolution(string solutionPath, string solutionName)
        {
            // Ensure the solution directory exists
            var solutionDir = Path.GetDirectoryName(solutionPath);
            if (!string.IsNullOrEmpty(solutionDir))
            {
                Directory.CreateDirectory(solutionDir);
            }

            var sb = new StringBuilder();

            // Solution header
            WriteSolutionHeader(sb);

            // Group projects by folder path and create nested folder structure
            var folderGuids = new Dictionary<string, string>();
            var allFolderPaths = new HashSet<string>();

            // Collect all folder paths and their parent paths
            CollectFolderPaths(allFolderPaths);

            // Sort paths by depth to ensure parents are created before children
            var sortedPaths = allFolderPaths.OrderBy(p => p.Count(c => c == '\\')).ThenBy(p => p);

            // Create solution folders
            foreach (var folderPath in sortedPaths)
            {
                var folderGuid = GenerateGuid($"SolutionFolder_{folderPath}");
                folderGuids[folderPath] = folderGuid;

                var folderName = folderPath.Split('\\').Last();

                sb.AppendLine($"Project(\"{SOLUTION_FOLDER_GUID}\") = \"{folderName}\", \"{folderName}\", \"{folderGuid}\"");
                sb.AppendLine("EndProject");
            }

            // Add projects
            foreach (var kvp in ProjectInfos)
            {
                var name = kvp.Key;
                var info = kvp.Value;

                var relativePath = GetSafeRelativePath(solutionDir, info.ProjectPath);

                sb.AppendLine($"Project(\"{CPP_PROJECT_GUID}\") = \"{name}\", \"{relativePath}\", \"{info.ProjectGuid}\"");
                sb.AppendLine("EndProject");
            }

            sb.AppendLine("Global");

            // Solution configurations
            WriteSolutionConfigurations(sb);

            // Project configurations
            sb.AppendLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
            foreach (var info in ProjectInfos.Values)
            {
                foreach (var config in new[] { "Debug", "Release" })
                {
                    foreach (var platform in new[] { "x64", "Win32" })
                    {
                        sb.AppendLine($"\t\t{info.ProjectGuid}.{config}|{platform}.ActiveCfg = {config}|{platform}");
                        sb.AppendLine($"\t\t{info.ProjectGuid}.{config}|{platform}.Build.0 = {config}|{platform}");
                    }
                }
            }
            sb.AppendLine("\tEndGlobalSection");

            sb.AppendLine("\tGlobalSection(SolutionProperties) = preSolution");
            sb.AppendLine("\t\tHideSolutionNode = FALSE");
            sb.AppendLine("\tEndGlobalSection");

            // Nested projects section - establish parent-child relationships
            sb.AppendLine("\tGlobalSection(NestedProjects) = preSolution");

            // Assign projects to their folders
            foreach (var kvp in ProjectInfos)
            {
                var info = kvp.Value;
                if (!string.IsNullOrEmpty(info.FolderPath) && folderGuids.TryGetValue(info.FolderPath, out var folderGuid))
                {
                    sb.AppendLine($"\t\t{info.ProjectGuid} = {folderGuid}");
                }
            }

            // Assign child folders to parent folders
            foreach (var folderPath in sortedPaths)
            {
                var parentPath = Path.GetDirectoryName(folderPath)?.Replace('/', '\\');
                if (!string.IsNullOrEmpty(parentPath) && folderGuids.TryGetValue(parentPath, out var parentGuid))
                {
                    sb.AppendLine($"\t\t{folderGuids[folderPath]} = {parentGuid}");
                }
            }

            sb.AppendLine("\tEndGlobalSection");

            sb.AppendLine("EndGlobal");

            File.WriteAllText(solutionPath, sb.ToString());
        }


        private IToolchain Toolchain { get; }
        private static ConcurrentDictionary<string, VSProjectInfo> ProjectInfos = new();

        // Additional helper methods
        private void ExtractFromArgs(Dictionary<string, string[]> args, string key, List<string> destination, params string[] prefixes)
        {
            if (args.TryGetValue(key, out var items))
            {
                destination.AddRange(
                    items.Select(item => StripPrefixes(item, prefixes))
                         .Where(processed => !string.IsNullOrWhiteSpace(processed))
                         .Select(processed => processed.Trim('"'))
                );
            }
        }

        private string StripPrefixes(string value, params string[] prefixes)
        {
            foreach (var prefix in prefixes)
            {
                if (value.StartsWith(prefix))
                    return value.Substring(prefix.Length);
            }
            return value;
        }

        private XElement CreateImport(XNamespace ns, string project)
        {
            return new XElement(ns + "Import", new XAttribute("Project", project));
        }

        private void SaveXmlDocument(XElement root, string path)
        {
            var doc = new XDocument(new XDeclaration("1.0", "utf-8", null), root);
            doc.Save(path);
        }

        private static string GetSafeRelativePath(string? basePath, string? fullPath)
        {
            try
            {
                if (string.IsNullOrEmpty(basePath) || string.IsNullOrEmpty(fullPath))
                    return fullPath ?? "";

                basePath = Path.GetFullPath(basePath);
                fullPath = Path.GetFullPath(fullPath);
                return Path.GetRelativePath(basePath, fullPath);
            }
            catch (Exception)
            {
                return fullPath ?? "";
            }
        }

        private void ProcessFiles(HashSet<string> files, VSProjectInfo projectInfo, Dictionary<string, string> filters, Action<string> collectAction)
        {
            foreach (var file in files)
            {
                var displayPath = GetFileDisplayPath(file, projectInfo);
                var dir = Path.GetDirectoryName(displayPath)?.Replace('/', '\\') ?? "";
                if (!string.IsNullOrEmpty(dir))
                {
                    collectAction(dir);
                    filters[file] = dir;
                }
            }
        }

        private List<string> ExtractCompilerFlags(Dictionary<string, string[]> calculatedArgs)
        {
            var allFlags = new List<string>();
            var flagKeys = new[] { "CppFlags", "CXFlags" }.Concat(CompilerSettingKeys);

            foreach (var key in flagKeys)
            {
                if (calculatedArgs.TryGetValue(key, out var flags))
                {
                    allFlags.AddRange(flags);
                }
            }

            return allFlags;
        }

        private bool IsSubdirectory(string baseDir, string potentialSubDir)
        {
            try
            {
                var relativePath = Path.GetRelativePath(baseDir, potentialSubDir);
                return !relativePath.StartsWith("..");
            }
            catch
            {
                return false;
            }
        }

        private static void WriteSolutionHeader(StringBuilder sb)
        {
            sb.AppendLine("Microsoft Visual Studio Solution File, Format Version 12.00");
            sb.AppendLine("# Visual Studio Version 17");
            sb.AppendLine("VisualStudioVersion = 17.0.31903.59");
            sb.AppendLine("MinimumVisualStudioVersion = 10.0.40219.1");
        }

        private static void WriteSolutionConfigurations(StringBuilder sb)
        {
            sb.AppendLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
            foreach (var config in Configurations)
            {
                foreach (var platform in Platforms)
                {
                    sb.AppendLine($"\t\t{config}|{platform} = {config}|{platform}");
                }
            }
            sb.AppendLine("\tEndGlobalSection");
        }

        private static void CollectFolderPaths(HashSet<string> allFolderPaths)
        {
            foreach (var projectInfo in ProjectInfos.Values)
            {
                if (!string.IsNullOrEmpty(projectInfo.FolderPath))
                {
                    var parts = projectInfo.FolderPath.Split('\\');
                    for (int i = 1; i <= parts.Length; i++)
                    {
                        allFolderPaths.Add(string.Join("\\", parts.Take(i)));
                    }
                }
            }
        }

        private (string, string)[] GetNMakeProperties(Target target, string config, string platform, string outDir)
        {
            var outputPath = GetTargetOutputPath(target);
            var buildCommand = $"cd \"{RootDirectory}\" && dotnet run SB build --target={target.Name} --mode={config.ToLower()}";

            return new[]
            {
                ("NMakeBuildCommandLine", buildCommand),
                ("NMakeReBuildCommandLine", $"cd \"{RootDirectory}\" && dotnet run SB clean --target={target.Name} && dotnet run SB build --target={target.Name} --mode={config.ToLower()}"),
                ("NMakeCleanCommandLine", $"cd \"{RootDirectory}\" && dotnet run SB clean --target={target.Name}"),
                ("NMakeOutput", outputPath),
                ("OutDir", outDir + "\\"),
                ("IntDir", $"temp\\{target.Name}\\{config}\\{platform}\\"),
                ("LocalDebuggerWorkingDirectory", outDir + "\\"),
                ("DebuggerFlavor", "WindowsLocalDebugger"),
                // 关键设置：让 VS 和 Rider 都知道每次都需要检查是否需要构建
                ("AlwaysCreate", "true"),
                // 针对 Rider 的特殊设置：强制每次都认为项目过期
                ("BuildDependsOn", "$(BuildDependsOn);AlwaysBuild"),
                ("DisableFastUpToDateCheck", "true")
            };
        }

        private XElement CreateProjectConfigurations(XNamespace ns)
        {
            var itemGroup = new XElement(ns + "ItemGroup",
                new XAttribute("Label", "ProjectConfigurations"));

            foreach (var config in Configurations)
            {
                foreach (var platform in Platforms)
                {
                    itemGroup.Add(new XElement(ns + "ProjectConfiguration",
                        new XAttribute("Include", $"{config}|{platform}"),
                        new XElement(ns + "Configuration", config),
                        new XElement(ns + "Platform", platform)));
                }
            }
            return itemGroup;
        }

        private string GetAbsolutePath(string path, string baseDir)
        {
            return Path.IsPathFullyQualified(path) ? path : Path.GetFullPath(Path.Combine(baseDir, path));
        }
    }
}