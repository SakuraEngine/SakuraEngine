using SB.Core;
using Serilog;

namespace SB
{
    using BS = BuildSystem;

    public class CodegenRenderAttribute
    {
        public List<string> Scripts { get; } = new();
    }

    [Setup<CodegenSetup>]
    public class CodegenRenderEmitter : TaskEmitter
    {
        public CodegenRenderEmitter(IToolchain Toolchain) => this.Toolchain = Toolchain;
        public override bool EnableEmitter(Target Target) => Target.HasAttribute<CodegenRenderAttribute>() && Target.HasFilesOf<MetaHeaderList>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            var ModAttribute = Target.GetAttribute<ModuleAttribute>()!;
            var MetaAttribute = Target.GetAttribute<CodegenMetaAttribute>()!;
            var RenderAttribute = Target.GetAttribute<CodegenRenderAttribute>()!;

            var CodegenDirectory = Target.GetCodegenDirectory();
            var Config = new CodegenRenderConfig { 
                output_dir = CodegenDirectory,
                main_module = new CodegenModuleInfo { 
                    meta_dir = MetaAttribute.MetaDirectory!,
                    api = ModAttribute.API,
                    module_name = Target.Name
                }
            };
            // Collect include modules
            Target.Dependencies.Select(D => GetModAttr(D)).Where(D => D is not null).Select(D => D!).ToList()
                .ForEach(D => {
                    if (D!.Target.GetAttribute<CodegenMetaAttribute>()!.MetaDirectory is null)
                        return;
                        
                    Config.include_modules.Add(new CodegenModuleInfo {
                        meta_dir = D!.Target.GetAttribute<CodegenMetaAttribute>()!.MetaDirectory!,
                        api = D!.API,
                        module_name = D!.Target.Name
                    });
                });
            // Collect generators
            RenderAttribute.Scripts.Select(S => new CodegenGenerator { entry_file = S }).ToList()
                .ForEach(G => Config.generators.Add(G));
            Target.Dependencies.Select(D => GetGenAttr(D)).Where(D => D is not null).SelectMany(D => D!.Scripts).ToList()
                .ForEach(Script => {
                    Config.generators.Add(new CodegenGenerator {
                        entry_file = Script
                    });
                });
            // Collect depend files
            List<string> DependFiles = new();
            DependFiles.AddRange(Config.generators.Select(G => G.entry_file));
            if (MetaAttribute.AllGeneratedMetaFiles is not null)
                DependFiles.AddRange(MetaAttribute.AllGeneratedMetaFiles);
            // Execute
            bool Changed = EngineDepends.Codegen.OnChanged(Target.Name, "", this.Name, (Depend depend) => {
                Directory.Delete(CodegenDirectory, true);
                Directory.CreateDirectory(CodegenDirectory);

                var ParamFile = Path.Combine(CodegenDirectory, $"{Target.Name}_codegen_config.json");
                File.WriteAllText(ParamFile, Json.Serialize(Config));

                var EXE = Path.Combine(CodegenSetup.Installation!.Result, BS.HostOS == OSPlatform.Windows ? "bun.exe" : "bun");
                
                var ExitCode = BS.RunProcess(EXE, $"{GenerateScript} {ParamFile}", out var OutputInfo, out var ErrorInfo);
                if (ExitCode != 0)
                {
                    throw new TaskFatalError($"Codegen render {Target.Name} failed with fatal error!", $"bun.exe: {ErrorInfo}");
                }
                var AllGeneratedSources = Directory.GetFiles(CodegenDirectory, "*.cpp", SearchOption.AllDirectories);
                var AllGeneratedHeaders = Directory.GetFiles(CodegenDirectory, "*.h", SearchOption.AllDirectories);
                depend.ExternalFiles.AddRange(AllGeneratedSources);
                depend.ExternalFiles.AddRange(AllGeneratedHeaders);
            }, DependFiles, null);
            // Add generated files to target
            Target.AddCppFiles(Directory.GetFiles(CodegenDirectory, "*.cpp", SearchOption.AllDirectories));
            return new PlainArtifact { IsRestored = !Changed };
        }

        private static CodegenRenderAttribute? GetGenAttr(string TargetName) => BS.GetTarget(TargetName)?.GetAttribute<CodegenRenderAttribute>();
        private static ModuleAttribute? GetModAttr(string TargetName) => BS.GetTarget(TargetName)?.GetAttribute<ModuleAttribute>();

        // TODO: INSTALL DIRECTORY
        private static string GenerateScript = Path.Combine(BuildDirs.EngineDir, "engine/tools/meta_codegen_ts/codegen.ts");
        private IToolchain Toolchain { get; }
        public static volatile int Time = 0;
    }

    public struct CodegenModuleInfo
    {
        public string meta_dir { get; set; }
        public string api { get; set; }
        public string module_name { get; set; }
    }

    public class CodegenGenerator
    {
        public required string entry_file { get; set; }
        IReadOnlyList<string> import_dirs { get; set; } = new List<string>();
    }

    public class CodegenRenderConfig
    {
        public List<CodegenModuleInfo> include_modules { get; set;} = new();
        public CodegenModuleInfo main_module { get; set; }
        public required string output_dir { get; set; }
        public List<CodegenGenerator> generators { get; set; } = new();
        public int batch_size { get; init; } = 64;
    }

    public static partial class TargetExtensions
    {
        public static string GetCodegenDirectory(this Target @this) => Path.Combine(@this.GetBuildGenDir(), $"codegen/{@this.Name}");
    }

    public class CodegenSetup : ISetup
    {
        public void Setup()
        {
            Installation = Install.Tool("bun_1.2.5");
            Installation.Wait();
            var EXE = Path.Combine(Installation!.Result, BS.HostOS == OSPlatform.Windows ? "bun.exe" : "bun");

            ProcessOptions Options = new ProcessOptions
            {
                WorkingDirectory = Path.Combine(BuildDirs.EngineDir, "engine/tools/meta_codegen_ts"),
                EnableTimeout = true,
                TimeoutMilliseconds = 30 * 60 * 1000 // 30 minutes
            };
            if (BuildSystem.RunProcess(EXE, $"install", out var Output, out var Error, Options) != 0)
            {
                Log.Fatal("bun install failed!\n{Output}\n{Error}", Output, Error);
                throw new Exception($"bun install failed in meta_codegen_ts: {Error}");
            }

            Options.WorkingDirectory = Path.Combine(BuildDirs.EngineDir, "engine/tools/merge_natvis_ts");
            if (BuildSystem.RunProcess(EXE, $"install", out var Output2, out var Error2, Options) != 0)
            {
                Log.Fatal("bun install failed!\n{Output2}\n{Error2}", Output2, Error2);
                throw new Exception($"bun install failed in merge_natvis_ts: {Error2}");
            } 
        }
        public static Task<string>? Installation { get; set; }
    }
}