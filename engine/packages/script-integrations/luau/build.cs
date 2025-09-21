using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Luau
{
    static Luau()
    {
        BuildSystem.Package("Luau")
            .AddTarget("Common", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 613, 1))
                    throw new TaskFatalError("luau version mismatch!", "luau version mismatch, only v0.613.1 is supported in source.");

                Target.TargetType(TargetType.HeaderOnly)
                    .CppVersion("17")
                    .Exception(false)
                    .IncludeDirs(Visibility.Public, "luau/Common/include");
            })
            .AddTarget("VM", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 613, 1))
                    throw new TaskFatalError("luau version mismatch!", "luau version mismatch, only v0.613.1 is supported in source.");

                Target.CppVersion("17")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .Require("Luau", Config)
                    .Depend(Visibility.Public, "Luau@Common")
                    .Defines(Visibility.Public, "LUA_USE_LONGJMP=1", "LUA_API=extern\\\"C\\\"")
                    .IncludeDirs(Visibility.Public, "luau/VM/include")
                    .AddCppFiles("luau/VM/src/**.cpp")
                    .CppFlags(Visibility.Public, "-fno-math-errno");

                    if (BuildSystem.TargetOS == OSPlatform.Windows)
                    {
                        var Options = new CFamilyFileOptions();
                        Options.Arguments.CppFlags("/d2ssa-pre-");
                        
                        Target.AddCppFiles(Options, "luau/VM/src/lvmexecute.cc"); 
                    }
                    else
                    {
                        Target.AddCppFiles("luau/VM/src/lvmexecute.cc");
                    }
            })
            .AddTarget("Ast", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 613, 1))
                    throw new TaskFatalError("luau version mismatch!", "luau version mismatch, only v0.613.1 is supported in source.");

                Target.CppVersion("17")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .Require("Luau", Config)
                    .Depend(Visibility.Public, "Luau@Common")
                    .IncludeDirs(Visibility.Public, "luau/Ast/include")
                    .AddCppFiles("luau/Ast/src/**.cpp");
            })
            .AddTarget("Compiler", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(0, 613, 1))
                    throw new TaskFatalError("luau version mismatch!", "luau version mismatch, only v0.613.1 is supported in source.");
                
                Target.CppVersion("17")
                    .Exception(false)
                    .TargetType(TargetType.Static)
                    .Require("Luau", Config)
                    .Depend(Visibility.Public, "Luau@Ast")
                    .Defines(Visibility.Public, "LUACODE_API=extern\\\"C\\\"")
                    .IncludeDirs(Visibility.Public, "luau/Compiler/include")
                    .AddCppFiles("luau/Compiler/src/**.cpp");
            });
    }
}