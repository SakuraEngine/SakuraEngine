using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class NvStreamline
{
    static NvStreamline()
    {
        if (BuildSystem.TargetOS != OSPlatform.Windows)
            return;

        BuildSystem.Package("NvStreamline")
            .AddTarget("NvStreamline", (Target Target, PackageConfig Config) =>
            {
                BuildSystem.AddSetup<NvStreamlineSetup>();

                Target.TargetType(TargetType.HeaderOnly);
                if (Config.Version == new Version(2, 9, 0))
                {
                    Target.IncludeDirs(Visibility.Public, "2.9.0");
                }
                else
                {
                    throw new TaskFatalError("NvStreamline version mismatch!", "NvStreamline version mismatch, only v2.9.0 is supported in source.");
                }
            });
    }
}

public class NvStreamlineSetup : ISetup
{
    public void Setup()
    {
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            Task.WaitAll(
                Install.SDK("nv-streamline-v2.9.0")
            );
        }
    }
}