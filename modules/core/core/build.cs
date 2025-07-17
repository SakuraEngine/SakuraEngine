using SB;
using SB.Core;
using Serilog;

[TargetScript]
[SkrCoreDoctor]
public static class SkrCore
{
    static SkrCore()
    {
        var DependencyGraph = Engine.StaticComponent("SkrDependencyGraph", "SkrCore")
            .Exception(true) // DAG uses lemon which uses exceptions
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .Require("lemon", new PackageConfig { Version = new Version(1, 3, 1) })
            .Depend(Visibility.Private, "lemon@lemon")
            .AddCppFiles("src/graph/build.*.cpp");
        
        var SkrString = Engine.StaticComponent("SkrString", "SkrCore")
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .Defines(Visibility.Public, "OPEN_STRING_API=")
            .AddCppFiles("src/string/build.*.cpp");

        var SkrSimpleAsync = Engine.StaticComponent("SkrSimpleAsync", "SkrCore")
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .AddCppFiles("src/async/build.*.cpp");

        var SkrArchive = Engine.StaticComponent("SkrArchive", "SkrCore")
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .Require("yyjson", new PackageConfig { Version = new Version(0, 9, 0) })
            .Depend(Visibility.Private, "yyjson@yyjson")
            .AddCppFiles("src/archive/build.*.cpp");

        var SkrCore = Engine.Module("SkrCore")
            .Require("phmap", new PackageConfig { Version = new Version(1, 3, 11) })
            .Depend(Visibility.Public, "phmap@phmap")
            .Depend(Visibility.Private, "mimalloc")
            .Depend(Visibility.Public, "SkrProfile")
            // TODO: STATIC COMPONENTS, JUST WORKAROUND NOW, WE NEED TO DEPEND THEM AUTOMATICALLY LATTER
            .Depend(Visibility.Public, "SkrDependencyGraph", "SkrString", "SkrSimpleAsync", "SkrArchive")
            // END WORKAROUNDS
            .IncludeDirs(Visibility.Public, "include")
            .Defines(Visibility.Private, "SKR_MEMORY_IMPL")
            // Core Files
            .AddCFiles("src/core/build.*.c")
            .AddCppFiles("src/core/build.*.cpp")
            // RTTR Files
            .AddCppFiles("src/rttr/build.*.cpp")
            // Codegen Files
            .AddMetaHeaders(
                "include/SkrRTTR/iobject.hpp",
                "include/SkrRTTR/script/scriptble_object.hpp",
                "include/SkrRTTR/export/export_data.hpp",
                "include/SkrCore/cli.hpp"
            )
            .AddCodegenScript("meta/basic.ts")
            .AddCodegenScript("meta/rttr.ts")
            .AddCodegenScript("meta/serialize.ts")
            .AddCodegenScript("meta/proxy.ts")
            .CreateSharedPCH("include/**.h", "include/**.hpp", "../profile/include/SkrProfile/profile.h")
            // TODO: REMOVE THIS
            .Depend(Visibility.Public, "SDL3");

        if (BuildSystem.TargetOS == OSPlatform.Windows)
            SkrCore.Link(Visibility.Private, "shell32", "Ole32", "Shlwapi");
        else if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SkrCore.AddObjCppFiles("src/**/build.*.mm")
                .AppleFramework(Visibility.Public, "CoreFoundation", "Cocoa", "IOKit");
        }
        else
            SkrCore.Link(Visibility.Private, "pthread");
    }
}

public class SkrCoreDoctor : DoctorAttribute
{
    public override bool Check()
    {
        Install.SDK("SDL_3.2.12").Wait();
        return true;
    }
    public override bool Fix() 
    { 
        Log.Fatal("core sdks install failed!");
        return true; 
    }
}