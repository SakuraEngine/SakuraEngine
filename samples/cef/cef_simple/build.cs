using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class CefSimple
{
    static CefSimple()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            var CefSimple = Engine.Target("cef_simple")
                .TargetType(TargetType.Executable)
                .Depend(Visibility.Private, "cef")
                .Defines(Visibility.Private, "UNICODE")
                .AddCppFiles("src/**.cc");
            
            if (BuildSystem.TargetOS == OSPlatform.Windows)
                CefSimple.Link(Visibility.Private, "User32");
        }
    }
}
