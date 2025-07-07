using System.Net;
using SB;
using SB.Core;
using Serilog;
using SharpCompress.Archives;

[TargetScript(TargetCategory.Tool)]
public static class LLVMTools
{
    static LLVMTools()
    {
        bool UsePrecompiledCompiler = false;
        if (UsePrecompiledCompiler)
            return;

        LLVMDownloader.Download();

        BuildSystem.Target("meta")
            .TargetType(TargetType.Executable)
            .LinkAgainstLLVM()
            .AddCppFiles("meta/src/**.cpp");

        BuildSystem.Target("CppSLAst")
            .TargetType(TargetType.Static)
            .RTTI(true)
            .IncludeDirs(Visibility.Public, "shader_compiler/AST/include")
            .IncludeDirs(Visibility.Public, "shader_compiler/AST/double-conversion")
            .AddCppFiles("shader_compiler/AST/double-conversion/**.cc")
            .AddCppFiles("shader_compiler/AST/src/**.cpp");

        BuildSystem.Target("CppSLLLVM")
            .TargetType(TargetType.Static)
            .Depend(Visibility.Public, "CppSLAst")
            .IncludeDirs(Visibility.Public, "shader_compiler/LLVM/include")
            .AddCppFiles("shader_compiler/LLVM/src/**.cpp")
            .LinkAgainstLLVM();

        BuildSystem.Target("CppSLCompiler")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "CppSLLLVM")
            .AddCppFiles("shader_compiler/shader_compiler.cpp");

        BuildSystem.Target("CppSLManualTest")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "CppSLAst")
            .AddCppFiles("shader_compiler/ast_test.cpp");
    }

    private static Target LinkAgainstLLVM(this Target @this)
    {
        var LibDir = Path.Combine(Engine.DownloadDirectory, "llvm-" + LLVMDownloader.Version, "lib");
        @this.RTTI(false)
            .IncludeDirs(Visibility.Private, Path.Combine(Engine.DownloadDirectory, "llvm-" + LLVMDownloader.Version, "include"))
            .LinkDirs(Visibility.Public, LibDir);

        var libs = new List<string>();
        if (BuildSystem.HostOS == OSPlatform.OSX)
        {
            var files = Directory.GetFiles(LibDir, "lib*.a");
            foreach (var filepath in files)
            {
                var basename = Path.GetFileName(filepath);
                var match = System.Text.RegularExpressions.Regex.Match(basename, @"lib(.*)\.a$");
                var libName = match.Success ? match.Groups[1].Value : Path.GetFileNameWithoutExtension(basename);
                libs.Add(libName);
            }
            @this.Link(Visibility.Public, libs.ToArray());

            @this.Require("zlib", new PackageConfig { Version = new Version(1, 2, 8) })
                .Depend(Visibility.Public, "zlib@zlib")
                .Link(Visibility.Public, "pthread", "curses");
        }
        else if (Engine.HostOS == OSPlatform.Windows)
        {
            var files = Directory.GetFiles(LibDir, "*.lib");
            foreach (var filepath in files)
            {
                var basename = Path.GetFileName(filepath);
                var match = System.Text.RegularExpressions.Regex.Match(basename, @"(.*)\.lib$");
                var libName = match.Success ? match.Groups[1].Value : Path.GetFileNameWithoutExtension(basename);
                libs.Add(libName);
            }
            libs.Remove("LLVM-C");
            libs.Remove("LTO");
            libs.Remove("libclang");
            libs.Remove("Remarks");
            @this.Link(Visibility.Private, libs.ToArray());

            @this.Link(Visibility.Private, "Ws2_32", "Version");
        }
        return @this;
    }
}

public class LLVMDownloader
{
    public static string Version = "18.1.6";
    public static bool Download()
    {
        string URL = "";
        Directory.CreateDirectory(Engine.DownloadDirectory);
        string Destination = Path.Combine(Engine.DownloadDirectory, "llvm-" + Version + ".7z");
        if (BuildSystem.HostOS == OSPlatform.OSX)
        {
            URL = "https://github.com/SakuraEngine/llvm-build/releases/download/llvm-darwin-" + Version + "/llvm-darwin-" + Version + "-clang-arm64-release.7z";
        }
        else if (BuildSystem.HostOS == OSPlatform.Windows)
        {
            URL = "https://github.com/SakuraEngine/llvm-build/releases/download/llvm-windows-" + Version + "/llvm-windows-" + Version + "-msvc-x64-md-release.7z";
        }
        Engine.ConfigureNotAwareDepend.OnChanged("Download-LLVM", "LLVM-" + Version, "LLVMDoctor", (Depend depend) =>
        {
            Log.Verbose("http handshaking ... from {URL} to {Destination}", URL, Destination);
            using (var Http = new HttpClient(new HttpClientHandler { Proxy = SB.Download.HttpProxyObject.Value }))
            {
                Log.Information("downloading ... from {URL} to {Destination}", URL, Destination);
                var Bytes = Http.GetByteArrayAsync(URL);
                Bytes.Wait();
                File.WriteAllBytes(Destination, Bytes.Result);
            }
            depend.ExternalFiles.Add(Destination);
        }, null, null);

        Engine.ConfigureNotAwareDepend.OnChanged("Install-LLVM", "LLVM-" + Version, "LLVMDoctor", (Depend depend) =>
        {
            var IntermediateDirectory = Path.Combine(Engine.DownloadDirectory, "llvm-" + Version);
            Directory.CreateDirectory(IntermediateDirectory);
            using (var archive = SharpCompress.Archives.SevenZip.SevenZipArchive.Open(Destination))
            {
                Log.Information("Extracting ... from {Destination} to {IntermediateDirectory}", Destination, IntermediateDirectory);
                archive.WriteToDirectory(IntermediateDirectory, new SharpCompress.Common.ExtractionOptions()
                {
                    ExtractFullPath = true,
                    Overwrite = true
                });
            }
            depend.ExternalFiles.Add(Destination);
        }, null, null);
        Log.Verbose("LLVM SDK Installation complete, version {Version}", Version);
        return true;
    }
}