using SB.Core;
using System.Collections.Generic;

namespace SB
{
    public enum DependencyModel
    {
        PerTarget,
        ExternalTarget
    }

    public abstract class TaskEmitter
    {
        public TaskEmitter AddDependency(string EmitterName, DependencyModel Model)
        {
            Dependencies.Add(new KeyValuePair<string, DependencyModel>(EmitterName, Model));
            return this;
        }

        public virtual bool EnableEmitter(Target Target) => false;

        public virtual bool EmitTargetTask(Target Target) => false;
        public virtual IArtifact? PerTargetTask(Target Target) => null;

        public virtual bool EmitFileTask(Target Target, FileList FileList) => false;
        public virtual IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string File) => null;

        public string Name => SelfName;

        internal string SelfName = "";
        internal HashSet<KeyValuePair<string, DependencyModel>> Dependencies { get; } = new();
    }
}
