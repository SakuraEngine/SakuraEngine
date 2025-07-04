using SB.Core;

namespace SB
{
    using BS = BuildSystem;

    public class UnityBuildAttribute
    {
        public int BatchCount { get; set; } = 16;
        public required bool Cpp;
        public required bool C;
    }

    public class UnityBuildEmitter : TaskEmitter
    {
        public override bool EnableEmitter(Target Target) => (Target.HasFilesOf<CppFileList>() || Target.HasFilesOf<CFileList>() || Target.HasFilesOf<ObjCppFileList>() || Target.HasFilesOf<ObjCFileList>()) && Target.HasAttribute<UnityBuildAttribute>();
        public override bool EmitTargetTask(Target Target) => true;
        public override IArtifact? PerTargetTask(Target Target)
        {
            var UnityFileDirectory = Path.Combine(Target.GetStorePath(BS.GeneratedSourceStore), "unity_build");
            Directory.CreateDirectory(UnityFileDirectory);

            var UnityBuildAttribute = Target.GetAttribute<UnityBuildAttribute>()!;
            bool Changed = false;

            var RunBatch = (FileList FileList, string[] Batch, string BatchName, string Postfix) => {
                var UnityFile = Path.Combine(UnityFileDirectory, $"unity_build.{BatchName}.{Postfix}");
                Changed |= BS.CppCompileDepends(Target).OnChanged(Target.Name, UnityFile, this.Name, (Depend depend) => {
                    var UnityContent = String.Join("\n", Batch.Select(F => $"#include \"{F}\""));
                    File.WriteAllText(UnityFile, UnityContent);
                    depend.ExternalFiles.Add(UnityFile);
                }, Batch, null);
                FileList.RemoveFiles(Batch);
                return UnityFile;
            };

            var RunBatches = (FileList FileList, IEnumerable<string[]> Batches, string Postfix) => {
                List<string> UnityFiles = new(Batches.Count());
                int BatchIndex = 0;
                foreach (var Batch in Batches.ToArray())
                {
                    var UnityFile = RunBatch(FileList, Batch, BatchIndex.ToString(), Postfix);
                    UnityFiles.Add(UnityFile);
                    BatchIndex += 1;
                }
                return UnityFiles;
            };

            var Unity = (FileList FileList, string Lang) => {
                var FileGroups = FileList.Files.GroupBy(F => UnityGroup(F, FileList));
                foreach (var FileGroup in FileGroups)
                {
                    if (FileGroup.Key is null) 
                        continue; // Skip files with UnityGroup == null, which means unity build is disabled
                    else if (FileGroup.Key == "")
                    {
                        var CommonBatches = FileGroup.Chunk(UnityBuildAttribute.BatchCount);
                        var UnityFiles = RunBatches(FileList, CommonBatches, Lang);
                        FileList.AddFiles(UnityFiles.ToArray());
                    }
                    else
                    {
                        var UnityFile = RunBatch(FileList, FileGroup.ToArray(), FileGroup.Key, Lang);
                        FileList.AddFiles(UnityFile);
                    }
                }
            };

            if (UnityBuildAttribute.C && Target.HasFilesOf<CFileList>())
            {
                Unity(Target.FileList<CFileList>(), "c");
            }
            
            if (UnityBuildAttribute.Cpp && Target.HasFilesOf<CppFileList>())
            {
                Unity(Target.FileList<CppFileList>(), "cpp");
            }
            
            return new PlainArtifact { IsRestored = !Changed };
        }

        private static string? UnityGroup(string File, FileList FL)
        {
            var UnityOptions = FL.GetFileOptions(File) as UnityBuildableFileOptions;
            if (UnityOptions is null) 
                return "";
            if (UnityOptions.DisableUnityBuild) 
                return null;
            return UnityOptions.UnityGroup;
        }
    }

    public static partial class TargetExtensions
    {
        public static Target EnableUnityBuild(this Target @this, int BatchCount = 16, bool Cpp = true, bool C = true)
        {
            @this.SetAttribute(new UnityBuildAttribute { BatchCount = BatchCount, Cpp = Cpp, C = C });
            return @this;
        }
    }
}