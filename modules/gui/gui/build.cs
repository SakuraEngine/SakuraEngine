using SB;
using SB.Core;

[TargetScript]
public static class SkrGui
{
    static SkrGui()
    {
        Engine.Module("SkrGui")
            .EnableUnityBuild()
            .Require("freetype", new PackageConfig { Version = new Version(2, 13, 0) })
            .Require("icu", new PackageConfig { Version = new Version(72, 1, 0) })
            .Require("harfbuzz", new PackageConfig { Version = new Version(7, 1, 0) })
            .Require("nanovg", new PackageConfig { Version = new Version(0, 1, 0) })
            .Depend(Visibility.Public, "SkrRT")
            .Depend(Visibility.Private, "freetype@freetype", "icu@icu", "harfbuzz@harfbuzz", "nanovg@nanovg")
            .IncludeDirs(Visibility.Public, "include")
            .IncludeDirs(Visibility.Private, "src")
            .AddMetaHeaders("include/**.hpp")
            .AddCppFiles("src/*.cpp")
            .AddCppFiles("src/math/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "framework" }, "src/framework/**.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "render_objects" }, "src/render_objects/**.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "widgets" }, "src/widgets/**.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "dev" }, "src/dev/**.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "system" }, "src/system/**.cpp")

            .AddCppFiles("src/backend/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "text" }, "src/backend/paragraph/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "text" }, "src/backend/text_server/*.cpp")
            .AddCppFiles(new CFamilyFileOptions { UnityGroup = "text_adv" }, "src/backend/text_server_adv/*.cpp")

            .UsePrivatePCH("include/**.hpp");
    }
}