using SB;
using SB.Core;

[TargetScript]
public static class AsyncTests
{
    static AsyncTests()
    {
        Engine.UnitTest("ThreadsTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("threads/threads.cpp");
            
        Engine.UnitTest("ServiceThreadTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("threads/service_thread.cpp");
            
        Engine.UnitTest("JobTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("threads/job.cpp");

        /* seems this little toy is buggy
        Engine.UnitTest("Task2Test")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("task2/**.cpp");
        */

        Engine.UnitTest("MarlTest")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("marl-test/**.cpp");
    }
}