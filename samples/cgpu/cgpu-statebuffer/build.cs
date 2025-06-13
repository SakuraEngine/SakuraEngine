using SB;
using SB.Core;
using System.Runtime.CompilerServices;

[TargetScript]
public static class StateBufferTriangle
{
    static StateBufferTriangle()
    {
        Engine.Program("StateBufferTriangle")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("triangle/statebuffer_triangle.c")
            .AddHLSLFiles("triangle/shaders/**.hlsl")
            .DXCOutputDirectory("resources/shaders/statebuffer-triangle");
    }
}