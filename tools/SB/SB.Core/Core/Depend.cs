using System.Security.Cryptography;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using System.Collections.Concurrent;
using Serilog;

namespace SB.Core
{
    using BS = BuildSystem;

    public struct DependOptions
    {
        public bool UseSHA { get; init; }
        public bool Force { get; init; }
    }

    public class DependDatabase
    {
        public bool OnChanged(string TargetName, string FileName, string EmitterName, Action<Depend> func, IEnumerable<string>? Files, IEnumerable<string>? Args, DependOptions? opt = null)
        {
            DependOptions option = opt ?? new DependOptions { Force = false, UseSHA = Depend.DefaultUseSHAInsteadOfDateTime };
            var SortedFiles = Files?.ToList() ?? new(); SortedFiles.Sort();
            var SortedArgs = Args?.ToList() ?? new(); SortedArgs.Sort();

            Depend? OldDepend = null;
            var CheckCtx = new CheckContext
            {
                TargetName = TargetName,
                FileName = FileName,
                EmitterName = EmitterName,
                SortedFiles = SortedFiles,
                SortedArgs = SortedArgs,
                opt = option
            };
            var NeedRerun = option.Force || !CheckDependency(ref CheckCtx, out OldDepend);
            if (NeedRerun)
            {
                Depend NewDepend = new Depend
                {
                    PrimaryKey = TargetName + FileName + EmitterName,
                    InputArgs = SortedArgs,
                    InputFiles = SortedFiles,
                    InputFileTimes = SortedFiles.Select(x => GetFileLastWriteTime(CacheMode.NoCache, x, option)).ToList(),
                    InputFileSHAs = SortedFiles.Select(x => GetFileSHA(CacheMode.NoCache, x, option)).ToList()
                };
                func(NewDepend);
                UpdateDependency(TargetName, NewDepend, OldDepend, option);
                return true;
            }
            return false;
        }

        public async Task<bool> OnChanged(string TargetName, string FileName, string EmitterName, Func<Depend, Task> func, IEnumerable<string>? Files, IEnumerable<string>? Args, DependOptions? opt = null)
        {
            DependOptions option = opt ?? new DependOptions { Force = false, UseSHA = Depend.DefaultUseSHAInsteadOfDateTime };
            var SortedFiles = Files?.ToList() ?? new(); SortedFiles.Sort();
            var SortedArgs = Args?.ToList() ?? new(); SortedArgs.Sort();

            Depend? OldDepend = null;
            var CheckCtx = new CheckContext
            {
                TargetName = TargetName,
                FileName = FileName,
                EmitterName = EmitterName,
                SortedFiles = SortedFiles,
                SortedArgs = SortedArgs,
                opt = option
            };
            var NeedRerun = option.Force || !CheckDependency(ref CheckCtx, out OldDepend);
            if (NeedRerun)
            {
                Depend NewDepend = new Depend
                {
                    PrimaryKey = TargetName + FileName + EmitterName,
                    InputArgs = SortedArgs,
                    InputFiles = SortedFiles,
                    InputFileTimes = SortedFiles.Select(x => GetFileLastWriteTime(CacheMode.NoCache, x, option)).ToList(),
                    InputFileSHAs = SortedFiles.Select(x => GetFileSHA(CacheMode.NoCache, x, option)).ToList()
                };
                await func(NewDepend);
                UpdateDependency(TargetName, NewDepend, OldDepend, option);
                return true;
            }
            return false;
        }

        private struct CheckContext
        {
            public required string TargetName;
            public required string FileName;
            public required string EmitterName;
            public required List<string> SortedFiles;
            public required List<string> SortedArgs;
            public required DependOptions opt;
        }

        private bool CheckDependency(ref CheckContext ctx, out Depend? OldDepend)
        {
            OldDepend = null;
            using (var DB = CreateContext(ctx.TargetName))
            {
                OldDepend = FromEntity(DB.Depends.Find(ctx.TargetName + ctx.FileName + ctx.EmitterName));
            }
            if (OldDepend is not null)
            {
                // check file list change
                if (!ctx.SortedFiles.SequenceEqual(OldDepend?.InputFiles!))
                {
                    Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: File list changed", ctx.TargetName, ctx.FileName, ctx.EmitterName);
                    return false;
                }
                // check arg list change
                if (!ctx.SortedArgs.SequenceEqual(OldDepend?.InputArgs!))
                {
                    Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Arg list changed", ctx.TargetName, ctx.FileName, ctx.EmitterName);
                    return false;
                }
                // check input file mtime change
                for (int i = 0; i < OldDepend?.InputFiles.Count; i++)
                {
                    var InputFile = OldDepend?.InputFiles[i];

                    if (!File.Exists(InputFile)) // deleted
                    {
                        Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Input file {InputFile} deleted", ctx.TargetName, ctx.FileName, ctx.EmitterName, InputFile);
                        return false;
                    }
                    if (ctx.opt.UseSHA == true)
                    {
                        string SHAString = GetFileSHA(CacheMode.Cache, InputFile!, ctx.opt);
                        if (OldDepend?.InputFileSHAs[i] != SHAString) // modified
                        {
                            Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Input file {InputFile} modified", ctx.TargetName, ctx.FileName, ctx.EmitterName, InputFile);
                            return false;
                        }
                    }
                    else
                    {
                        DateTime LastWriteTime = GetFileLastWriteTime(CacheMode.Cache, InputFile, ctx.opt);
                        if (OldDepend?.InputFileTimes[i] != LastWriteTime) // modified
                        {
                            Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Input file {InputFile} modified", ctx.TargetName, ctx.FileName, ctx.EmitterName, InputFile);
                            return false;
                        }
                    }
                }
                // check output file mtime change
                for (int i = 0; i < OldDepend?.ExternalFiles.Count; i++)
                {
                    var ExternalFile = OldDepend?.ExternalFiles[i];

                    if (ctx.opt.UseSHA == true)
                    {
                        string SHAString = GetFileSHA(CacheMode.Cache, ExternalFile!, ctx.opt);
                        if (OldDepend?.ExternalFileSHAs[i] != SHAString) // modified
                        {
                            Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Output file {OutputFile} modified", ctx.TargetName, ctx.FileName, ctx.EmitterName, ExternalFile);
                            return false;
                        }
                    }
                    else
                    {
                        DateTime LastWriteTime = GetFileLastWriteTime(CacheMode.Cache, ExternalFile!, ctx.opt);
                        if (LastWriteTime == DateTime.MinValue) // deleted
                        {
                            Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Output file {OutputFile} deleted", ctx.TargetName, ctx.FileName, ctx.EmitterName, ExternalFile);
                            return false;
                        }
                        if (OldDepend?.ExternalFileTimes[i] != LastWriteTime) // modified
                        {
                            Log.Verbose("Dependency changed for {TargetName} {FileName} {EmitterName}: Output file {OutputFile} modified", ctx.TargetName, ctx.FileName, ctx.EmitterName, ExternalFile);
                            return false;
                        }
                    }
                }
                return true;
            }
            Log.Verbose("Dependency not found for {TargetName} {FileName} {EmitterName}: No previous record", ctx.TargetName, ctx.FileName, ctx.EmitterName);
            return false;
        }

        private void UpdateDependency(string TargetName, Depend NewDepend, Depend? OldDepend, DependOptions opt)
        {
            NewDepend.ExternalFileTimes = NewDepend.ExternalFiles.Select(x => GetFileLastWriteTime(CacheMode.Cache, x, opt)).ToList();
            NewDepend.ExternalFileSHAs = NewDepend.ExternalFiles.Select(x => GetFileSHA(CacheMode.Cache, x, opt)).ToList();

            TaskFingerprint Fingerprint = new TaskFingerprint { TargetName = TargetName, File = NewDepend.PrimaryKey, TaskName = "UpdateDependency" };
            TaskManager.Run(Fingerprint, async () =>
            {
                using (Profiler.BeginZone($"WriteToDB", color: (uint)Profiler.ColorType.Gray))
                {
                    var DB = CreateContext(TargetName);
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
                InputFileSHAs = depend.InputFileSHAs,
                ExternalFiles = depend.ExternalFiles,
                ExternalFileSHAs = depend.ExternalFileSHAs,
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
                InputFileSHAs = entity.InputFileSHAs,
                ExternalFiles = entity.ExternalFiles,
                ExternalFileSHAs = entity.ExternalFileSHAs,
                ExternalFileTimes = entity.ExternalFileTimes
            };
        }

        private enum CacheMode
        {
            Cache,
            NoCache
        }

        private static string GetFileSHA(CacheMode CacheMode, string FilePath, DependOptions opt)
        {
            bool FileExist = CheckFileExist(CacheMode, FilePath);
            if (!FileExist || (opt.UseSHA != true))
                return String.Empty;

            string? SHAString = String.Empty;
            if (!cachedFileSHAs.TryGetValue(FilePath, out SHAString) || CacheMode == CacheMode.NoCache)
            {
                if (!OperatingSystem.IsWindows())
                {
                    var Mode = File.GetUnixFileMode(FilePath);
                    if (!Mode.HasFlag(UnixFileMode.UserRead))
                        File.SetUnixFileMode(FilePath, Mode | UnixFileMode.UserRead);
                }

                byte[] SHA = SHA256.HashData(File.ReadAllBytes(FilePath));
                SHAString = Convert.ToHexString(SHA).ToLowerInvariant();
                cachedFileSHAs[FilePath] = SHAString;
            }
            return SHAString;
        }

        private static DateTime GetFileLastWriteTime(CacheMode CacheMode, string FilePath, DependOptions opt)
        {
            bool FileExist = CheckFileExist(CacheMode, FilePath);
            if (!FileExist || (opt.UseSHA == true))
                return DateTime.MinValue; // Use SHA, so we don't care about last write time

            DateTime LastWriteTime = DateTime.MinValue;
            if (!cachedFileDateTimes.TryGetValue(FilePath, out LastWriteTime) || CacheMode == CacheMode.NoCache)
            {
                LastWriteTime = File.GetLastWriteTimeUtc(FilePath);
                cachedFileDateTimes[FilePath] = LastWriteTime;
            }
            return LastWriteTime;
        }

        private static bool CheckFileExist(CacheMode CacheMode, string Path)
        {
            bool Exist = false;
            if (CacheMode == CacheMode.NoCache || !cachedFileExists.TryGetValue(Path, out Exist))
            {
                Exist = File.Exists(Path);
                cachedFileExists[Path] = Exist;
            }
            return Exist;
        }
        private static ConcurrentDictionary<string, bool> cachedFileExists = new();
        private static ConcurrentDictionary<string, DateTime> cachedFileDateTimes = new();
        private static ConcurrentDictionary<string, string> cachedFileSHAs = new();

        public DependDatabase(string Location, string Name)
        {
            this.Name = Name;

            Factory = new(
                new DbContextOptionsBuilder<DependContext>()
                    .UseSqlite($"Data Source={Path.Join(Location, Name + ".db")}")
                    .UseQueryTrackingBehavior(QueryTrackingBehavior.NoTracking)
                    .Options
            );

            WarmUpContext = Factory.CreateDbContext();
            WarmUpContext!.Database.EnsureCreated();
            WarmUpContext!.FindAsync<DependEntity>("");
        }

        private DependContext CreateContext(string TargetName) => Factory.CreateDbContext();
        private string Name { get; init; } = "depend";
        private PooledDbContextFactory<DependContext> Factory;
        private DbContext? WarmUpContext;
    }

    public struct Depend
    {
        internal string PrimaryKey { get; set; } = "Invalid";
        internal List<string> InputArgs { get; init; } = new();
        internal List<string> InputFiles { get; init; } = new();
        internal List<DateTime> InputFileTimes { get; init; } = new();
        internal List<string> InputFileSHAs { get; init; } = new();
        internal List<DateTime> ExternalFileTimes { get; set; } = new();
        internal List<string> ExternalFileSHAs { get; set; } = new();
        public List<string> ExternalFiles { get; set; } = new();

        public Depend() { }
        public static bool DefaultUseSHAInsteadOfDateTime = false;
    }

    public class DependContext : DbContext
    {
        public DependContext(DbContextOptions<DependContext> options)
            : base(options)
        {

        }
        internal DbSet<DependEntity> Depends { get; set; }
    }

    [PrimaryKey(nameof(PrimaryKey))]
    internal class DependEntity
    {
        public string PrimaryKey { get; set; } = "Invalid";
        public List<string> InputArgs { get; init; } = new();
        public List<string> InputFiles { get; init; } = new();
        public List<DateTime> InputFileTimes { get; init; } = new();
        public List<string> InputFileSHAs { get; init; } = new();
        public List<string> ExternalFiles { get; init; } = new();
        public List<string> ExternalFileSHAs { get; init; } = new();
        public List<DateTime> ExternalFileTimes { get; init; } = new();
    }
}