using SB;
using SB.Core;

[TargetScript]
public static class CGPUSamples
{
    static CGPUSamples()
    {
        Engine.Program("CGPUMandelbrot")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon", "lodepng")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("mandelbrot/mandelbrot.c")
            .AddCppSLFiles("mandelbrot/**.cxx")
            .CppSLOutputDirectory("resources/shaders/cgpu-mandelbrot");

        Engine.Program("CGPURayTracing")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon", "lodepng")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("raytracing/raytracing.c")
            .AddCppSLFiles("raytracing/**.cxx")
            .CppSLOutputDirectory("resources/shaders/cgpu-raytracing");

        Engine.Program("CGPUIndexedInstance")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("indexed-instance/*.c")
            .AddHLSLFiles("indexed-instance/**.hlsl")
            .DXCOutputDirectory("resources/shaders/cgpu-indexed-instance");

        Engine.Program("CGPUTexture")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/texture.c")
            .AddCppSLFiles("texture/**.cxx")
            .CppSLOutputDirectory("resources/shaders/cgpu-texture");

        Engine.Program("CGPUTiledTexture")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/tiled_texture.c");

        Engine.Program("CGPUBindlessTexture")
            .Depend(Visibility.Public, "SkrRuntime")
            .Depend(Visibility.Private, "AppSampleCommon")
            .IncludeDirs(Visibility.Private, "./../../common")
            .AddCFiles("texture/bindless_texture.c");
    }
}