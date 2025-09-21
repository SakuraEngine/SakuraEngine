using SB;
using SB.Core;

[TargetScript]
public static class SkrV8
{
    static SkrV8()
    {
        Engine.Module("SkrV8")
            .Require("V8", new PackageConfig { Version = new Version(1, 0, 0) })
            .Require("LibHV", new PackageConfig { Version = new Version(1, 3, 3) })
            .Depend(Visibility.Public, "SkrRuntime", "V8@V8", "LibHV@LibHV")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/**.cpp")
            .AddMetaHeaders("include/**.hpp");
    }
}