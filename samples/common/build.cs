using SB;
using SB.Core;

[TargetScript]
public static class AppSampleCommon
{
    static AppSampleCommon()
    {
        Engine.Target("lodepng")
            .TargetType(TargetType.Static)
            .IncludeDirs(Visibility.Public, "lodepng")
            .AddCFiles("lodepng/lodepng.c");
            
        Engine.Target("AppSampleCommon")
            .TargetType(TargetType.HeaderOnly)
            .IncludeDirs(Visibility.Public, "include")
            .Depend(Visibility.Public, "SDL3")
            .Depend(Visibility.Public, "SkrCore", "SkrGraphics");
    }
}