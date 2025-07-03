using System.Diagnostics;
using System.Threading.Tasks.Schedulers;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Serilog;

namespace SB.Core
{
    using BS = BuildSystem;

    [Doctor<DependDbDoctor>]
    public class DependDbContext : DbContext
    {
        static DependDbContext()
        {
            WarmUpContext = PackagesFactory.CreateDbContext();
            WarmUpContext!.Database.EnsureCreated();

            WarmUpContext = ProjectFactory.CreateDbContext();
            WarmUpContext!.Database.EnsureCreated();
        }

        public DependDbContext(DbContextOptions<DependDbContext> options)
            : base(options)
        {

        }

        public static DependDbContext CreateContext(string TargetName)
            => (BS.GetTarget(TargetName)?.IsFromPackage == true) ? PackagesFactory.CreateDbContext() : ProjectFactory.CreateDbContext();

        public static PooledDbContextFactory<DependDbContext> ProjectFactory = new(
            new DbContextOptionsBuilder<DependDbContext>()
                .UseSqlite($"Data Source={Path.Join(BS.BuildPath, BS.GlobalConfiguration + "_depend.db")}")
                .UseQueryTrackingBehavior(QueryTrackingBehavior.NoTracking)
                .Options
        );

        public static PooledDbContextFactory<DependDbContext> PackagesFactory = new(
            new DbContextOptionsBuilder<DependDbContext>()
                .UseSqlite($"Data Source={Path.Join(BS.PackageBuildPath, BS.GlobalConfiguration + "_depend.db")}")
                .UseQueryTrackingBehavior(QueryTrackingBehavior.NoTracking)
                .Options
        );

        internal DbSet<DependEntity> Depends { get; set; }
        internal static DbContext? WarmUpContext;
    }

    public struct Depend
    {
        private string PrimaryKey { get; set; } = "Invalid";
        private List<string> InputArgs { get; init; } = new();
        private List<string> InputFiles { get; init; } = new();
        private List<DateTime> InputFileTimes { get; init; } = new();
        private List<DateTime> ExternalFileTimes { get; set; } = new();
        public List<string> ExternalFiles { get; set; } = new();

        public struct Options
        {
            public bool UseSHA { get; init; }
            public bool Force { get; init; }
        }

        public Depend() {}

        public static bool OnChanged(string TargetName, string FileName, string EmitterName, Action<Depend> func, IEnumerable<string>? Files, IEnumerable<string>? Args, Options? opt = null)
        {
            Options option = opt ?? new Options { Force = false, UseSHA = false };
            var SortedFiles = Files?.ToList() ?? new(); SortedFiles.Sort();
            var SortedArgs = Args?.ToList() ?? new(); SortedArgs.Sort();

            Depend? OldDepend = null;
            var NeedRerun = option.Force || !CheckDependency(TargetName, FileName, EmitterName, SortedFiles, SortedArgs, out OldDepend);
            if (NeedRerun)
            {
                Depend NewDepend = new Depend
                {
                    PrimaryKey = TargetName + FileName + EmitterName,
                    InputArgs = SortedArgs,
                    InputFiles = SortedFiles,
                    InputFileTimes = SortedFiles.Select(x => Directory.GetLastWriteTimeUtc(x)).ToList()
                };
                func(NewDepend);
                UpdateDependency(TargetName, NewDepend, OldDepend);
                return true;
            }
            return false;
        }

        public static async Task<bool> OnChanged(string TargetName, string FileName, string EmitterName, Func<Depend, Task> func, IEnumerable<string>? Files, IEnumerable<string>? Args, Options? opt = null)
        {
            Options option = opt ?? new Options { Force = false, UseSHA = false };
            var SortedFiles = Files?.ToList() ?? new(); SortedFiles.Sort();
            var SortedArgs = Args?.ToList() ?? new(); SortedArgs.Sort();

            Depend? OldDepend = null;
            var NeedRerun = option.Force || !CheckDependency(TargetName, FileName, EmitterName, SortedFiles, SortedArgs, out OldDepend);
            if (NeedRerun)
            {
                Depend NewDepend = new Depend
                {
                    PrimaryKey = TargetName + FileName + EmitterName,
                    InputArgs = SortedArgs,
                    InputFiles = SortedFiles,
                    InputFileTimes = SortedFiles.Select(x => Directory.GetLastWriteTimeUtc(x)).ToList()
                };
                await func(NewDepend);
                UpdateDependency(TargetName, NewDepend, OldDepend);
                return true;
            }
            return false;
        }

        private static bool CheckDependency(string TargetName, string FileName, string EmitterName, List<string> SortedFiles, List<string> SortedArgs, out Depend? OldDepend)
        {
            OldDepend = null;
            using (var DB = DependDbContext.CreateContext(TargetName))
            {
                OldDepend = FromEntity(DB.Depends.Find(TargetName + FileName + EmitterName));
            }
            if (OldDepend is not null)
            {
                // check file list change
                if (!SortedFiles.SequenceEqual(OldDepend?.InputFiles!))
                {
                    Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: File list changed", TargetName, FileName, EmitterName);
                    return false;
                }
                // check arg list change
                if (!SortedArgs.SequenceEqual(OldDepend?.InputArgs!))
                {
                    Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Arg list changed", TargetName, FileName, EmitterName);
                    return false;
                }
                // check input file mtime change
                for (int i = 0; i < OldDepend?.InputFiles.Count; i++)
                {
                    var InputFile = OldDepend?.InputFiles[i];
                    var DepTime = OldDepend?.InputFileTimes[i];

                    if (!File.Exists(InputFile)) // deleted
                    {
                        Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Input file {InputFile} deleted", TargetName, FileName, EmitterName, InputFile);
                        return false;
                    }
                    if (DepTime != Directory.GetLastWriteTimeUtc(InputFile)) // modified
                    {
                        Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Input file {InputFile} modified", TargetName, FileName, EmitterName, InputFile);
                        return false;
                    }
                }
                // check output file mtime change
                for (int i = 0; i < OldDepend?.ExternalFiles.Count; i++)
                {
                    var ExternalFile = OldDepend?.ExternalFiles[i];
                    var DepTime = OldDepend?.ExternalFileTimes[i];

                    DateTime LastWriteTime;
                    if (!BuildSystem.CachedFileExists(ExternalFile!, out LastWriteTime)) // deleted
                    {
                        Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Output file {OutputFile} deleted", TargetName, FileName, EmitterName, ExternalFile);
                        return false;
                    }
                    if (DepTime != LastWriteTime) // modified
                    {
                        Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Output file {OutputFile} modified", TargetName, FileName, EmitterName, ExternalFile);
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        private static void UpdateDependency(string TargetName, Depend NewDepend, Depend? OldDepend)
        {
            NewDepend.ExternalFileTimes = NewDepend.ExternalFiles.Select(x => Directory.GetLastWriteTimeUtc(x)).ToList();

            TaskFingerprint Fingerprint = new TaskFingerprint{ TargetName = TargetName, File = NewDepend.PrimaryKey, TaskName = "UpdateDependency"};
            TaskManager.Run(Fingerprint, async () => {
                using (Profiler.BeginZone($"WriteToDB", color: (uint)Profiler.ColorType.Gray))
                {
                    var DB = DependDbContext.CreateContext(TargetName);
                    {
                        if (OldDepend is not null)
                            DB.Depends.Update(ToEntity(NewDepend));
                        else
                            DB.Depends.Add(ToEntity(NewDepend));
                                
                        await DB.SaveChangesAsync();
                    }
                    return true;
                }
            }, TaskManager.IOQTS).GetAwaiter();
        }

        private static DependEntity ToEntity(Depend depend)
        {
            return new DependEntity
            {
                PrimaryKey = depend.PrimaryKey,
                InputArgs = depend.InputArgs,
                InputFiles = depend.InputFiles,
                InputFileTimes = depend.InputFileTimes,
                ExternalFiles = depend.ExternalFiles,
                ExternalFileTimes = depend.ExternalFileTimes
            };
        }

        private static Depend? FromEntity(DependEntity? entity)
        {
            if (entity is null)
                return null;

            return new Depend
            {
                PrimaryKey = entity.PrimaryKey,
                InputArgs = entity.InputArgs,
                InputFiles = entity.InputFiles,
                InputFileTimes = entity.InputFileTimes,
                ExternalFiles = entity.ExternalFiles,
                ExternalFileTimes = entity.ExternalFileTimes
            };
        }
    }

    [PrimaryKey(nameof(PrimaryKey))]
    internal class DependEntity
    {
        public string PrimaryKey { get; set; } = "Invalid";
        public List<string> InputArgs { get; init; } = new();
        public List<string> InputFiles { get; init; } = new();
        public List<DateTime> InputFileTimes { get; init; } = new();
        public List<string> ExternalFiles { get; init; } = new();
        public List<DateTime> ExternalFileTimes { get; init; } = new();
    }

    public class DependDbDoctor : IDoctor
    {
        public bool Check()
        {
            using (Profiler.BeginZone("WarmUp | EntityFramework", color: (uint)Profiler.ColorType.WebMaroon))
            {
                DependDbContext.WarmUpContext!.FindAsync<DependEntity>("");
                return true;
            }
        }

        public bool Fix()
        {
            return true;
        }
    }
}