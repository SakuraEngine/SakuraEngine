using Microsoft.Extensions.FileSystemGlobbing;
using System.Text;
using System.Diagnostics;
using System.Collections;
using Serilog;

namespace SB.Core
{
    [Setup<VisualStudioSetup>]
    public partial class VisualStudio : IToolchain
    {
        // https://blog.pcitron.fr/2022/01/04/dont-use-vcvarsall-vsdevcmd/
        public bool FastFind => VSVersion == 2022;

        public VisualStudio(int VSVersion = 2022, Architecture? HostArch = null, Architecture? TargetArch = null)
        {
            this.VSVersion = VSVersion;
            this.HostArch = HostArch ?? HostInformation.HostArch;
            this.TargetArch = TargetArch ?? HostInformation.HostArch;
        }

        public static bool UseClangCl = true;
        public string Name => UseClangCl ? "clang-cl" : "msvc";
        public Version Version => new Version(VSVersion, 0);
        public ICompiler Compiler => UseClangCl ? ClangCLCC! : CLCC!;
        public ILinker Linker => LINK!;
        public IArchiver Archiver => LINK!;
        public string BuildTempPath => Directory.CreateDirectory(Path.Combine(SourceLocation.BuildTempPath, this.Version.ToString())).FullName;

        internal void FindVCVars()
        {
            Log.Information("VisualStudio version ... {VSVersion}", VSVersion);

            var fastMatcher = new Matcher();
            var slowMatcher = new Matcher();
            if (FastFind)
            {
                fastMatcher.AddIncludePatterns(new[] {
                    "*/Common7/Tools/vsdevcmd/ext/vcvars.bat",
                    "*/Common7/Tools/vsdevcmd/core/winsdk.bat"
                });
                slowMatcher.AddIncludePatterns(new[] {
                    "./**/Tools/vsdevcmd/ext/vcvars.bat",
                    "./**/Tools/vsdevcmd/core/winsdk.bat"
                });
            }
            else
            {
                fastMatcher.AddIncludePatterns(new[] {
                    "./Program Files/Microsoft Visual Studio/2022/*/VC/Auxiliary/Build/vcvarsall.bat",
                });
                slowMatcher.AddIncludePatterns(new[] {
                    "./**/VC/Auxiliary/Build/vcvarsall.bat"
                });
            }
            foreach (var Disk in Windows.EnumLogicalDrives())
            {
                bool FoundVS = false;
                var FindWithMatcher = (Matcher matcher) =>
                {
                    var VersionPostfix = (VSVersion == 2022) ? "/2022" : "";
                    var searchDirectory = $"{Disk}:/Program Files/Microsoft Visual Studio{VersionPostfix}";
                    IEnumerable<string> matchingFiles = matcher.GetResultsInFullPath(searchDirectory);
                    foreach (string file in matchingFiles)
                    {
                        var FileName = Path.GetFileName(file);
                        switch (FileName)
                        {
                            case "vcvarsall.bat":
                                FoundVS = true;
                                VCVarsAllBat = file; //file.Contains("Preview") ? file : VCVarsAllBat;
                                break;
                            case "vcvars.bat":
                                FoundVS = true;
                                VCVarsBat = file; //file.Contains("Preview") ? file : VCVarsBat;
                                break;
                            case "winsdk.bat":
                                FoundVS = true;
                                WindowsSDKBat = file; //file.Contains("Preview") ? file : WindowsSDKBat;
                                break;
                        }
                    }
                    return searchDirectory;
                };

                var SDKRoot = FindWithMatcher(fastMatcher);
                if (!FoundVS)
                {
                    Log.Verbose("Fast find failed, trying slow find in {SDKRoot}", SDKRoot);
                    FindWithMatcher(slowMatcher);
                }

                if (FoundVS)
                {
                    // "*/Visual Studio/2022/*/**"
                    var SomeBatPath = VCVarsAllBat ?? VCVarsBat;
                    var PayInfo = SomeBatPath!
                        .Replace("\\", "/")
                        .Replace(SDKRoot, "")
                        .Split('/')[1];
                    VSInstallDir = $"{SDKRoot}/{PayInfo}/";
                    break;
                }
            }
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

        static readonly Dictionary<Architecture, string> archStringMap = new Dictionary<Architecture, string> { { Architecture.X86, "x86" }, { Architecture.X64, "x64" }, { Architecture.ARM64, "arm64" } };
        
        private bool IsInDeveloperPrompt()
        {
            // 检查关键的 VS Developer Prompt 环境变量
            // 这些变量只有在真正的 Developer Prompt 中才会同时存在
            var devPromptArch = Environment.GetEnvironmentVariable("VSCMD_ARG_TGT_ARCH");
            var vsInstallDir = Environment.GetEnvironmentVariable("VSINSTALLDIR");
            var vcInstallDir = Environment.GetEnvironmentVariable("VCINSTALLDIR");
            var vscmdVer = Environment.GetEnvironmentVariable("VSCMD_VER");
            
            // 确保是真正的 Developer Prompt，而不仅仅是安装了 VS
            return !string.IsNullOrEmpty(devPromptArch) && 
                   !string.IsNullOrEmpty(vsInstallDir) && 
                   !string.IsNullOrEmpty(vcInstallDir) &&
                   !string.IsNullOrEmpty(vscmdVer);
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

        internal void RunVCVars()
        {
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
                // 不在 Developer Prompt 中，执行原有的初始化逻辑
                var oldEnvPath = Path.Combine(Path.GetTempPath(), $"vcvars_{VSVersion}_prev_{HostArch}_{TargetArch}.txt");
                var newEnvPath = Path.Combine(Path.GetTempPath(), $"vcvars_{VSVersion}_post_{HostArch}_{TargetArch}.txt");

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
                    cmd.StartInfo.Arguments = $"/c set > \"{oldEnvPath}\" && \"{VCVarsBat}\" && \"{WindowsSDKBat}\" && set > \"{newEnvPath}\"";
                }
                else
                {
                    string ArchString = (TargetArch == HostArch) ? archStringMap[TargetArch] : $"{archStringMap[HostArch]}_{archStringMap[TargetArch]}";
                    cmd.StartInfo.Arguments = $"/c set > \"{oldEnvPath}\" && \"{VCVarsAllBat}\" {ArchString} && set > \"{newEnvPath}\"";
                }
                cmd.Start();
                cmd.WaitForExit();

                var oldEnv = EnvReader.Load(oldEnvPath)!;
                VCEnvVariables = EnvReader.Load(newEnvPath)!;
                // Preprocess: cull old env variables
                foreach (var oldVar in oldEnv)
                {
                    if (VCEnvVariables.ContainsKey(oldVar.Key) && VCEnvVariables[oldVar.Key] == oldEnv[oldVar.Key])
                        VCEnvVariables.Remove(oldVar.Key);
                }
                // Preprocess: cull user env variables
                var vcPaths = VCEnvVariables["Path"]!.Split(';').ToHashSet();
                var oldPaths = oldEnv["Path"].Split(';').ToHashSet();
                vcPaths.ExceptWith(oldPaths);
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
                        if (Path.GetFileName(file) == "cl.exe")
                            CLCCPath = file;
                        if (Path.GetFileName(file) == "link.exe")
                            LINKPath = file;
                        if (Path.GetFileName(file) == "clang-cl.exe")
                            ClangCLPath = file;
                    }
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
            if (!string.IsNullOrEmpty(CLCCPath))            
                CLCC = new CLCompiler(CLCCPath!, VCEnvVariables);
            if (!string.IsNullOrEmpty(LINKPath))            
                LINK = new LINK(LINKPath!, VCEnvVariables);
            if (!string.IsNullOrEmpty(ClangCLPath))            
                ClangCLCC = new ClangCLCompiler(ClangCLPath!, VCEnvVariables);
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

        #region HelpersForTools
        public static bool IsValidRT(string what) => ValidRuntimeArguments.Contains(what);
        private static readonly string[] ValidRuntimeArguments = ["MT", "MTd", "MD", "MDd"];
        #endregion
    }

    public class VisualStudioSetup : ISetup
    {
        public void Setup()
        {
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