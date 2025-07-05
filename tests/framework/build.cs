using SB;
using SB.Core;
using Serilog;
using System.Runtime.CompilerServices;

[TargetScript]
public static class TestFramework
{
    static TestFramework()
    {
        var TestFramework = BuildSystem.Target("SkrTestFramework")
            .TargetType(TargetType.Static)
            .Depend(Visibility.Public, "SkrCore")
            .IncludeDirs(Visibility.Public, "include", "include/SkrTestFramework")
            .AddCppFiles("src/framework.cpp");

        if (BuildSystem.TargetOS == OSPlatform.Windows)
            TestFramework.CXFlags(Visibility.Public, "/utf-8");
    }
}

namespace SB
{
    public partial class Engine : BuildSystem
    {
        public static Target UnitTest(string Name, [CallerFilePath] string? Location = null, [CallerLineNumber] int LineNumber = 0)
        {
            return BuildSystem.Target(Name, Location, LineNumber)
                .UseSharedPCH()
                .TargetType(TargetType.Executable)
                .Depend(Visibility.Private, "SkrTestFramework")
                .Exception(false)
                .SetCategory(TargetCategory.Tests);
        }
    }
}