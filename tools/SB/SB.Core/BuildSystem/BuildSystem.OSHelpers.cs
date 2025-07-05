using System.Text;
using System.Security.Cryptography;
using System.Diagnostics;
using SB.Core;

namespace SB
{
    using BS = BuildSystem;
    public partial class BuildSystem
    {
        public static string GetUniqueTempFileName(string File, string Hint, string Extension, IEnumerable<string>? Args = null)
        {
            string FullIdentifier = File + (Args is null ? "" : String.Join("", Args));
            var SHA = SHA256.HashData(Encoding.UTF8.GetBytes(FullIdentifier));
            return $"{Hint}.{Path.GetFileName(File)}.{Convert.ToHexString(SHA)}.{Extension}";
        }

        public static bool CheckPath(string P, bool MustExist) => Path.IsPathFullyQualified(P) && (!MustExist || Directory.Exists(P));
        public static bool CheckFile(string P, bool MustExist) => Path.IsPathFullyQualified(P) && (!MustExist || File.Exists(P));

        public static int RunProcess(string ExecutablePath, string Arguments, out string Output, out string Error, Dictionary<string, string?>? Env = null, string? WorkingDirectory = null)
        {
            using (Profiler.BeginZone($"RunProcess", color: (uint)Profiler.ColorType.Yellow1))
            {
                if (!OperatingSystem.IsWindows() && File.Exists(ExecutablePath))
                {
                    var Mode = File.GetUnixFileMode(ExecutablePath);
                    if (!Mode.HasFlag(UnixFileMode.UserExecute))
                        File.SetUnixFileMode(ExecutablePath, UnixFileMode.UserExecute);
                }

                try
                {
                    Process P = new Process
                    {
                        StartInfo = new ProcessStartInfo
                        {
                            FileName = ExecutablePath,
                            RedirectStandardInput = false,
                            RedirectStandardOutput = true,
                            RedirectStandardError = true,
                            CreateNoWindow = false,
                            UseShellExecute = false,
                            Arguments = Arguments,
                            WorkingDirectory = Directory.GetParent(ExecutablePath)!.FullName
                        }
                    };
                    if (WorkingDirectory is not null)
                        P.StartInfo.WorkingDirectory = WorkingDirectory;
                    if (Env is not null)
                    {
                        foreach (var kvp in Env)
                        {
                            P.StartInfo.Environment.Add(kvp.Key, kvp.Value);
                        }
                    }
                    string localOutput = string.Empty;
                    string localError = string.Empty;
                    P.OutputDataReceived += (sender, e) => { if (e.Data is not null) localOutput += e.Data + "\n"; };
                    P.ErrorDataReceived += (sender, e) => { if (e.Data is not null) localError += e.Data + "\n"; };
                    P.Start();
                    P.BeginOutputReadLine();
                    P.BeginErrorReadLine();
                    P.WaitForExit();
                    Output = localOutput;
                    Error = localError;
                    return P.ExitCode;
                }
                catch (Exception e)
                {
                    throw new TaskFatalError($"Failed to run process {ExecutablePath} with arguments {Arguments}", e.Message);
                }
            }
        }

        public static string DepsStore = ".deps";
        public static string ObjsStore = ".objs";
        public static string GeneratedSourceStore = ".gens";
        public static string TempPath { get; set; } = Directory.CreateDirectory(Path.Join(Directory.GetCurrentDirectory(), ".sb")).FullName;
        public static string BuildPath { get; set; } = TempPath!;
        public static string PackageBuildPath { get; set; } = TempPath!;
    }

    public static class BuildPathExtensions
    {
        public static string GetStorePath(this Target Target, string StoreName) => Directory.CreateDirectory(Path.Combine(Target.GetBuildPath(), StoreName, $"{BS.TargetOS}-{BS.TargetArch}-{BS.GlobalConfiguration}", Target.Name)).FullName;
        public static string GetBinaryPath(this Target Target) => Directory.CreateDirectory(Path.Combine(Target.GetBuildPath(), $"{BS.TargetOS}-{BS.TargetArch}-{BS.GlobalConfiguration}")).FullName;
        public static string GetBuildPath(this Target Target) => Target.IsFromPackage ? BS.PackageBuildPath : BS.BuildPath;
    }

    public static class StringExtensions
    {
        public static string ToUpperSnakeCase(this string text)
        {
            if(text == null) {
                throw new ArgumentNullException(nameof(text));
            }
            if(text.Length < 2) {
                return text.ToUpperInvariant();
            }
            var sb = new StringBuilder();
            sb.Append(char.ToUpperInvariant(text[0]));
            for(int i = 1; i < text.Length; ++i) {
                char c = text[i];
                if(char.IsUpper(c)) {
                    sb.Append('_');
                    sb.Append(char.ToUpperInvariant(c));
                } else {
                    sb.Append(char.ToUpperInvariant(c));
                }
            }
            return sb.ToString();
        }

        public static bool Is_C_Cpp(this string p) => p.EndsWith(".c") || p.EndsWith(".cpp") || p.EndsWith(".cc") || p.EndsWith(".cxx");
        public static bool Is_OC_OCpp(this string p) => p.EndsWith(".m") || p.EndsWith(".mm") || p.EndsWith(".mpp");
    }
}