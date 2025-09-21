using System.Diagnostics;
using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrGraphics
{
    static SkrGraphics()
    {
        Engine.AddSetup<SkrGraphicsSetup>();

        var SkrGraphics = Engine
            .Module("SkrGraphics")
            .Depend(Visibility.Public, "SkrCore")
            .Require("VulkanHeaders", new PackageConfig { Version = new Version(1, 4, 324) })
            .Depend(Visibility.Public, "VulkanHeaders@VulkanHeaders")
            .IncludeDirs(Visibility.Public, "include")
            .AddCFiles("src/build.*.c")
            .AddCppFiles("src/build.*.cpp");

        // ignore warnings from SDKs
        SkrGraphics.ClangCl_CXFlags(Visibility.Private,
            "-Wno-switch",
            "-Wno-microsoft-cast",
            "-Wno-ignored-attributes",
            "-Wno-nullability-completeness",
            "-Wno-tautological-undefined-compare"
        );

        if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            var OCOptions = new CFamilyFileOptions();
            OCOptions.Arguments.CppFlags("-fobjc-arc");

            SkrGraphics
                .AddObjCFiles(OCOptions, "src/build.*.m")
                .AddObjCppFiles(OCOptions, "src/build.*.mm")
                .AppleFramework(Visibility.Public, "CoreFoundation", "Cocoa", "Metal", "IOKit", "QuartzCore")
                .Defines(Visibility.Private, "VK_USE_PLATFORM_MACOS_MVK");
        }

        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SkrGraphics.Defines(Visibility.Private, "UNICODE")
                .Defines(Visibility.Private, "D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED")

                .Require("D3D12Agility", new PackageConfig { Version = new Version(1, 616, 1) })
                .Depend(Visibility.Public, "D3D12Agility@D3D12Agility")

                .Require("WinPixEventRuntime", new PackageConfig { Version = new Version(1, 0, 240308001) })
                .Depend(Visibility.Private, "WinPixEventRuntime@WinPixEventRuntime")

                .Require("DirectStorage", new PackageConfig { Version = new Version(1, 3, 0) })
                .Depend(Visibility.Private, "DirectStorage@DirectStorage")

                .Require("NvML", new PackageConfig { Version = new Version(13, 0, 0) })
                .Depend(Visibility.Private, "NvML@NvML")

                .Require("NvApi", new PackageConfig { Version = new Version(580, 0, 0) })
                .Depend(Visibility.Private, "NvApi@NvApi")

                .Require("NvPerf", new PackageConfig { Version = new Version(2025, 1, 0) })
                .Depend(Visibility.Private, "NvPerf@NvPerf")

                .Require("NvAftermath", new PackageConfig { Version = new Version(2025, 1, 0) })
                .Depend(Visibility.Private, "NvAftermath@NvAftermath")

                .Require("AmdAgs", new PackageConfig { Version = new Version(6, 3, 0) })
                .Depend(Visibility.Private, "AmdAgs@AmdAgs")

                .Require("AmdGPUPerf", new PackageConfig { Version = new Version(4, 1, 0) })
                .Depend(Visibility.Private, "AmdGPUPerf@AmdGPUPerf")

                .Link(Visibility.Private, "WinPixEventRuntime");
        }
    }
}

public class SkrGraphicsSetup : ISetup
{
    public void Setup()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            Task.WaitAll(
                Install.SDK("dxc-2025_07_14")
            );
        }
    }
}