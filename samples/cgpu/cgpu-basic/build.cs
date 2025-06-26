using SB;
using SB.Core;

[TargetScript]
public static class CGPUSamples
{
    static CGPUSamples()
    {
        Engine.Program("CGPUMandelbrot")
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon", "lodepng")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("mandelbrot/mandelbrot.c")
            .AddHLSLFilesWithEntry("compute_main", "mandelbrot/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-mandelbrot");

        Engine.Program("CGPURayTracing")
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon", "lodepng")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("raytracing/raytracing.c")
            .AddHLSLFilesWithEntry("compute_main", "raytracing/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-raytracing");

        Engine.Program("CGPUIndexedInstance")
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("indexed-instance/*.c")
            .AddHLSLFiles("indexed-instance/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-indexed-instance");

        Engine.Program("CGPUTexture")
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/texture.c")
            .AddHLSLFiles("texture/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-texture");

        Engine.Program("CGPUTiledTexture")
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/tiled_texture.c");
    }
}