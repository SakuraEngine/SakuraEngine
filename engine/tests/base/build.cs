using SB;
using SB.Core;
using System.Runtime.CompilerServices;

[TargetScript]
public static class BaseTests
{
    static BaseTests()
    {
        Test.UnitTest("TestOS")
            .AddCppFiles("os/main.cpp");

        Test.UnitTest("TestAlgo")
            .AddCppFiles("algo/*.cpp");

        Test.UnitTest("TestContainers")
            .AddCppFiles("containers/*.cpp");

        Test.UnitTest("TestMath")
            .AddCppFiles("math/*.cpp");

        Test.UnitTest("TestFileSystem")
            .AddCppFiles("filesystem/*.cpp");
    }
}