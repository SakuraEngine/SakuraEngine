using SB;
using SB.Core;

[TargetScript]
public static class CGPUSamples
{
    static CGPUSamples()
    {
        Engine.Program("CGPUMandelbrot")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("mandelbrot/*.c")
            .AddHLSLFilesWithEntry("compute_main", "mandelbrot/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-mandelbrot");

        Engine.Program("CGPUIndexedInstance")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("indexed-instance/*.c")
            .AddHLSLFiles("indexed-instance/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-indexed-instance");

        Engine.Program("CGPUTexture")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/texture.c")
            .AddHLSLFiles("texture/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-texture");

        Engine.Program("CGPUTiledTexture")
            .EnableUnityBuild()
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/tiled_texture.c");
    }
}