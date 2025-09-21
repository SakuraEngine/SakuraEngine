using Serilog;
using System.Collections.Concurrent;
using System.Net;
using System.Security.Cryptography;
using FluentFTP;

namespace SB
{
    public static class Download
    {
        public const int StageOrder = -1000;
        [AfterStage(EBuildStage.PrepareCommandline, StageOrder)]
        static void _AfterSetupPaths()
        {
            Directory.CreateDirectory(BuildDirs.DownloadDir);

            // add default source
            if (Sources.Count == 0)
            {
                var githubSource = "https://github.com/SakuraEngine/Sakura.Resources/releases/download/SDKs/";
                Log.Information("Download source: {githubSource}", githubSource);
                AddSource("skr_github", githubSource);
            }
        }

        public static void AddSource(string Name, string URL, string? Username = null, string? Password = null)
        {
            Sources.Add(Name, new DownloadSource
            {
                Name = Name,
                URL = URL,
                Destination = BuildDirs.DownloadDir,
                ManifestPath = Path.Combine(BuildDirs.DownloadDir, "manifests", $"{Name}.json"),
                Username = Username,
                Password = Password
            });
        }

        public static void FetchManifests()
        {
            lock (FetchLock)
            {
                if (!FetchOnceFlag)
                {
                    FetchOnceFlag = true;
                    Directory.CreateDirectory(Path.Combine(BuildDirs.DownloadDir, "manifests"));
                    foreach (var Source in Sources.Values)
                    {
                        var URL = Source.URL + "manifest.json";
                        try
                        {
                            Log.Information("fetching manifest ... from {URL} to {SourceManifestPath}", URL, Source.ManifestPath);

                            byte[] bytes;
                            if (IsFtpUrl(URL))
                            {
                                bytes = DownloadFromFtpAsync(URL, Source.Username, Source.Password).Result;
                            }
                            else
                            {
                                using (var Http = CreateHttpClient(Source))
                                {
                                    Http.Timeout = TimeSpan.FromMinutes(30);
                                    var Bytes = Http.GetByteArrayAsync(URL);
                                    Bytes.Wait();
                                    bytes = Bytes.Result;
                                }
                            }

                            Source.ManifestString = bytes;
                            // write to disk, just for debugging
                            File.WriteAllBytes(Source.ManifestPath, Source.ManifestString);
                        }
                        catch (Exception e)
                        {
                            Log.Error(e, "Failed to fetch manifest from {URL}, message: {eMessage}", URL, e.Message);
                        }
                    }
                    LoadManifests();
                }
            }
        }

        private static void LoadManifests()
        {
            foreach (var Source in Sources.Values)
            {
                var JSONReader = new System.Text.Json.Utf8JsonReader(Source.ManifestString);
                JSONReader.Read(); // ROOT
                while (JSONReader.Read())
                {
                    if (JSONReader.TokenType == System.Text.Json.JsonTokenType.PropertyName)
                    {
                        var NAME = JSONReader.GetString()!;
                        JSONReader.Read();
                        var SHA = JSONReader.GetString()!;
                        // add result to source list
                        List<DownloadSource> SourcesList;
                        if (!FileSources.TryGetValue(NAME, out SourcesList!))
                        {
                            SourcesList = new List<DownloadSource>();
                            FileSources.Add(NAME, SourcesList);
                        }
                        SourcesList.Add(Source);
                        // add result to sha checker
                        HashSet<string> SHAs;
                        if (!FileSHAs.TryGetValue(NAME, out SHAs!))
                        {
                            SHAs = new HashSet<string>();
                            FileSHAs.Add(NAME, SHAs);
                        }
                        SHAs.Add(SHA);
                    }
                }
            }
            // check sha conflict
            FileSHAs.Where(static KVP => KVP.Value.Count > 1).ToList().ForEach(static KVP =>
            {
                throw new Exception($"{KVP.Key} SHA conflict detected!");
            });
            // print manifest info
            Log.Information("----------------manifest info----------------");
            foreach (var Source in Sources.Values)
            {
                Log.Information("{SourceName} ... {SourceURL}", Source.Name, Source.URL);
            }
            Log.Information("----------------manifest info----------------");
        }

        public static async Task UploadSDKAndUpdateManifest(string Source, params string[] SDKFiles)
        {
            // 验证源是否存在且为FTP源
            if (!Sources.TryGetValue(Source, out var sourceInfo))
            {
                throw new ArgumentException($"Source '{Source}' not found");
            }

            if (!IsFtpUrl(sourceInfo.URL))
            {
                throw new NotSupportedException($"Source '{Source}' is not an FTP source. Only FTP sources are supported.");
            }

            Log.Information("开始上传SDK并更新manifest，源: {Source}", Source);

            try
            {
                // 1. 直接从FTP拉取现有的manifest.json到内存
                Dictionary<string, string> manifest;

                try
                {
                    var manifestUrl = sourceInfo.URL + "manifest.json";
                    Log.Information("从FTP拉取manifest: {ManifestUrl}", manifestUrl);

                    var manifestBytes = await DownloadFromFtpAsync(manifestUrl, sourceInfo.Username, sourceInfo.Password);
                    var manifestJson = System.Text.Encoding.UTF8.GetString(manifestBytes);
                    manifest = System.Text.Json.JsonSerializer.Deserialize<Dictionary<string, string>>(manifestJson)
                               ?? new Dictionary<string, string>();

                    Log.Information("成功拉取manifest，包含 {Count} 个文件", manifest.Count);
                }
                catch (Exception ex)
                {
                    Log.Warning(ex, "无法从FTP拉取manifest，将创建新的manifest: {Message}", ex.Message);
                    manifest = new Dictionary<string, string>();
                }

                // 2. 并发计算传入文件的SHA256并检查是否需要上传
                var filesToUpload = new ConcurrentBag<string>();
                var updatedManifest = new ConcurrentDictionary<string, string>(manifest);

                await Parallel.ForEachAsync(SDKFiles, async (filePath, cancellationToken) =>
                {
                    if (!File.Exists(filePath))
                    {
                        Log.Warning("文件不存在，跳过: {FilePath}", filePath);
                        return;
                    }

                    var fileName = Path.GetFileName(filePath);
                    var fileBytes = await File.ReadAllBytesAsync(filePath, cancellationToken);
                    var sha256 = Convert.ToHexString(SHA256.HashData(fileBytes)).ToLowerInvariant();

                    // 检查manifest中是否已存在相同文件名和SHA256的文件
                    if (updatedManifest.TryGetValue(fileName, out var existingSha) && existingSha == sha256)
                    {
                        Log.Information("文件已存在且SHA256匹配，跳过上传: {FileName} ({SHA256})", fileName, sha256);
                    }
                    else
                    {
                        updatedManifest[fileName] = sha256;
                        filesToUpload.Add(filePath);
                        Log.Information("文件需要上传: {FileName} -> {SHA256}", fileName, sha256);
                    }
                });

                // 更新原始manifest
                manifest.Clear();
                foreach (var kvp in updatedManifest)
                {
                    manifest[kvp.Key] = kvp.Value;
                }

                // 3. 上传需要上传的文件到FTP
                if (filesToUpload.Count > 0)
                {
                    Log.Information("开始上传 {Count} 个文件到FTP", filesToUpload.Count);
                    Parallel.ForEach(filesToUpload, (filePath) =>
                    {
                        var fileName = Path.GetFileName(filePath);
                        UploadFileToFtpAsync(sourceInfo, fileName, filePath).Wait();
                    });
                }
                else
                {
                    Log.Information("所有文件都已存在且SHA256匹配，无需上传");
                }

                // 4. 序列化更新后的manifest并直接上传到FTP
                var updatedManifestJson = System.Text.Json.JsonSerializer.Serialize(manifest, new System.Text.Json.JsonSerializerOptions
                {
                    WriteIndented = true
                });

                // 将manifest内容写入临时内存流并上传
                using var manifestStream = new MemoryStream(System.Text.Encoding.UTF8.GetBytes(updatedManifestJson));
                await UploadStreamToFtpAsync(sourceInfo, "manifest.json", manifestStream);

                Log.Information("SDK上传和manifest更新完成");
            }
            catch (Exception ex)
            {
                Log.Error(ex, "上传SDK和更新manifest失败: {Message}", ex.Message);
                throw;
            }
        }

        private static async Task DownloadFromSource(DownloadSource Source, string FileName)
        {
            var Destination = Path.Combine(BuildDirs.DownloadDir, FileName);
            var URL = Source.URL + FileName;
            Log.Information("downloading ... from {URL} to {Destination}", URL, Destination);
            byte[] bytes;
            if (IsFtpUrl(URL))
            {
                bytes = await DownloadFromFtpAsync(URL, Source.Username, Source.Password);
            }
            else
            {
                using (var Http = CreateHttpClient(Source))
                {
                    Http.Timeout = TimeSpan.FromMinutes(30);
                    var Bytes = Http.GetByteArrayAsync(URL);
                    await Bytes;
                    bytes = Bytes.Result;
                }
            }
            await File.WriteAllBytesAsync(Destination, bytes);
        }

        public static async Task<string> DownloadFile(string FileName, bool Force = false)
        {
            using (Profiler.BeginZone($"FetchManifests", color: (uint)Profiler.ColorType.Pink1))
            {
                Download.FetchManifests();
            }

            // Already exist on disk
            var FilePath = Path.Combine(BuildDirs.DownloadDir, FileName);
            if (!Force && File.Exists(FilePath))
            {
                var ExistedSHA = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(Path.Combine(BuildDirs.DownloadDir, FileName)))).ToUpperInvariant();
                var ManifestSHA = FileSHAs[FileName].First().ToUpperInvariant();
                if (ExistedSHA == ManifestSHA)
                {
                    Log.Information("downloading ... restore existed {FileName}", FileName);
                    return FilePath;
                }
            }

            // Download from source
            Task? DownloadTask = null;
            lock (DownloadingLocks.GetOrAdd(FileName, (Name) => new System.Threading.Lock()))
            {
                if (!DownloadingTasks.TryGetValue(FileName, out DownloadTask))
                {
                    DownloadTask = DownloadFromSource(Sources.Values.First(), FileName);
                    DownloadingTasks.TryAdd(FileName, DownloadTask);
                }
            }
            await DownloadTask!;
            return FilePath;
        }

        private static bool FetchOnceFlag = false;
        private static readonly object FetchLock = new object();
        public static string HttpProxy = "";
        public static Lazy<IWebProxy> HttpProxyObject = new Lazy<IWebProxy>(() =>
        {
            if (string.IsNullOrEmpty(HttpProxy))
                return HttpClient.DefaultProxy;
            return new WebProxy(HttpProxy);
        });

        private static HttpClient CreateHttpClient(DownloadSource? source = null)
        {
            var handler = new HttpClientHandler { Proxy = HttpProxyObject.Value };
            var username = source?.Username;
            var password = source?.Password;

            if (!string.IsNullOrEmpty(username))
            {
                handler.Credentials = new NetworkCredential(username, password);
            }

            var httpClient = new HttpClient(handler);

            if (!string.IsNullOrEmpty(username))
            {
                var credentials = Convert.ToBase64String(
                    System.Text.Encoding.ASCII.GetBytes($"{username}:{password}"));
                httpClient.DefaultRequestHeaders.Authorization =
                    new System.Net.Http.Headers.AuthenticationHeaderValue("Basic", credentials);
            }

            return httpClient;
        }

        private static bool IsFtpUrl(string url)
        {
            return url.StartsWith("ftp://", StringComparison.OrdinalIgnoreCase);
        }

        private static async Task<byte[]> DownloadFromFtpAsync(string url, string? username, string? password)
        {
            try
            {
                var uri = new Uri(url);
                using var ftpClient = new AsyncFtpClient(uri.Host, username, password);

                await ftpClient.Connect();

                using var stream = new MemoryStream();
                await ftpClient.DownloadStream(stream, uri.AbsolutePath);

                return stream.ToArray();
            }
            catch (Exception ex)
            {
                Log.Error(ex, "FTP download failed from {URL}: {Message}", url, ex.Message);
                throw;
            }
        }

        private static async Task UploadFileToFtpAsync(DownloadSource source, string fileName, string localFilePath)
        {
            try
            {
                var uri = new Uri(source.URL);
                using var ftpClient = new AsyncFtpClient(uri.Host, source.Username, source.Password);

                await ftpClient.Connect();

                var remotePath = uri.AbsolutePath.TrimEnd('/') + "/" + fileName;
                Log.Information("上传文件到FTP: {LocalFilePath} -> {RemotePath}", localFilePath, remotePath);

                await ftpClient.UploadFile(localFilePath, remotePath);

                Log.Information("文件上传成功: {FileName}", fileName);
            }
            catch (Exception ex)
            {
                Log.Error(ex, "FTP upload failed for {FileName}: {Message}", fileName, ex.Message);
                throw;
            }
        }

        private static async Task UploadStreamToFtpAsync(DownloadSource source, string fileName, Stream stream)
        {
            try
            {
                var uri = new Uri(source.URL);
                using var ftpClient = new AsyncFtpClient(uri.Host, source.Username, source.Password);

                await ftpClient.Connect();

                var remotePath = uri.AbsolutePath.TrimEnd('/') + "/" + fileName;
                Log.Information("上传流到FTP: {FileName} -> {RemotePath}", fileName, remotePath);

                await ftpClient.UploadStream(stream, remotePath);

                Log.Information("流上传成功: {FileName}", fileName);
            }
            catch (Exception ex)
            {
                Log.Error(ex, "FTP stream upload failed for {FileName}: {Message}", fileName, ex.Message);
                throw;
            }
        }


        private static Dictionary<string, HashSet<string>> FileSHAs = new();
        private static Dictionary<string, List<DownloadSource>> FileSources = new();
        private static Dictionary<string, DownloadSource> Sources = new();
        private static ConcurrentDictionary<string, Task> DownloadingTasks = new();
        private static ConcurrentDictionary<string, System.Threading.Lock> DownloadingLocks = new();
    }

    public class DownloadSource
    {
        public required string Name { get; init; }
        public required string URL { get; init; }
        public required string Destination { get; init; }
        public required string ManifestPath { get; init; }
        public Byte[]? ManifestString { get; set; }
        public string? Username { get; init; }
        public string? Password { get; init; }
    }
}
