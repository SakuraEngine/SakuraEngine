using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class HotfixTest
{
    static HotfixTest()
    {
        Engine.Module("HotfixTest")
            .Depend(Visibility.Public, "SkrRuntime")
            .AddCppFiles("hotfix_module.cpp");

        Engine.Program("HotfixTestHost")
            .Depend(Visibility.Public, "HotfixTest")
            .AddCppFiles("main.cpp");
    }
}
