using SB;
using SB.Core;

[TargetScript]
public static class SkrTask
{
    static SkrTask()
    {
        var MarlSourceDir = Path.Combine(Engine.EngineDirectory, "thirdparty/marl");
        var SkrTask = Engine
            .Module("SkrTask")
            .IncludeDirs(Visibility.Public, "include")
            .AddCppFiles("src/build.*.cpp")
            .Depend(Visibility.Public, "SkrCore")
            // add marl source
            .IncludeDirs(Visibility.Public, Path.Combine(MarlSourceDir, "include"))
            .AddCppFiles(Path.Combine(MarlSourceDir, "src/build.*.cpp"));

        if (BuildSystem.TargetOS != OSPlatform.Windows)
        {
            SkrTask.AddCFiles(
                Path.Combine(MarlSourceDir, "src/**.c"), 
                Path.Combine(MarlSourceDir, "src/**.S")
            );
        }

        if (!Engine.ShippingOneArchive)
        {
            SkrTask.Defines(Visibility.Public, "MARL_DLL")
                   .Defines(Visibility.Private,"MARL_BUILDING_DLL");
        }
    }
}