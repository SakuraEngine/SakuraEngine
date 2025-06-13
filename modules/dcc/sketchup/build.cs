using SB;
using SB.Core;
using Serilog;

[TargetScript]
[SkecthUpDoctor]
public static class SkrSketchUp
{
    static SkrSketchUp()
    {
        SketchUpAPI();
        SketchUpLive();
    }

    private static void SketchUpAPI()
    {
        var SkrSketchUp = Engine.Module("SkrSketchUp", "SKR_SKETCH_UP")
            .ApplySketchUpSettings()
            .AddCppFiles("src/build.*.cpp");

        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SkrSketchUp.Link(Visibility.Public, "SketchUpAPI");
        }
        else if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SkrSketchUp.Link(Visibility.Public, 
                "CommonGeometry",
                "CommonGeoutils",
                "CommonImage",
                "CommonPreferences",
                "CommonUnits",
                "CommonUtils",
                "CommonZip"
            );
        }
    }

    private static void SketchUpLive()
    {
        var SkrSketchUpLive = Engine.Module("SkrSketchUpLive", "SKR_SKETCH_UP")
            .ApplySketchUpSettings()
            .AddCppFiles("src/build.*.cpp");
        
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SkrSketchUpLive.LinkDirs(Visibility.Public, "libs/windows")
                .Link(Visibility.Public, "sketchup")
                .Link(Visibility.Public, "x64-msvcrt-ruby270");
        }
        else if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            // SkrSketchUpLive.BundleLoader("/Applications/SketchUp 2023/SketchUp.app/Contents/MacOS/SketchUp");
        }
    }

    private static Target ApplySketchUpSettings(this Target @this)
    {
        @this.Depend(Visibility.Public, "SkrCore")
            .AppleFramework(Visibility.Public, "CoreFoundation")
            .IncludeDirs(Visibility.Public, "SketchUp");
        return @this;
    }
}

public class SkecthUpDoctor : DoctorAttribute
{
    public override bool Check()
    {
        Task.WaitAll(
            Install.SDK("sketchup-sdk-v2023.1.315")
        );
        return true;
    }
    public override bool Fix() 
    { 
        Log.Fatal("sketchup sdks install failed!");
        return true; 
    }
}