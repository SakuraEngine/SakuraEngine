namespace SB.Core
{
    public struct CompileResult : IArtifact
    {
        public string ObjectFile { get; init; }
        public string PDBFile { get; init; }
        public bool IsRestored { get; init; }
    }

    public interface ICompiler
    {
        public Version Version { get; }
        public string ExecutablePath { get; }
        public IArgumentDriver CreateArgumentDriver(CFamily Language, bool isPCH);
        public CompileResult Compile(TaskEmitter Emitter, Target Target, IArgumentDriver Driver, string? WorkDirectory = null);
    }
}
