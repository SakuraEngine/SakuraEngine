using SB;
using SB.Core;

[TargetScript]
public static class SkrRT
{
    static SkrRT()
    {
        var SkrRT = Engine.Module("SkrRT", "SKR_RUNTIME")
            .Depend(Visibility.Public, "SkrTask")
            .Depend(Visibility.Public, "SkrGraphics")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/**/build.*.cpp")
            .AddCodegenScript("meta/ecs.ts");

        if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SkrRT.AppleFramework(Visibility.Public, "CoreFoundation", "Cocoa", "IOKit")
                // .MppFlags(Visibility.Public, "-fno-objc-arc")
                .AddObjCFiles("src/**/build.*.m")
                .AddObjCppFiles("src/**/build.*.mm");
        }
    }
}
