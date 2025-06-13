using SB;
using SB.Core;

[TargetScript]
public static class AppSampleCommon
{
    static AppSampleCommon()
    {
        Engine.Target("AppSampleCommon")
            .TargetType(TargetType.HeaderOnly)
            .IncludeDirs(Visibility.Public, "include")
            .Depend(Visibility.Public, "SDL3")
            .Depend(Visibility.Public, "SkrCore", "SkrGraphics");
    }
}