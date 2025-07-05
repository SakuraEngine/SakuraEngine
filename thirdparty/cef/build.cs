using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class Cef
{
    static Cef()
    {
        if (BuildSystem.TargetOS != OSPlatform.Windows)
            return;
            
        Engine.AddDoctor<CefDoctor>();
        var @this = Engine.Target("cef")
            .TargetType(TargetType.Static)
            .Defines(Visibility.Public, "WRAPPING_CEF_SHARED", "NOMINMAX", "USING_CEF_SHARED=1")
            .Defines(Visibility.Private, "_HAS_EXCEPTIONS=0")
            .IncludeDirs(Visibility.Public, "include")
            .IncludeDirs(Visibility.Private, "./")
            .Link(Visibility.Private, "libcef")
            .AddCppFiles("libcef_dll/**.cc");

        @this.ClangCl_CXFlags(Visibility.Private, "-Wno-undefined-var-template");
    }
}

public class CefDoctor : IDoctor
{
    public bool Check()
    {
        if (BuildSystem.TargetOS != OSPlatform.Windows)
            return true;

        Install.SDK("cef-no-dxc-6778", new Dictionary<string, string> {
            { "Release", "./" }, 
            { "Resources", "Resources" }
        }).Wait();
        return true;
    }

    public bool Fix()
    {
        Log.Fatal("Cef SDK install failed!");
        return true;
    }
}