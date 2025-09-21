using SB;
using SB.Core;

[TargetScript]
public static class AsyncTests
{
    static AsyncTests()
    {
        Test.UnitTest("TestThreads")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("threads/threads.cpp");
            
        Test.UnitTest("TestServiceThread")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("threads/service_thread.cpp");
            
        Test.UnitTest("TestJob")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("threads/job.cpp");

        /* seems this little toy is buggy
        Test.UnitTest("Task2Test")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("task2/**.cpp");
        */

        Test.UnitTest("TestMarl")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrTask")
            .AddCppFiles("marl-test/**.cpp");
    }
}