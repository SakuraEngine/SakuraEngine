using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SketchUpDump
{
    static SketchUpDump()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            var SkrSketchUpDump = Engine.Program("SketchUpDump")
                    .EnableUnityBuild()
                    .Depend(Visibility.Private, "SkrCore", "SkrTask", "SkrSketchUp")
                    .IncludeDirs(Visibility.Private, "include")
                    .AddCppFiles("src/**.cpp");
        }
    }
}
