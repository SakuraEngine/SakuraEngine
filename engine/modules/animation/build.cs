using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrAnim
{
    static SkrAnim()
    {
        Engine.Module("SkrOzz", "SKR_OZZ")
            .EnableUnityBuild()
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrRuntime")
            .IncludeDirs(Visibility.Public, "ozz")
            .IncludeDirs(Visibility.Public, "ozz/SkrAnim")
            .IncludeDirs(Visibility.Private, "ozz_src")
            .AddCppFiles("ozz_src/**.cc")
            .Cl_CXFlags(Visibility.Private, "/wd4661")
            .UsePrivatePCH("ozz/SkrAnim/ozz/*.h");

        Engine.Module("SkrAnim", "SKR_ANIM")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrOzz", "SkrRenderer")
            .IncludeDirs(Visibility.Public, "include")
            .IncludeDirs(Visibility.Public, "ozz")
            .AddCppFiles("src/**.cpp")
            .AddMetaHeaders("include/**.h", "include/**.hpp")
            .UsePrivatePCH("src/pch.hpp");
    }
}