using SB.Core;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using Serilog;

namespace SB
{
    using BS = BuildSystem;
    
    /// <summary>
    /// TaskEmitter for copying files from source directories to build output directories
    /// </summary>
    public class CopyFilesEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => Target.HasFilesOf<CopyFileList>();
        
        public override bool EmitTargetTask(Target Target)
        {
            // Get all copy file lists for this target
            var CopyFileList = Target.FileList<CopyFileList>();
            var targetBuildPath = BuildDirs.BuildDir;
            var destinationBase = Path.Combine(targetBuildPath, CopyFileList.Destination ?? "");
            
            // Ensure destination directory exists
            Directory.CreateDirectory(destinationBase);
            
            foreach (var sourceFile in CopyFileList.Files)
            {
                // Use RootDir if specified, otherwise fall back to SourceBaseDirectory or Target.Directory
                var rootDir = CopyFileList.RootDir ?? CopyFileList.SourceBaseDirectory ?? Target.Directory;
                var relativePath = Path.GetRelativePath(rootDir, sourceFile);
                var destinationFile = Path.Combine(destinationBase, relativePath);
                
                // Ensure parent directory exists
                var destinationDir = Path.GetDirectoryName(destinationFile);
                if (destinationDir != null)
                {
                    Directory.CreateDirectory(destinationDir);
                }
                
                // Use dependency system to track changes
                EngineDepends.Misc.OnChanged(Target.Name, sourceFile, Name, (depend) =>
                {
                    Log.Verbose("Copying file {SourceFile} to {DestinationFile}", sourceFile, destinationFile);
                    File.Copy(sourceFile, destinationFile, overwrite: true);
                    depend.ExternalFiles.Add(destinationFile);
                }, new string[] { sourceFile }, new[] { destinationFile });
            }
            return true;
        }
    }
    
    /// <summary>
    /// FileList for files that need to be copied to the build output
    /// </summary>
    public class CopyFileList : FileList
    {
        /// <summary>
        /// Optional: Root directory for calculating relative paths. 
        /// This is used to determine the relative path structure that will be preserved in the destination.
        /// If not set, uses SourceBaseDirectory or Target.Directory
        /// </summary>
        public string? RootDir { get; set; }
        
        /// <summary>
        /// Optional: Base directory for calculating relative paths. 
        /// If not set, uses Target.Directory
        /// </summary>
        public string? SourceBaseDirectory { get; set; }
        
        /// <summary>
        /// Optional: Destination folder relative to build directory.
        /// If not set, copies to build directory root
        /// </summary>
        public string? Destination { get; set; }
    }
    
    /// <summary>
    /// Extension methods for easily adding copy tasks to targets
    /// </summary>
    public static partial class TargetExtensions
    {
        /// <summary>
        /// Add files to be copied to the build output directory
        /// </summary>
        /// <param name="this">Target instance</param>
        /// <param name="files">Files to copy (supports glob patterns)</param>
        /// <returns>Target instance for chaining</returns>
        public static Target CopyFiles(this Target @this, params string[] files)
        {
            @this.FileList<CopyFileList>().AddFiles(files);
            return @this;
        }
        
        /// <summary>
        /// Add files to be copied with custom destination
        /// </summary>
        /// <param name="this">Target instance</param>
        /// <param name="Destination">Destination folder relative to build directory</param>
        /// <param name="files">Files to copy (supports glob patterns)</param>
        /// <returns>Target instance for chaining</returns>
        public static Target CopyFilesTo(this Target @this, string Destination, params string[] files)
        {
            var fileList = @this.FileList<CopyFileList>();
            fileList.Destination = Destination;
            fileList.AddFiles(files);
            return @this;
        }
        
        /// <summary>
        /// Add files to be copied with custom root directory for relative path calculation
        /// </summary>
        /// <param name="this">Target instance</param>
        /// <param name="RootDir">Root directory for calculating relative paths that will be preserved in destination</param>
        /// <param name="Destination">Destination folder relative to build directory</param>
        /// <param name="files">Files to copy (supports glob patterns)</param>
        /// <returns>Target instance for chaining</returns>
        public static Target CopyFilesWithRoot(this Target @this, string RootDir, string Destination, params string[] files)
        {
            var fileList = @this.FileList<CopyFileList>();
            fileList.RootDir = Path.IsPathFullyQualified(RootDir) 
                ? RootDir
                : Path.Combine(@this.Directory, RootDir);
            fileList.Destination = Destination;
            fileList.AddFiles(files);
            return @this;
        }
    }
}