using System.Diagnostics;
using Microsoft.Extensions.FileSystemGlobbing;
using Serilog;

namespace SB.Core
{
    [XCodeDoctor]
    public partial class XCode : IToolchain
    {
        internal void FastPathFind()
        {
            HasCommandLineTools = Directory.Exists(CommandLineToolsDirectory);
            HasXCodeIDE = Directory.Exists(XCodeDirectory);
            if (!HasCommandLineTools && !HasXCodeIDE)
                return;
            var RootDirectoryToFind = HasCommandLineTools ? CommandLineToolsDirectory : XCodeDirectory;
            // find tools
            HasClang = File.Exists(ClangDirectory = Path.Combine(RootDirectoryToFind, "usr/bin/clang"));
            HasClangPP = File.Exists(ClangPPDirectory = Path.Combine(RootDirectoryToFind, "usr/bin/clang++"));
            HasAR = File.Exists(ARDirectory = Path.Combine(RootDirectoryToFind, "usr/bin/ar"));
            // find sdks
            var SDKs = Directory.GetDirectories(Path.Combine(RootDirectoryToFind, "SDKs"));
            var LatestVersion = SDKs.Select(P => Path.GetFileName(P).Replace("MacOSX", "").Replace(".sdk", "")).OrderByDescending(P => P).First();
            SDKVersion = Version.Parse(LatestVersion);
            PlatSDKDirectory = Path.Combine(RootDirectoryToFind, "SDKs", $"MacOSX{SDKVersion}.sdk");
        }

        internal void XCRunFind()
        {
            if (PlatSDKDirectory is null)
            {
                BuildSystem.RunProcess("xcrun", "-sdk macosx --show-sdk-path", out string? output, out string? error);
                PlatSDKDirectory = output.Trim();
            }
            if (SDKVersion is null)
            {
                BuildSystem.RunProcess("xcrun", "-sdk macosx --show-sdk-version", out string? output, out string? error);
                SDKVersion = Version.Parse(output);
            }
            if (!HasCommandLineTools && !HasXCodeIDE)
            {
                if (PlatSDKDirectory.Contains("Xcode.app"))
                {
                    XCodeDirectory = Directory.GetParent(PlatSDKDirectory)!.FullName;
                    HasXCodeIDE = true;
                }
                else
                {
                    CommandLineToolsDirectory = Directory.GetParent(PlatSDKDirectory)!.FullName;
                    HasCommandLineTools = true;
                }
                if (!HasCommandLineTools && !HasXCodeIDE)
                    throw new Exception("Failed to detect XCode or CommandLineTools!");
            }
            if (!HasClang)
            {
                BuildSystem.RunProcess("xcrun", "-sdk macosx --find clang", out string? output, out string? error);
                ClangDirectory = output.Trim();
                HasClang = true;
            }
            if (!HasClangPP)
            {
                BuildSystem.RunProcess("xcrun", "-sdk macosx --find clang++", out string? output, out string? error);
                ClangPPDirectory = output.Trim();
                HasClangPP = true;
            }
            if (!HasAR)
            {
                BuildSystem.RunProcess("xcrun", "-sdk macosx --find ar", out string? output, out string? error);
                ARDirectory = output.Trim();
                HasAR = true;
            }
        }

        internal void InitializeTools()
        {
            AppleClang = new AppleClangCompiler(ClangDirectory!, this);
            AppleClangPP = new AppleClangCompiler(ClangPPDirectory!, this);
            AR = new AR(ARDirectory!);
        }

        public string Name => "apple-clang";
        public Version Version => SDKVersion!;
        public ICompiler Compiler => AppleClang!;
        public IArchiver Archiver => AR!;
        public ILinker Linker => AppleClangPP!;
        public bool HasCommandLineTools { get; private set; } = false;
        public string CommandLineToolsDirectory { get; private set; } = "/Library/Developer/CommandLineTools/";
        public bool HasXCodeIDE { get; private set; } = false;
        public string XCodeDirectory { get; private set; } = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/";
        // Auto detect:
        // xcrun -sdk macosx --show-sdk-path
        // From XCode:
        // /Applications/Xcode*.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.*.sdk
        // /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX*.*.sdk
        // From CommandLineTools:
        // /Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
        internal string? PlatSDKDirectory { get; set; }
        public bool HasClang { get; private set; } = false;
        public string? ClangDirectory { get; private set; }
        public bool HasClangPP { get; private set; } = false;
        public AppleClangCompiler? AppleClang { get; private set; }
        public string? ClangPPDirectory { get; private set; }
        public AppleClangCompiler? AppleClangPP { get; private set; }
        public bool HasAR { get; private set; } = false;
        public string? ARDirectory { get; private set; }
        public AR? AR { get; private set; }
        internal Version? SDKVersion = null;
    }

    public class XCodeDoctor : DoctorAttribute
    {
        public override bool Check()
        {
            if (BuildSystem.HostOS == OSPlatform.OSX)
            {
                XCode.FastPathFind();
                XCode.XCRunFind();
                XCode.InitializeTools();
            }
            return true;
        }
        public override bool Fix()
        {
            Log.Fatal("XCode discover failed!");
            return true;
        }

        public static XCode XCode { get; private set; } = new XCode();
    }
}