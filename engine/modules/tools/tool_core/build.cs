using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrToolCore
{
    static SkrToolCore()
    {
        Engine.Module("SkrToolCore", "TOOL_CORE")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/**.cpp")
            .UsePrivatePCH("src/pch.hpp")
            .AddMetaHeaders("include/**.h", "include/**.hpp")
            .CreateSharedPCH("include/**.h", "include/**.hpp");
    }
}