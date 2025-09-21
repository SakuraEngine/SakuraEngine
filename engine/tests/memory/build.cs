using SB;
using SB.Core;
using System.Runtime.CompilerServices;

[TargetScript]
public static class MemoryTests
{
    static MemoryTests()
    {
        Test.UnitTest("TestSSM")
            .AddCppFiles("SSM/*.cpp");
    }
}