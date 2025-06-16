using SB;
using SB.Core;

[TargetScript]
public static class WebSocketSample
{
    static WebSocketSample()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            var WebSocketSample = Engine.Program("WebSocketSample")
                .Depend(Visibility.Public, "libhv")
                .Depend(Visibility.Public, "SkrCore")
                .IncludeDirs(Visibility.Public, "include")
                .AddMetaHeaders("include/**.hpp")
                .AddCppFiles("src/**.cpp");
        }
    }
}
