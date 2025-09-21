using System.Runtime.Versioning;
using System.Diagnostics;
using System.Collections;
using Serilog;
using System.Runtime.InteropServices;

namespace SB.Core
{
    public enum WindowsSDKStrategy
    {
        Default,
        FindLatest,
        UserSpecified
    };
    
    public partial class VisualStudio : IToolchain
    {
        public static bool UseClangCl = true;
        public static WindowsSDKStrategy WindowsSDKStrategy = WindowsSDKStrategy.Default;
    }

    [Setup<VisualStudioSetup>]
    public partial class VisualStudio : IToolchain
    {
        // https://blog.pcitron.fr/2022/01/04/dont-use-vcvarsall-vsdevcmd/
        public bool FastFind => VSVersion >= 2022;

        public VisualStudio(int VSVersion, Architecture? HostArch = null, Architecture? TargetArch = null)
        {
            this.VSVersion = VSVersion;
            this.HostArch = HostArch ?? HostInformation.HostArch;
            this.TargetArch = TargetArch ?? HostInformation.HostArch;
        }

        public string Name => UseClangCl ? "clang-cl" : "msvc";
        public Version Version => new Version(VSVersion, 0);
        public ICompiler Compiler => UseClangCl ? ClangCLCC! : CLCC!;
        public ILinker Linker => LINK!;
        public IArchiver Archiver => LINK!;
        public string BuildTempPath => Directory.CreateDirectory(Path.Combine(SourceLocation.BuildTempPath, this.Version.ToString())).FullName;

        internal void FindVCVars()
        {
            if (!OperatingSystem.IsWindows())
                return;

            Log.Information("VisualStudio version ... {VSVersion}", VSVersion);
            if (VSVersion == 2022)
            {
                var vsInstance = SearchVS2022.FindBestInstance();
                
                if (vsInstance != null && vsInstance.IsValid)
                {
                    // 重要：VS 批处理文件期望 VSINSTALLDIR 以斜杠结尾
                    VSInstallDir = vsInstance.InstallPath;
                    if (!VSInstallDir!.EndsWith("/") && !VSInstallDir.EndsWith("\\"))
                    {
                        VSInstallDir = VSInstallDir.Replace("\\", "/") + "/";
                    }
                    else
                    {
                        VSInstallDir = VSInstallDir.Replace("\\", "/");
                    }
                    
                    VCVarsAllBat = vsInstance.VCVarsAllBat;
                    VCVarsBat = vsInstance.VCVarsBat;
                    WindowsSDKBat = vsInstance.WindowsSDKBat;
                    
                    Log.Verbose("Found VS2022 at: {InstallDir}", VSInstallDir);
                    if (FastFind)
                    {
                        Log.Verbose("Found VCVarsBat: {VCVarsBat}", VCVarsBat);
                        Log.Verbose("Found WindowsSDKBat: {WindowsSDKBat}", WindowsSDKBat);
                    }
                    else
                    {
                        Log.Verbose("Found VCVarsAllBat: {VCVarsAllBat}", VCVarsAllBat);
                    }
                }
                else
                {
                    Log.Error("Visual Studio 2022 not found");
                }
            }
            else
            {
                Log.Error("VS Version not supported!");
            }
        }

        static readonly Dictionary<Architecture, string> archStringMap = new Dictionary<Architecture, string> { { Architecture.X86, "x86" }, { Architecture.X64, "x64" }, { Architecture.ARM64, "arm64" } };

        private bool IsInVisualStudio()
        {
            var vsEdition = Environment.GetEnvironmentVariable("VisualStudioEdition");
            bool IsVS = !string.IsNullOrEmpty(vsEdition);
            return IsVS;
        }

        private bool IsInDeveloperPrompt()
        {
            // 检查关键的 VS Developer Prompt 环境变量
            // 这些变量只有在真正的 Developer Prompt 中才会同时存在
            var devPromptArch = Environment.GetEnvironmentVariable("VSCMD_ARG_TGT_ARCH");
            var vsInstallDir = Environment.GetEnvironmentVariable("VSINSTALLDIR");
            var vcInstallDir = Environment.GetEnvironmentVariable("VCINSTALLDIR");
            var vscmdVer = Environment.GetEnvironmentVariable("VSCMD_VER");

            // 确保是真正的 Developer Prompt，而不仅仅是安装了 VS
            bool IsPrompt = !string.IsNullOrEmpty(devPromptArch) &&
                   !string.IsNullOrEmpty(vsInstallDir) &&
                   !string.IsNullOrEmpty(vcInstallDir) &&
                   !string.IsNullOrEmpty(vscmdVer);
            return IsPrompt;
        }

        private void FindToolsInCurrentEnvironment()
        {
            var pathEnv = Environment.GetEnvironmentVariable("Path") ?? "";
            var paths = pathEnv.Split(';', StringSplitOptions.RemoveEmptyEntries);

            foreach (var path in paths)
            {
                if (!Directory.Exists(path))
                    continue;

                try
                {
                    foreach (var file in Directory.EnumerateFiles(path))
                    {
                        var fileName = Path.GetFileName(file);
                        switch (fileName.ToLowerInvariant())
                        {
                            case "cl.exe":
                                CLCCPath = file;
                                break;
                            case "link.exe":
                                LINKPath = file;
                                break;
                            case "clang-cl.exe":
                                ClangCLPath = file;
                                break;
                        }
                    }
                }
                catch
                {
                    // 忽略无法访问的目录
                }
            }
        }

        internal void RunBat(string oldEnvPath, string newEnvPath)
        {
            Process cmd = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    RedirectStandardInput = false,
                    RedirectStandardOutput = false,
                    RedirectStandardError = false,
                    CreateNoWindow = true,
                    UseShellExecute = false
                }
            };
            if (FastFind)
            {
                cmd.StartInfo.Environment.Add("VSCMD_ARG_HOST_ARCH", archStringMap[HostArch]);
                cmd.StartInfo.Environment.Add("VSCMD_ARG_TGT_ARCH", archStringMap[TargetArch]);
                cmd.StartInfo.Environment.Add("VSCMD_ARG_APP_PLAT", "Desktop");
                cmd.StartInfo.Environment.Add("VSINSTALLDIR", VSInstallDir!.Replace("/", "\\"));
                
                // 强制使用最新的工具链版本
                var latestToolsetVersion = FindLatestToolsetVersion();
                if (!string.IsNullOrEmpty(latestToolsetVersion))
                {
                    cmd.StartInfo.Environment.Add("VSCMD_ARG_VCVARS_VER", latestToolsetVersion);
                    Log.Information("Forcing toolset version to latest: {Version}", latestToolsetVersion);
                }
                
                cmd.StartInfo.Arguments = $"/c set > \"{oldEnvPath}\" && \"{VCVarsBat}\" && \"{WindowsSDKBat}\" && set > \"{newEnvPath}\"";
            }
            else
            {
                string ArchString = (TargetArch == HostArch) ? archStringMap[TargetArch] : $"{archStringMap[HostArch]}_{archStringMap[TargetArch]}";
                cmd.StartInfo.Arguments = $"/c set > \"{oldEnvPath}\" && \"{VCVarsAllBat}\" {ArchString} && set > \"{newEnvPath}\"";
            }
            cmd.Start();
            cmd.WaitForExit();
        }

        internal void RunVCVars()
        {
            var oldEnvPath = Path.Combine(Path.GetTempPath(), $"vcvars_{VSVersion}_prev_{HostArch}_{TargetArch}.txt");
            var newEnvPath = Path.Combine(Path.GetTempPath(), $"vcvars_{VSVersion}_post_{HostArch}_{TargetArch}.txt");

            // 检测是否已在 Developer Prompt 中
            if (IsInDeveloperPrompt())
            {
                Log.Information("Already in Visual Studio Developer Command Prompt, using existing environment");

                // 直接使用当前环境变量
                VCEnvVariables = new Dictionary<string, string?>();
                foreach (DictionaryEntry entry in Environment.GetEnvironmentVariables())
                {
                    VCEnvVariables[entry.Key.ToString()!] = entry.Value?.ToString();
                }

                // 从当前环境中查找工具
                FindToolsInCurrentEnvironment();

                // 验证是否找到必要的工具
                if (string.IsNullOrEmpty(CLCCPath) || !File.Exists(CLCCPath))
                {
                    Log.Warning("cl.exe not found in Developer Prompt environment");
                }
                if (string.IsNullOrEmpty(LINKPath) || !File.Exists(LINKPath))
                {
                    Log.Warning("link.exe not found in Developer Prompt environment");
                }
            }
            else
            {
                // VS 下配了一部分的环境变量，所以不能乱剔除
                bool isInVisualStudio = IsInVisualStudio();

                // 不在 Developer Prompt 中，执行原有的初始化逻辑
                RunBat(oldEnvPath, newEnvPath);

                var oldEnv = EnvReader.Load(oldEnvPath)!;
                VCEnvVariables = EnvReader.Load(newEnvPath)!;
                // Preprocess: cull old env variables
                if (!isInVisualStudio)
                {
                    foreach (var oldVar in oldEnv)
                    {
                        if (VCEnvVariables.ContainsKey(oldVar.Key) && VCEnvVariables[oldVar.Key] == oldEnv[oldVar.Key])
                            VCEnvVariables.Remove(oldVar.Key);
                    }
                }
                // Preprocess: cull user env variables
                var vcPaths = VCEnvVariables["Path"]!.Split(';').ToHashSet();
                var oldPaths = oldEnv["Path"].Split(';').ToHashSet();
                if (!isInVisualStudio)
                {
                    vcPaths.ExceptWith(oldPaths);
                }
                VCEnvVariables["Path"] = string.Join(";", vcPaths);
                // Preprocess: calculate include dir
                var OriginalIncludes = VCEnvVariables.TryGetValue("INCLUDE", out var V0) ? V0 : "";
                var VCVarsIncludes = VCEnvVariables.TryGetValue("__VSCMD_VCVARS_INCLUDE", out var V1) ? V1 : "";
                var WindowsSDKIncludes = VCEnvVariables.TryGetValue("__VSCMD_WINSDK_INCLUDE", out var V2) ? V2 : "";
                var NetFXIncludes = VCEnvVariables.TryGetValue("__VSCMD_NETFX_INCLUDE", out var V3) ? V3 : "";
                VCEnvVariables["INCLUDE"] = VCVarsIncludes + WindowsSDKIncludes + NetFXIncludes + OriginalIncludes;
                // Enum all files and pick usable tools
                foreach (var path in vcPaths)
                {
                    if (!Directory.Exists(path))
                        continue;

                    foreach (var file in Directory.EnumerateFiles(path))
                    {
                        if (Path.GetFileName(file) == "cl.exe" && file.Contains("MSVC"))
                            CLCCPath = file;
                        if (Path.GetFileName(file) == "link.exe" && file.Contains("MSVC"))
                            LINKPath = file;
                        if (Path.GetFileName(file) == "clang-cl.exe")
                            ClangCLPath = file;
                    }
                }

                // 如果在标准路径中没有找到link.exe，尝试更广泛的搜索
                if (string.IsNullOrEmpty(LINKPath))
                {
                    LINKPath = FindLinkerInAlternativePaths(vcPaths);
                }
                // clang-cl may be installed in a different user path
                if (!File.Exists(ClangCLPath))
                {
                    foreach (var path in oldPaths)
                    {
                        if (!Directory.Exists(path))
                            continue;

                        foreach (var file in Directory.EnumerateFiles(path))
                        {
                            if (Path.GetFileName(file) == "clang-cl.exe")
                                ClangCLPath = file;
                        }
                    }
                }
            }

            var WindowsSDKVersion = VCEnvVariables["WindowsSDKVersion"]!.Replace('\\', ' ');
            var WindowsSDKLibVersion = VCEnvVariables["WindowsSDKLibVersion"]!.Replace('\\', ' ');
            var UCRTVersion = VCEnvVariables["UCRTVersion"]!.Replace('\\', ' ');
            Log.Information("WindowsSDKVersion version ... {WindowsSDKVersion}", WindowsSDKVersion);
            Log.Verbose("WindowsSDKLibVersion version ... {WindowsSDKLibVersion}", WindowsSDKLibVersion);
            Log.Verbose("UCRTVersion version ... {UCRTVersion}", UCRTVersion);

            if (!string.IsNullOrEmpty(CLCCPath))
                CLCC = new CLCompiler(CLCCPath!, VCEnvVariables);
            if (!string.IsNullOrEmpty(LINKPath))
                LINK = new LINK(LINKPath!, VCEnvVariables);
            if (!string.IsNullOrEmpty(ClangCLPath))
                ClangCLCC = new ClangCLCompiler(ClangCLPath!, VCEnvVariables);

            if (CLCC is null && !UseClangCl)
                Log.Fatal("CL.exe tool not found, please ensure Visual Studio is installed correctly.");
            if (ClangCLCC is null && UseClangCl)
                Log.Fatal("ClangCLCC tool not found, please ensure Clang is installed correctly.");
            if (LINK is null)
                Log.Fatal("LINK tool not found, please ensure Visual Studio is installed correctly.");
        }

        public readonly int VSVersion;
        public readonly Architecture HostArch;
        public readonly Architecture TargetArch;

        public string? VSInstallDir { get; private set; }
        public string? VCVarsAllBat { get; private set; }
        public string? VCVarsBat { get; private set; }
        public string? WindowsSDKBat { get; private set; }

        public Dictionary<string, string?>? VCEnvVariables { get; private set; }
        public CLCompiler? CLCC { get; private set; }
        public ClangCLCompiler? ClangCLCC { get; private set; }
        public LINK? LINK { get; private set; }
        public string? CLCCPath { get; private set; }
        public string? ClangCLPath { get; private set; }
        public string? LINKPath { get; private set; }

        /// <summary>
        /// 在备用路径中查找链接器
        /// </summary>
        private string FindLinkerInAlternativePaths(IEnumerable<string> vcPaths)
        {
            var searchPaths = new List<string>();
            
            // 添加VC路径（移除MSVC限制，允许在任何VC路径中查找）
            foreach (var path in vcPaths)
            {
                if (Directory.Exists(path))
                {
                    searchPaths.Add(path);
                }
            }
            
            // 添加常见的Visual Studio工具链路径
            var commonToolchainPaths = new[]
            {
                @"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC",
                @"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC",
                @"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC",
                @"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC",
                @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC",
                @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC",
                @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Tools\MSVC",
                @"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC"
            };
            
            foreach (var basePath in commonToolchainPaths)
            {
                if (Directory.Exists(basePath))
                {
                    try
                    {
                        var versionDirs = Directory.GetDirectories(basePath);
                        foreach (var versionDir in versionDirs)
                        {
                            searchPaths.Add(Path.Combine(versionDir, "bin", "Hostx64", "x64"));
                            searchPaths.Add(Path.Combine(versionDir, "bin", "Hostx86", "x86"));
                        }
                    }
                    catch
                    {
                        // 忽略访问异常
                    }
                }
            }
            
            // 查找link.exe
            foreach (var searchPath in searchPaths.Where(Directory.Exists).Distinct())
            {
                var linkPath = Path.Combine(searchPath, "link.exe");
                if (File.Exists(linkPath))
                {
                    Log.Information("Found LINK.exe at alternative path: {LinkPath}", linkPath);
                    return linkPath;
                }
            }
            
            return "";
        }


        #region HelpersForTools
        public static bool IsValidRT(string what) => ValidRuntimeArguments.Contains(what);
        private static readonly string[] ValidRuntimeArguments = ["MT", "MTd", "MD", "MDd"];
        #endregion

        #region ToolsetVersion
        /// <summary>
        /// 查找最新的工具链版本
        /// </summary>
        private string FindLatestToolsetVersion()
        {
            try
            {
                if (string.IsNullOrEmpty(VSInstallDir))
                    return "";

                var toolsDir = Path.Combine(VSInstallDir, "VC", "Tools", "MSVC");
                if (!Directory.Exists(toolsDir))
                    return "";

                // 查找所有工具链版本目录
                var versions = new List<Version>();
                foreach (var dir in Directory.GetDirectories(toolsDir))
                {
                    var dirName = Path.GetFileName(dir);
                    if (Version.TryParse(dirName, out var version))
                    {
                        // 检查是否包含必要的工具文件
                        var clExePath = Path.Combine(dir, "bin", "Hostx64", "x64", "cl.exe");
                        if (File.Exists(clExePath))
                        {
                            versions.Add(version);
                        }
                    }
                }

                if (versions.Count == 0)
                    return "";

                // 返回最新版本
                var latestVersion = versions.OrderByDescending(v => v).First();
                return latestVersion.ToString();
            }
            catch (Exception ex)
            {
                Log.Verbose("Failed to find latest toolset version: {Message}", ex.Message);
                return "";
            }
        }
        #endregion
    }

    public class VisualStudioSetup : ISetup
    {
        public void Setup()
        {
            if (!OperatingSystem.IsWindows())
                return;
                
            if (BuildSystem.TargetOS == OSPlatform.Windows && BuildSystem.HostOS == OSPlatform.Windows)
            {
                using (Profiler.BeginZone("InitializeVisualStudio", color: (uint)Profiler.ColorType.WebMaroon))
                {
                    Stopwatch sw = Stopwatch.StartNew();

                    VisualStudio.FindVCVars();
                    sw.Stop();
                    Log.Information("Find VCVars took {ElapsedMilliseconds}s", sw.ElapsedMilliseconds / 1000.0f);
                    sw.Restart();
                    VisualStudio.RunVCVars();
                    sw.Stop();
                    Log.Information("Run VCVars took {ElapsedMilliseconds}s", sw.ElapsedMilliseconds / 1000.0f);
                }
            }
        }

        public static VisualStudio VisualStudio { get; set; } = new VisualStudio(2022);
    }
}
