using SB;
using SB.Core;

[TargetScript]
public static class SkrRuntime
{
    static SkrRuntime()
    {
        var SkrRuntime = Engine.Module("SkrRuntime", "SKR_RUNTIME")
            .Depend(Visibility.Public, "SkrTask")
            .Depend(Visibility.Public, "SkrGraphics")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/**/build.*.cpp")
            .AddCodegenScript("meta/ecs.ts")
            .AddNatvisFiles("dbg/*.natvis");

        if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SkrRuntime.AppleFramework(Visibility.Public, "CoreFoundation", "Cocoa", "IOKit")
                // .MppFlags(Visibility.Public, "-fno-objc-arc")
                .AddObjCFiles("src/**/build.*.m")
                .AddObjCppFiles("src/**/build.*.mm");
        }
    }
}
