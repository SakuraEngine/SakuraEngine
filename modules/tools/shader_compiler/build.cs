using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrShaderCompiler
{
    static SkrShaderCompiler()
    {
        var ShaderCompiler = Engine.Module("SkrShaderCompiler")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRenderer", "SkrToolCore")
            .IncludeDirs(Visibility.Public, "include")
            .IncludeDirs(Visibility.Private, "src")
            .AddMetaHeaders("include/**.hpp")
            .AddCppFiles("src/**.cpp")
            .UsePrivatePCH("src/pch.hpp");

        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            ShaderCompiler.ClangCl_CppFlags(Visibility.Private, "-fms-extensions");
        }
        else
        {
            ShaderCompiler.CppFlags(Visibility.Private, "-fms-extensions");
            ShaderCompiler.IncludeDirs(Visibility.Private, "src/dxc/Support");
        }
    }
}