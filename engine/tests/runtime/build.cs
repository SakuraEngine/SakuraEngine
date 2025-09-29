using SB;
using SB.Core;

[TargetScript]
public static class RuntimeTests
{
    static RuntimeTests()
    {
        Test.UnitTest("TestGoap")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("goap/test_goap.cpp");

        Test.UnitTest("TestGraph")
            .Depend(Visibility.Public, "SkrRenderGraph")
            .AddCppFiles("graph/graph.cpp");

        Test.UnitTest("TestVFS")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("vfs/main.cpp");

        Test.UnitTest("TestIOService")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("io_service/*.cpp");

        Test.UnitTest("TestECS_CStyle")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("ecs/c_style/*.cpp");

        Engine.Program("TestECS_CPPStyle")
            .EnableCodegen("ecs/cpp_style")
            .AddMetaHeaders("ecs/cpp_style/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("ecs/cpp_style/*.cpp");

        Engine.Program("TestV8")
            .EnableCodegen("v8")
            .AddMetaHeaders("v8/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrV8")
            .AddCppFiles("v8/**.cpp");

    }
}