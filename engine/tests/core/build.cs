using SB;
using SB.Core;

[TargetScript]
public static class CoreTests
{
    static CoreTests()
    {
        Engine.Program("TestSerde")
            .EnableUnityBuild()
            .EnableCodegen("serde")
            .AddMetaHeaders("serde/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("serde/*.cpp");

        Test.UnitTest("TestNatvis")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("natvis/*.cpp");

        Test.UnitTest("TestDelegate")
            .AddCppFiles("delegate/*.cpp");

        Engine.Program("TestRTTR")
            .EnableCodegen("rttr")
            .AddMetaHeaders("rttr/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("rttr/**.cpp");

        Engine.Program("TestProxy")
            .EnableCodegen("proxy")
            .AddMetaHeaders("proxy/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("proxy/**.cpp");

        Test.UnitTest("TestRC")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrCore")
            .AddCppFiles("rc/*.cpp");
    }
}