using SB.Core;
using SB;

[TargetScript]
public static class TimelineStressTest
{
    static TimelineStressTest()
    {
        Engine.Program("TimelineStressTest")
            .TargetType(TargetType.Executable)
            .IncludeDirs(Visibility.Public, ".")
            .AddCppFiles("timeline_stress_test.cpp")
            .Depend(Visibility.Private, "SkrRenderGraph")
            .Depend(Visibility.Private, "SkrCore")
            .Depend(Visibility.Private, "SkrBase");
            
        Engine.Program("AdvancedTimelineTest")
            .TargetType(TargetType.Executable)
            .IncludeDirs(Visibility.Public, ".")
            .AddCppFiles("advanced_timeline_test.cpp")
            .Depend(Visibility.Private, "SkrRenderGraph")
            .Depend(Visibility.Private, "SkrCore")
            .Depend(Visibility.Private, "SkrBase");
    }
}