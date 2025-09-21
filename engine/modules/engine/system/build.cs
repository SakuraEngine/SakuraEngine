using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrSystem
{
    static SkrSystem()
    {
        var SkrSystem = Engine.Module("SkrSystem", "SKR_SYSTEM")
            .EnableUnityBuild()
            .Require("SDL3", new PackageConfig { Version = new Version(1, 0, 0) })
            .Depend(Visibility.Public, "SDL3@SDL3")
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles(
                "src/advanced_input/*.cpp",
                "src/advanced_input/common/**.cpp",
                "src/advanced_input/sdl3/**.cpp"
            )
            .AddCppFiles(
                "src/system/*.cpp",
                "src/system/sdl3/**.cpp"
            );

        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SkrSystem
                .Defines(Visibility.Private, "SKR_INPUT_USE_GAME_INPUT")
                .AddCppFiles("src/advanced_input/game_input/**.cpp")
                .AddCppFiles("src/system/win32/**.cpp");
        }
        else if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SkrSystem.AddCppFiles("src/system/cocoa/**.cpp")
                     .AddObjCppFiles("src/system/cocoa/**.mm")
                     .AppleFramework(Visibility.Private, "Cocoa", "Metal", "QuartzCore");
        }
    }
}