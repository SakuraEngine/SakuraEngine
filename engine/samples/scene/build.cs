using SB;
using SB.Core;

[TargetScript]
public static class SceneSamples
{
    static SceneSamples()
    {
        Engine.Module("SceneRenderer", "SCENE_RENDERER")
            .Depend(Visibility.Public, "SkrScene", "SkrGraphics"
                , "SkrImGui", "SkrCore", "SkrSystem", "SkrAnim")
            .IncludeDirs(Visibility.Public, "renderer")
            .AddCppFiles("renderer/*.cpp")
            .AddCppSLFiles("renderer/shaders/*.cxx")
            .CppSLOutputDirectory("resources/shaders/scene");

        Engine.Program("SceneSample_Simple")
            .AddCppFiles("simple/*.cpp")
            .Depend(Visibility.Private, "SkrScene")
            .Depend(Visibility.Private, "SkrSystem");

        Engine.Program("SceneSample_Serde")
            .AddCppFiles("serde/*.cpp")
            .Depend(Visibility.Private, "SceneRenderer")
            .Depend(Visibility.Private, "SkrTextureCompiler")
            .Depend(Visibility.Private, "SkrMeshTool");

        Engine.Program("SceneSample_Mesh")
            .AddCppFiles("mesh/*.cpp")
            .Depend(Visibility.Private, "SceneRenderer")
            .Depend(Visibility.Private, "SkrTextureCompiler")
            .Depend(Visibility.Private, "SkrMeshTool")
            .CopyFilesWithRoot(
                RootDir: "assets",
                Destination: "resources/scene",
                "assets/*.gltf",
                "assets/*.bin"
            );

        Engine.Program("SceneSample_SerdeResource")
            .AddCppFiles("serde_resource/*.cpp")
            .Depend(Visibility.Private, "SceneRenderer")
            .Depend(Visibility.Private, "SkrTextureCompiler")
            .Depend(Visibility.Private, "SkrMeshTool");

        Engine.Program("SceneSample_SkelMesh")
            .AddCppFiles("skelmesh/*.cpp")
            .Depend(Visibility.Private, "SceneRenderer")
            .Depend(Visibility.Private, "SkrTextureCompiler")
            .Depend(Visibility.Private, "SkrAnimTool")
            .Depend(Visibility.Private, "SkrMeshTool");
    }
}