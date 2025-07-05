using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Zlib
{
    static Zlib()
    {
        BuildSystem
            .Package("zlib")
            .AvailableVersions(new Version(1, 2, 8))
            .AddTarget("zlib", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 2, 8))
                    throw new TaskFatalError("zlib version mismatch!", "zlib version mismatch, only v1.2.8 is supported in source.");

                Target
                    .TargetType(TargetType.HeaderOnly)
                    .IncludeDirs(Visibility.Public, "port/zlib/include");
                
                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    Target
                        .LinkDirs(Visibility.Public, "port/zlib/lib/windows/x64")
                        .Link(Visibility.Public, "zlibstatic");
                }
                else if (BuildSystem.TargetOS == OSPlatform.OSX)
                {
                    Target
                        .LinkDirs(Visibility.Public, "port/zlib/lib/macos/arm64")
                        .Link(Visibility.Public, "z", "zstd");
                }
                else
                {
                    throw new TaskFatalError("Unsupported OS", "zlib is not supported on this OS.");
                }
            });
    }
} 