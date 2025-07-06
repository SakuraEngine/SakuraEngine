using System.Diagnostics;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Serilog;
using SB.Core;

namespace SB
{
    using BS = BuildSystem;
    
    [Doctor<TargetDbDoctor>]
    public class TargetDbContext : DbContext
    {
        static TargetDbContext()
        {
            UpdateContext = Factory.CreateDbContext();
            UpdateContext!.Database.EnsureDeleted();
            UpdateContext!.Database.EnsureCreated();
        }

        public TargetDbContext(DbContextOptions<TargetDbContext> options)
            : base(options)
        {

        }

        public static PooledDbContextFactory<TargetDbContext> Factory = new(
            new DbContextOptionsBuilder<TargetDbContext>()
                .UseSqlite($"Data Source={Path.Join(BS.TempPath, "targets.db")}")
                .UseQueryTrackingBehavior(QueryTrackingBehavior.NoTracking)
                .Options
        );
        internal DbSet<TargetEntity> Targets { get; set; }
        internal static TargetDbContext? UpdateContext;

    }
    
    public partial class BuildSystem
    {
        public static void UpdateTargetDatabase(ICollection<Target> targetsToBuild)
        {
            var targetsToUpdate = targetsToBuild != null
                ? AllTargets.Where(t => targetsToBuild.Contains(t.Value)).Select(t => t.Value)
                : AllTargets.Values;
                
            List<TargetEntity> TargetEntities = new (targetsToUpdate.Count());
            foreach (var Target in targetsToUpdate)
            {
                var FileLists = Target.FileLists.Select(FL => new KeyValuePair<string, IReadOnlySet<string>>(FL.GetType().Name, FL.Files)).ToDictionary();
                var Entity = new TargetEntity
                {
                    TargetName = Target.Name,
                    TargetType = (Target.Arguments["TargetType"] as SB.Core.TargetType?)?.ToString() ?? "null",
                    IsPackage = Target.IsFromPackage ? "true" : "false",
                    Category = Target.GetAttribute("TargetCategory")?.ToString() ?? "",
                    ScriptLocation = Target.Location,
                    LineNumber = Target.LineNumber,
                    FileLists = Json.Serialize(FileLists),
                    TargetArguments = Json.Serialize(Target.Arguments)
                };
                TargetEntities.Add(Entity);
            }

            TaskFingerprint Fingerprint = new TaskFingerprint {
                TargetName = "BuildSystem",
                File = "",
                TaskName = "UpdateTargetDatabase"
            };
            TaskManager.Run(Fingerprint, async () => {
                using (Profiler.BeginZone($"FlushTargetDatabase", color: (uint)Profiler.ColorType.Green))
                {
                    foreach (var Entity in TargetEntities)
                    {
                        if (TargetDbContext.UpdateContext!.Targets.Find(Entity.TargetName) is not null)
                            TargetDbContext.UpdateContext!.Targets.Update(Entity);
                        else
                                TargetDbContext.UpdateContext!.Targets.Add(Entity);
                    }
                    TargetDbContext.UpdateContext!.SaveChanges();
                }
                return await Task.FromResult(true);
            }, TaskManager.IOQTS).GetAwaiter();
        }
    }

    [PrimaryKey(nameof(TargetName))]
    internal class TargetEntity
    {
        public required string TargetName { get; set; }
        public required string TargetType { get; set; }
        public required string IsPackage { get; set; }
        public required string Category { get; set; }
        public required string ScriptLocation { get; set; }
        public required int LineNumber { get; set; }
        public required string FileLists { get; set; }
        public required string TargetArguments { get; set; }
    }

    public class TargetDbDoctor : IDoctor
    {
        public bool Check()
        {
            using (Profiler.BeginZone("WarmUp | EntityFramework", color: (uint)Profiler.ColorType.WebMaroon))
            {
                TargetDbContext.UpdateContext!.FindAsync<TargetEntity>("");
                return true;
            }
        }

        public bool Fix()
        {
            return true;
        }
    }
}