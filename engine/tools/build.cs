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
            .AddCppFiles("meta/src/**.cpp")
            .InstallArtifact();  // Auto-install to tools directory

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
            .IncludeDirs(Visibility.Private, "shader_compiler/LLVM/src")
            .AddCppFiles("shader_compiler/LLVM/src/**.cpp")
            .UsePrivatePCH("shader_compiler/LLVM/src/LLVM.pch.hpp")
            .LinkAgainstLLVM();

        BuildSystem.Target("CppSLCompiler")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "CppSLLLVM")
            .AddCppFiles("shader_compiler/shader_compiler.cpp")
            .InstallArtifact();  // Auto-install to tools directory based on Tool category

        BuildSystem.Target("CppSLManualTest")
            .TargetType(TargetType.Executable)
            .Depend(Visibility.Public, "CppSLAst")
            .AddCppFiles("shader_compiler/ast_test.cpp");
    }

    internal static string LLVMInstallPath => Path.Combine(BuildDirs.DownloadDir, "llvm-" + LLVMDownloader.Version);

    private static Target LinkAgainstLLVM(this Target @this)
    {
        var LibDir = Path.Combine(LLVMInstallPath, "lib");
        @this.RTTI(false)
            .Cl_CXFlags(Visibility.Private, "/wd4244", "/wd4291", "/wd4819")
            .IncludeDirs(Visibility.Private, Path.Combine(LLVMInstallPath, "include"))
            .LinkDirs(Visibility.Public, LibDir)
            .Defines(Visibility.Public, "CLANG_BUILD_STATIC")
            .Clang_CXFlags(Visibility.Public, "-Wno-preferred-type-bitfield-enum-conversion");

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

            @this.Link(Visibility.Private, "Ws2_32", "Version", "ntdll");
        }
        return @this;
    }
}

public class LLVMDownloader
{
    public static string Version = "20.1.8";
    public static bool Download()
    {
        Directory.CreateDirectory(BuildDirs.DownloadDir);

        string ZFileName = "";
        if (BuildSystem.HostOS == OSPlatform.OSX)
        {
            ZFileName = "llvm-darwin-" + Version + "-clang-arm64-release.7z";
        }
        else if (BuildSystem.HostOS == OSPlatform.Windows)
        {
            ZFileName = "llvm-windows-" + Version + "-msvc-x64-md-release.7z";
        }

        Install.SDK("llvm-21.1.1-release", new Dictionary<string, string> {
            { "./", LLVMTools.LLVMInstallPath }
        }).Wait();
        return false;
    }
}