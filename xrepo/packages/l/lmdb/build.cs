using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Lmdb
{
    static Lmdb()
    {
        BuildSystem
            .Package("lmdb")
            .AddTarget("lmdb", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 9, 29))
                    throw new TaskFatalError("lmdb version mismatch!", "lmdb version mismatch, only v0.9.29 is supported in source.");

                Target.CVersion("11")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .IncludeDirs(Visibility.Public, "port/lmdb/include")
                    .IncludeDirs(Visibility.Public, "port/lmdb/include/lmdb")
                    .AddCFiles("port/lmdb/*.c");
                
                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    Target.CFlags(Visibility.Private, "/wd4333", "/wd4172")
                        .ClangCl_CFlags(Visibility.Private, "-Wno-deprecated-declarations")
                        .Defines(Visibility.Private, "_CRT_SECURE_NO_WARNINGS");
                }
            });
    }
}