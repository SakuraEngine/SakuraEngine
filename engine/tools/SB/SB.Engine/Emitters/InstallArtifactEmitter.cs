using SB.Core;
using Serilog;
using System.IO;
using System.Linq;

namespace SB
{
    using BS = BuildSystem;

    /// <summary>
    /// Attribute to mark targets for artifact installation
    /// </summary>
    public class InstallArtifactAttribute
    {
        /// <summary>
        /// Destination directory for installation (absolute path or relative to project root)
        /// </summary>
        public string? InstallDirectory { get; set; }
        
        /// <summary>
        /// Whether to install PDB files alongside executables/DLLs
        /// </summary>
        public bool InstallPDB { get; set; } = true;
        
        /// <summary>
        /// Whether to install the artifact
        /// </summary>
        public bool Enable { get; set; } = true;
    }

    /// <summary>
    /// TaskEmitter for installing build artifacts (EXE, DLL, PDB) to specified directories
    /// </summary>
    public class InstallArtifactEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) 
        {
            var attr = Target.GetAttribute<InstallArtifactAttribute>();
            return attr != null && attr.Enable;
        }
        
        public override bool EmitTargetTask(Target Target) => true;
        
        public override IArtifact? PerTargetTask(Target Target)
        {
            var attr = Target.GetAttribute<InstallArtifactAttribute>();
            if (attr == null || !attr.Enable)
                return null;
            
            // Wait for link result
            var LinkResults = BS.Artifacts.OfType<LinkResult>()
                .Where(a => a.Target == Target)
                .ToList();
            
            if (!LinkResults.Any())
            {
                Log.Verbose("No link result found for target {TargetName}, skipping installation", Target.Name);
                return null;
            }
            
            // Determine installation directory
            string installDir;
            if (!string.IsNullOrEmpty(attr.InstallDirectory))
            {
                installDir = Path.IsPathFullyQualified(attr.InstallDirectory)
                    ? attr.InstallDirectory
                    : Path.Combine(BuildDirs.TempDir, attr.InstallDirectory);
            }
            else
            {
                // Default installation directory based on target category
                if (Target.IsCategory(TargetCategory.Tool))
                    installDir = Path.Combine(BuildDirs.TempDir, "tools");
                else
                    installDir = Path.Combine(BuildDirs.BuildDir, $"{BS.TargetOS}-{BS.TargetArch}-{BS.GlobalConfiguration}");
            }
            
            // Ensure installation directory exists
            Directory.CreateDirectory(installDir);

            SortedDictionary<string, string> FilesToCopy = new();
            var LinkResult = LinkResults.First();
            if (File.Exists(LinkResult.TargetFile))
            {
                var destinationFile = Path.Combine(installDir, Path.GetFileName(LinkResult.TargetFile));
                FilesToCopy.Add(LinkResult.TargetFile, destinationFile);
            }
            if (attr.InstallPDB && !string.IsNullOrEmpty(LinkResult.PDBFile) && File.Exists(LinkResult.PDBFile))
            {
                var destinationPDB = Path.Combine(installDir, Path.GetFileName(LinkResult.PDBFile));
                FilesToCopy.Add(LinkResult.PDBFile, destinationPDB);
            }

            bool Changed = BuildDepends.Solve(Target).OnChanged(Target.Name, "InstallArtifact", this.Name,
                (Depend depend) =>
                {
                    foreach (var FilePair in FilesToCopy)
                    {
                        File.Copy(FilePair.Key, FilePair.Value, overwrite: true);
                    }
                    depend.ExternalFiles.AddRange(FilesToCopy.Values);
                }, FilesToCopy.Keys, null);
            
            return new InstallResult
            {
                Target = Target,
                InstallDirectory = installDir,
                IsRestored = !Changed
            };
        }
    }
    
    /// <summary>
    /// Result of artifact installation
    /// </summary>
    public struct InstallResult : IArtifact
    {
        public required Target Target { get; init; }
        public required string InstallDirectory { get; init; }
        public bool IsRestored { get; init; }
    }
    
    /// <summary>
    /// Extension methods for configuring artifact installation
    /// </summary>
    public static partial class TargetExtensions
    {
        /// <summary>
        /// Configure the target to install its artifacts to a specific directory
        /// </summary>
        public static Target InstallArtifactTo(this Target @this, string installDirectory, bool installPDB = true)
        {
            @this.SetAttribute(new InstallArtifactAttribute
            {
                InstallDirectory = installDirectory,
                InstallPDB = installPDB,
                Enable = true
            });
            return @this;
        }
        
        /// <summary>
        /// Configure the target to install its artifacts using default directory based on category
        /// </summary>
        public static Target InstallArtifact(this Target @this, bool installPDB = true)
        {
            @this.SetAttribute(new InstallArtifactAttribute
            {
                InstallPDB = installPDB,
                Enable = true
            });
            return @this;
        }
        
        /// <summary>
        /// Disable artifact installation for this target
        /// </summary>
        public static Target NoInstallArtifact(this Target @this)
        {
            @this.SetAttribute(new InstallArtifactAttribute
            {
                Enable = false
            });
            return @this;
        }
    }
}