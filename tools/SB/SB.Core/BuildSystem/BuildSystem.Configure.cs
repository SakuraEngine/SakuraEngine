
using SB.Core;
using Serilog;

namespace SB
{
    public partial class BuildSystem
    {
        public static void LoadConfigurations()
        {
            TargetDelegate Debug = (Target) =>
            {
                Target.DebugSymbols(true)
                    .OptimizationLevel(OptimizationLevel.None);
            };
            Configurations.Add("debug", Debug);

            TargetDelegate DebugRelease = (Target) =>
            {
                Target.DebugSymbols(true)
                    .OptimizationLevel(OptimizationLevel.Faster);
            };
            Configurations.Add("releasedbg", DebugRelease);

            TargetDelegate Release = (Target) =>
            {
                Target.DebugSymbols(false)
                    .OptimizationLevel(OptimizationLevel.Fastest);
            };
            Configurations.Add("release", Release);
        }

        public static void EnableAsan()
        {
            if (BuildSystem.TargetOS == OSPlatform.Windows)
            {
                BuildSystem.TargetDefaultSettings += (Target Target) =>
                {
                    Target.Link(Visibility.Private, "clang_rt.asan_dynamic-x86_64")
                        .Link(Visibility.Private, "clang_rt.asan_dynamic_runtime_thunk-x86_64")
                        .MSVC_CXFlags(Visibility.Private, "/fsanitize=address")
                        .Defines(Visibility.Public, "_DISABLE_VECTOR_ANNOTATION")
                        .Defines(Visibility.Public, "_DISABLE_STRING_ANNOTATION");
                };
            }
            else
            {
                Log.Warning("AddressSanitizer is not supported on this platform yet! Please use Clang on Linux or macOS to enable ASan.");
            }
        }

        public static Dictionary<string, TargetDelegate> Configurations = new();
        public static string GlobalConfiguration = "debug";
    }
}