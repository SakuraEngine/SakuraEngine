using System.Diagnostics;
using SB;
using SB.Core;
using Serilog;

[TargetScript]
[SkrGraphicsDoctor]
public static class SkrGraphics
{
    static SkrGraphics()
    {
        var SkrGraphics = Engine
            .Module("SkrGraphics")
            .Depend(Visibility.Public, "SkrCore")
            .Depend(Visibility.Private, "VulkanHeaders")
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
            OCOptions.Arguments.CppFlags(Visibility.Private, "-fno-objc-arc");

            SkrGraphics
                .AddObjCFiles(OCOptions, "src/build.*.m") 
                .AddObjCppFiles(OCOptions, "src/build.*.mm") 
                .AppleFramework(Visibility.Public, "CoreFoundation", "Cocoa", "Metal", "IOKit")
                .Defines(Visibility.Private, "VK_USE_PLATFORM_MACOS_MVK");
        }

        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SkrGraphics
                .Defines(Visibility.Private, "UNICODE")
                .Link(Visibility.Private, "nvapi_x64")
                .Link(Visibility.Private, "WinPixEventRuntime");
        }
    }
}

public class SkrGraphicsDoctor : DoctorAttribute
{
    public override bool Check()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            Task.WaitAll(
                Install.SDK("dxc-2025_02_21"),
                Install.SDK("amdags"),
                Install.SDK("nvapi"),
                Install.SDK("nsight"),
                Install.SDK("WinPixEventRuntime")
            );
        }
        return true;
    }
    public override bool Fix() 
    { 
        Log.Fatal("graphics sdks install failed!");
        return true; 
    }
}