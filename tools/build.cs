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

        BuildSystem.Target("SSLAst")
            .TargetType(TargetType.Static)
            .RTTI(true)
            .IncludeDirs(Visibility.Public, "shader_compiler/AST/include")
            .IncludeDirs(Visibility.Public, "shader_compiler/AST/double-conversion")
            .AddCppFiles("shader_compiler/AST/double-conversion/**.cc")
            .AddCppFiles("shader_compiler/AST/src/**.cpp");

        BuildSystem.Target("SSLLLVM")
            .TargetType(TargetType.Static)
            .Depend(Visibility.Public, "SSLAst")
            .IncludeDirs(Visibility.Public, "shader_compiler/LLVM/include")
            .AddCppFiles("shader_compiler/LLVM/src/**.cpp")
            .LinkAgainstLLVM();

        BuildSystem.Target("SSLCompiler")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "SSLLLVM")
            .AddCppFiles("shader_compiler/shader_compiler.cpp");

        BuildSystem.Target("SSL_ManualTest")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "SSLAst")
            .AddCppFiles("shader_compiler/ast_test.cpp");
    }

    private static Target LinkAgainstLLVM(this Target @this)
    {
        var LibDir = Path.Combine(Engine.DownloadDirectory, "llvm-" + LLVMDownloader.Version, "lib");
        @this.RTTI(false)
            .IncludeDirs(Visibility.Private, Path.Combine(Engine.DownloadDirectory, "llvm-" + LLVMDownloader.Version, "include"))
            .LinkDirs(Visibility.Private, LibDir);

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
            libs.Remove("LLVM-C");
            libs.Remove("LTO");
            libs.Remove("libclang");
            libs.Remove("Remarks");
            @this.Link(Visibility.Private, libs.ToArray());
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
        string Destination = Path.Combine(Engine.DownloadDirectory, "llvm-" + Version + ".zip");
        if (BuildSystem.HostOS == OSPlatform.OSX)
        {
            URL = "https://github.com/SakuraEngine/llvm-build/releases/download/llvm-darwin-" + Version + "/llvm-darwin-" + Version + "-clang-arm64-release.7z";
        }
        else if (BuildSystem.HostOS == OSPlatform.Windows)
        {
            URL = "https://github.com/SakuraEngine/llvm-build/releases/download/llvm-windows-" + Version + "/llvm-windows-" + Version + "-msvc-x64-md-release.7z";
        }
        Depend.OnChanged("Download-LLVM", "LLVM-" + Version, "LLVMDoctor", (Depend depend) =>
        {
            using (var Http = new HttpClient())
            {
                Log.Information("downloading ... from {URL} to {Destination}", URL, Destination);
                var Bytes = Http.GetByteArrayAsync(URL);
                Bytes.Wait();
                File.WriteAllBytes(Destination, Bytes.Result);
            }
            depend.ExternalFiles.Add(Destination);
        }, null, null);

        Depend.OnChanged("Install-LLVM", "LLVM-" + Version, "LLVMDoctor", (Depend depend) =>
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
        return true;
    }
}