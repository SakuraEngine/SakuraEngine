using SB;
using SB.Core;

[TargetScript]
public static class SkrBase
{
    static SkrBase()
    {
        var Target = Engine.StaticComponent("SkrBase", "SkrCore")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "rtm", "SkrProfile")
            // TODO: MIGRATE COMPILE FLAGS FROM XMAKE
            // .Depend(Visibility.Public, "SkrCompileFlags")
            .IncludeDirs(Visibility.Public, "include")
            .AddCFiles("src/**/build.*.c")
            .AddCppFiles("src/**/build.*.cpp");
        
        if (BuildSystem.TargetOS == OSPlatform.Windows)
            Target.Link(Visibility.Private, "advapi32");
        else if (BuildSystem.TargetOS == OSPlatform.OSX)
            Target.AppleFramework(Visibility.Private, "CoreFoundation");
    }
}