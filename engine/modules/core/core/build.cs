using SB;
using SB.Core;
using Serilog;

[TargetScript]
public static class SkrCore
{
    static SkrCore()
    {
        Engine.AddSetup<SkrCoreSetup>();

        var DependencyGraph = Engine.StaticComponent("SkrDependencyGraph", "SkrCore")
            .Exception(true) // DAG uses lemon which uses exceptions
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .Require("lemon", new PackageConfig { Version = new Version(1, 3, 1) })
            .Depend(Visibility.Private, "lemon@lemon")
            .AddCppFiles("src/graph/build.*.cpp");

        var SkrSimpleAsync = Engine.StaticComponent("SkrSimpleAsync", "SkrCore")
            .OptimizationLevel(OptimizationLevel.Fastest)
            .Depend(Visibility.Public, "SkrBase")
            .AddCppFiles("src/async/build.*.cpp");

        var SkrCore = Engine.Module("SkrCore")
            .Require("MiMalloc", new PackageConfig { Version = new Version(3, 1, 5) })
            .Depend(Visibility.Private, "MiMalloc@MiMalloc")

            .Depend(Visibility.Public, "SkrProfile")
            // TODO: STATIC COMPONENTS, JUST WORKAROUND NOW, WE NEED TO DEPEND THEM AUTOMATICALLY LATTER
            .Depend(Visibility.Public, "SkrDependencyGraph", "SkrSimpleAsync")
            // END WORKAROUNDS
            .IncludeDirs(Visibility.Public, "include")
            .Defines(Visibility.Private, "SKR_MEMORY_IMPL")
            // Core Files
            .AddCFiles("src/core/build.*.c")
            .AddCppFiles("src/core/build.*.cpp")
            // RTTR Files
            .AddCppFiles("src/rttr/build.*.cpp")
            // object Files
            .AddCppFiles("src/object/build.*.cpp")
            .Require("yyjson", new PackageConfig { Version = new Version(0, 12, 0) })
            .Depend(Visibility.Private, "yyjson@yyjson")
            // Codegen Files
            .AddMetaHeaders(
                "include/SkrRTTR/irttr_basic.hpp",
                "include/SkrRTTR/script/scriptble_object.hpp",
                "include/SkrRTTR/export/export_data.hpp",
                "include/SkrCore/cli.hpp",
                "include/SkrObject/**.hpp"
            )
            .AddCodegenScript("meta/basic.ts")
            .AddCodegenScript("meta/rttr.ts")
            .AddCodegenScript("meta/serialize.ts")
            .AddCodegenScript("meta/proxy.ts")
            .CreateSharedPCH("include/**.h", "include/**.hpp", "../profile/include/SkrProfile/profile.h");

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

public class SkrCoreSetup : ISetup
{
    public void Setup()
    {
        Install.SDK("SDL_3.2.12").Wait();
    }
}