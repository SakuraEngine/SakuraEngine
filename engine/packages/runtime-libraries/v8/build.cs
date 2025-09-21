using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class V8
{
    static V8()
    {
        Engine.AddSetup<V8Setup>();

        BuildSystem.Package("V8")
            .AddTarget("V8", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(1, 0, 0))
                    throw new TaskFatalError("V8 version mismatch!", "V8 version mismatch, only v1.0.0 is supported in source.");

                Target.TargetType(TargetType.HeaderOnly)
                    .Depend(Visibility.Public, "SkrRuntime")
                    .IncludeDirs(Visibility.Public, "include")
                    .Defines(Visibility.Public, "USING_V8_PLATFORM_SHARED", "USING_V8_SHARED");

                if (BuildSystem.TargetOS == OSPlatform.Windows)
                    Target.Link(Visibility.Public, "v8.dll", "v8_libplatform.dll");
                else if (BuildSystem.TargetOS == OSPlatform.OSX)
                    Target.Link(Visibility.Public, "v8", "v8_libbase", "v8_libplatform", "chrome_zlib", "third_party_abseil-cpp_absl");
            });
    }
}

public class V8Setup : ISetup
{
    public void Setup()
    {
        Install.SDK("v8_11.8.172").Wait();
    }
}