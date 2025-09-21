using Serilog;
using Serilog.Events;
using Microsoft.Extensions.FileSystemGlobbing;
using Microsoft.Extensions.FileSystemGlobbing.Abstractions;

namespace SB;

public class UploadSDKCommand
{
    [Cli.Option(Name = "verbose", ShortName = 'v', Help = "Enable verbose logging", IsRequired = false)]
    public bool Verbose { get; set; } = false;
    [Cli.Option(Name = "glob", ShortName = 'g', Help = "GLOB pattern to match files for upload", IsRequired = true)]
    public string GlobPattern { get; set; } = "";

    [Cli.Option(Name = "source", ShortName = 's', Help = "FTP source name to upload to", IsRequired = false)]
    public string Source { get; set; } = "d5_ftp";

    [Cli.Option(Name = "directory", ShortName = 'd', Help = "Directory to search for files", IsRequired = false)]
    public string SearchDirectory { get; set; } = ".";

    [Cli.ExecCmd]
    public int Exec()
    {
        Engine.InitializeLogger(Verbose ? LogEventLevel.Verbose : LogEventLevel.Information);

        // notify prepare commandline stage for some basic setup
        BuildStage.UpdateStage(EBuildStage.PrepareCommandline);

        Log.Information("开始上传SDK文件...");
        Log.Information("GLOB模式: {GlobPattern}", GlobPattern);
        Log.Information("搜索目录: {Directory}", Path.GetFullPath(SearchDirectory));
        Log.Information("目标源: {Source}", Source);

        try
        {
            // 搜索匹配的文件
            var searchDirectory = Path.GetFullPath(SearchDirectory);
            if (!Directory.Exists(searchDirectory))
            {
                Log.Error("目录不存在: {Directory}", searchDirectory);
                return 1;
            }

            var matchingFiles = GlobFileSearch(searchDirectory, GlobPattern);

            if (matchingFiles.Count == 0)
            {
                Log.Warning("没有找到匹配GLOB模式的文件: {GlobPattern}", GlobPattern);
                return 0;
            }

            Log.Information("找到 {Count} 个匹配的文件:", matchingFiles.Count);
            foreach (var file in matchingFiles)
            {
                Log.Information("  - {FileName}", Path.GetFileName(file));
            }

            // 执行上传
            Log.Information("开始上传文件到FTP...");
            Download.UploadSDKAndUpdateManifest(Source, matchingFiles.ToArray()).Wait();

            Log.Information("SDK上传完成！");
            return 0;
        }
        catch (Exception ex)
        {
            Log.Error(ex, "上传SDK时发生错误: {Message}", ex.Message);
            return 1;
        }
    }

    private List<string> GlobFileSearch(string directory, string pattern)
    {
        var files = new List<string>();

        try
        {
            // 使用 Microsoft.Extensions.FileSystemGlobbing 进行GLOB模式匹配
            var matcher = new Matcher();
            matcher.AddInclude(pattern);

            var directoryInfo = new DirectoryInfo(directory);
            var directoryInfoWrapper = new DirectoryInfoWrapper(directoryInfo);
            var result = matcher.Execute(directoryInfoWrapper);

            foreach (var file in result.Files)
            {
                var fullPath = Path.Combine(directory, file.Path);
                files.Add(fullPath);
            }
        }
        catch (Exception ex)
        {
            Log.Warning(ex, "搜索文件时发生错误: {Message}", ex.Message);
        }

        return files;
    }

    [Cli.RegisterCmd(Name = "upload-sdk", Help = "Upload SDK files to FTP using GLOB pattern", Usage = "SB upload-sdk --glob <pattern> [options]")]
    public static object RegisterCommand() => new UploadSDKCommand();
}
