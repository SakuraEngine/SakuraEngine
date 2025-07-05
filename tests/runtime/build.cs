using SB;
using SB.Core;

[TargetScript]
public static class RuntimeTests
{
    static RuntimeTests()
    {
        Engine.UnitTest("MathTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("math/math.cpp");

        Engine.UnitTest("GoapTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("goap/test_goap.cpp");

        Engine.UnitTest("GraphTest")
            .Depend(Visibility.Public, "SkrRenderGraph")
            .AddCppFiles("graph/graph.cpp");

        Engine.UnitTest("VFSTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("vfs/main.cpp");

        Engine.UnitTest("IOServiceTest")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("io_service/main.cpp");

        Engine.UnitTest("SceneTest")
            .Depend(Visibility.Public, "SkrScene")
            .AddCppFiles("scene/main.cpp");

        Engine.UnitTest("ECSTest_CStyle")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("ecs/c_style/*.cpp");

        Engine.Program("ECSTest_CPPStyle")
            .EnableCodegen("ecs/cpp_style")
            .AddMetaHeaders("ecs/cpp_style/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("ecs/cpp_style/*.cpp");

        Engine.Program("RTTRTest")
            .EnableCodegen("rttr")
            .AddMetaHeaders("rttr/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("rttr/**.cpp");

        Engine.Program("ProxyTest")
            .EnableCodegen("proxy")
            .AddMetaHeaders("proxy/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrRT")
            .AddCppFiles("proxy/**.cpp");

        Engine.Program("V8Test")
            .EnableCodegen("v8")
            .AddMetaHeaders("v8/**.hpp")
            .Depend(Visibility.Private, "SkrTestFramework")
            .Depend(Visibility.Public, "SkrV8")
            .AddCppFiles("v8/**.cpp");
    }
}