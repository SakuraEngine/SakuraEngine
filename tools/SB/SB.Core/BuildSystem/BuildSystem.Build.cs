using SB.Core;
using Serilog;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace SB
{
    public partial class BuildSystem
    {
        public static Target Target(string Name, [CallerFilePath] string? Location = null, [CallerLineNumber] int LineNumber = 0)
        {
            if (AllTargets.TryGetValue(Name, out var Existed))
                throw new ArgumentException($"Target with name {Name} already exists! Name should be unique to every target!");

            var NewTarget = new Target(Name, false, Location!, LineNumber);
            if (!AllTargets.TryAdd(Name, NewTarget))
            {
                throw new ArgumentException($"Failed to add target with name {Name}! Are you adding same targets in parallel?");
            }
            return NewTarget;
        }

        public static Package Package(string Name)
        {
            if (AllPackages.TryGetValue(Name, out var _))
                throw new ArgumentException($"Package with name {Name} already exists! Name should be unique to every package!");

            var NewPackage = new Package(Name);
            if (!AllPackages.TryAdd(Name, NewPackage))
            {
                throw new ArgumentException($"Failed to add package with name {Name}! Are you adding same packages in parallel?");
            }
            return NewPackage;
        }

        public static Target? GetTarget(string Name) => AllTargets.TryGetValue(Name, out var Found) ? Found : null;
        public static Package GetPackage(string Name) => AllPackages.TryGetValue(Name, out var Found) ? Found : throw new ArgumentException($"Package with name {Name} not found!");

        public static TaskEmitter AddTaskEmitter(string Name, TaskEmitter Emitter)
        {
            if (TaskEmitters.TryGetValue(Name, out var _))
                throw new ArgumentException($"Emitter with name {Name} already exists! Name should be unique to every emitter!");
            TaskEmitters.Add(Name, Emitter);
            Emitter.SelfName = Name;
            return Emitter;
        }
        public static TaskEmitter? GetTaskEmitter(string Name) => TaskEmitters.TryGetValue(Name, out var Found) ? Found : null;

        public static void RunBuild(string? SingleTargetName = null)
        {
            try
            {
                Log.Verbose("Run Build... ");
                using (Profiler.BeginZone("RunBuild", color: (uint)Profiler.ColorType.WebPurple))
                {
                    RunBuildImpl(SingleTargetName);
                }
            }
            catch (OperationCanceledException)
            {
                TaskManager.ForceQuit();
            }
            finally
            {
                bool First = true;
                TaskFatalError? Fatal = null;
                while (TaskManager.FatalErrors.TryDequeue(out Fatal))
                {
                    if (First)
                    {
                        Log.Error("{FatalTidy} Detail:\n{FatalMessage}", Fatal.Tidy, Fatal.Message);
                        First = false;
                    }
                    else
                    {
                        Log.Error("{FatalTidy}", Fatal.Tidy);
                    }
                }
            }
        }

        private struct PlanKey
        {
            public required string Target { get; init; }
            public required string Emitter { get; init; }
        }

        private class TaskPlan
        {
            public required PlanKey Key { get; set; }
            public required Target Target { get; set; }
            public required TaskEmitter Emitter { get; set; }
            public HashSet<PlanKey> DirectDependencies { get; set; } = new();
        }

        private static TaskScheduler TQTS = TaskManager.BuildQTS.ActivateNewQueue(0);
        private static TaskScheduler FQTS = TaskManager.BuildQTS.ActivateNewQueue(1);

        private static void GlobDependencies(Target ToGlob, Dictionary<string, Target> TargetsToBuild)
        {
            var DirectDependencies = ToGlob!.Dependencies.Select(d => AllTargets[d]);
            foreach (var DirectDependency in DirectDependencies)
            {
                if (DirectDependency.Dependencies.Count > 0)
                {
                    GlobDependencies(DirectDependency, TargetsToBuild);
                }
                TargetsToBuild.TryAdd(DirectDependency.Name, DirectDependency);
            }
        }

        public static void RunBuildImpl(string? SingleTargetName = null)
        {
            Log.Verbose("Resolving Packages... ");
            using (Profiler.BeginZone($"ResolvePackages", color: (uint)Profiler.ColorType.Yellow))
            {
                Dictionary<string, Target> PackageTargets = new();
                foreach (var TargetKVP in AllTargets)
                    TargetKVP.Value.ResolvePackages(ref PackageTargets);
                AllTargets.AddRange(PackageTargets);
            }

            Log.Verbose("Resolving Dependencies... ");
            using (Profiler.BeginZone($"ResolveDependencies", color: (uint)Profiler.ColorType.Blue2))
            {
                foreach (var TargetKVP in AllTargets)
                    TargetKVP.Value.ResolveDependencies();
            }

            Dictionary<string, Target> TargetsToBuild = new();
            if (!String.IsNullOrEmpty(SingleTargetName))
            {
                var SingleTarget = GetTarget(SingleTargetName);
                GlobDependencies(SingleTarget!, TargetsToBuild);
                TargetsToBuild.Add(SingleTargetName, AllTargets[SingleTargetName]);
            }
            else
            {
                TargetsToBuild = AllTargets.ToDictionary();
            }

            using (Profiler.BeginZone($"CallAfterLoads", color: (uint)Profiler.ColorType.Green))
            {
                foreach (var TargetKVP in TargetsToBuild)
                    TargetKVP.Value.CallAllActions(TargetKVP.Value.AfterLoadActions);
            }

            Log.Verbose("Resolving Arguments... ");
            using (Profiler.BeginZone($"ResolveArguments", color: (uint)Profiler.ColorType.Pink))
            {
                Parallel.ForEach(TargetsToBuild.Values,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount, TaskScheduler = TQTS },
                Target =>
                {
                    using (Profiler.BeginZone($"ResolveArgument | {Target.Name}", color: (uint)Profiler.ColorType.Pink))
                    {
                        Target.ResolveArguments();
                    }
                });
            }

            List<Target> SortedTargets;
            Log.Verbose("Sorting... ");
            using (Profiler.BeginZone($"SortTargets", color: (uint)Profiler.ColorType.Purple))
            {
                SortedTargets = TargetsToBuild.Values.OrderBy(T => T.Dependencies.Count).ToList();
            }

            using (Profiler.BeginZone($"UpdateTargetDatabase", color: (uint)Profiler.ColorType.Brown))
            {
                // TODO: MOVE THIS TO SOMEWHERE ELES
                UpdateTargetDatabase(SortedTargets);
            }

            // == 第一阶段：构建发射器任务计划和依赖图 ==
            Dictionary<PlanKey, TaskPlan> AllTaskPlans;
            List<TaskPlan> ExecutionOrder;

            Log.Verbose("Planning... ");
            using (Profiler.BeginZone($"BuildEmitterPlans", color: (uint)Profiler.ColorType.Orange))
            {
                AllTaskPlans = BuildEmitterPlans(SortedTargets);
            }

            using (Profiler.BeginZone($"ResolvePlanDependencies", color: (uint)Profiler.ColorType.Cyan))
            {
                ResolvePlanDependencies(AllTaskPlans);
            }

            Log.Verbose("Topological Sorting... ");
            using (Profiler.BeginZone($"SortTaskPlans", color: (uint)Profiler.ColorType.Magenta))
            {
                ExecutionOrder = TopologicalSort(AllTaskPlans);
            }

            // == 第二阶段：按依赖顺序执行发射器任务，充分利用双调度器 ==
            using (Profiler.BeginZone($"ExecuteEmitterTasksWithSchedulers", color: (uint)Profiler.ColorType.Red))
            {
                ExecuteEmitterTasksWithSchedulers(ExecutionOrder);
            }

            TaskManager.WaitAll();
        }

        // 第一阶段：构建发射器任务计划（每个Target+Emitter组合一个）
        private static Dictionary<PlanKey, TaskPlan> BuildEmitterPlans(List<Target> SortedTargets)
        {
            var AllTaskPlans = new Dictionary<PlanKey, TaskPlan>();

            foreach (var Target in SortedTargets)
            {
                Target.CallAllActions(Target.BeforeBuildActions);

                foreach (var EmitterKVP in TaskEmitters)
                {
                    var EmitterName = EmitterKVP.Key;
                    var Emitter = EmitterKVP.Value;

                    if (!Emitter.EnableEmitter(Target))
                        continue;

                    var Plan = new TaskPlan
                    {
                        Key = new PlanKey
                        {
                            Target = Target.Name,
                            Emitter = EmitterName
                        },
                        Target = Target,
                        Emitter = Emitter
                    };
                    AllTaskPlans[Plan.Key] = Plan;
                }
            }

            return AllTaskPlans;
        }

        // 第一阶段：解析发射器任务间的依赖关系（与原始代码逻辑一致）
        private static void ResolvePlanDependencies(Dictionary<PlanKey, TaskPlan> AllTaskPlans)
        {
            foreach (var PlanKVP in AllTaskPlans)
            {
                var Plan = PlanKVP.Value;
                var PlanKey = PlanKVP.Key;
                var Target = Plan.Target;
                var Emitter = Plan.Emitter;

                // ExternalTarget依赖：依赖于其他目标的指定发射器
                // 对应原始代码中的AwaitExternalTargetDependencies
                foreach (var DepTarget in Target.Dependencies)
                {
                    foreach (var DepEmitter in Emitter.Dependencies.Where(KVP => KVP.Value.Equals(DependencyModel.ExternalTarget)))
                    {
                        var DepFingerprint = new PlanKey
                        {
                            Target = DepTarget,
                            Emitter = DepEmitter.Key
                        };

                        if (AllTaskPlans.ContainsKey(DepFingerprint))
                        {
                            Plan.DirectDependencies.Add(DepFingerprint);
                        }
                    }
                }

                // PerTarget依赖：依赖于同目标的其他发射器
                // 对应原始代码中的AwaitPerTargetDependencies
                foreach (var Dependency in Emitter.Dependencies.Where(KVP => KVP.Value.Equals(DependencyModel.PerTarget)))
                {
                    var DepFingerprint = new PlanKey
                    {
                        Target = Target.Name,
                        Emitter = Dependency.Key
                    };

                    if (AllTaskPlans.ContainsKey(DepFingerprint))
                    {
                        Plan.DirectDependencies.Add(DepFingerprint);
                    }
                }
            }
        }

        // 第一阶段：拓扑排序获得执行顺序
        private static List<TaskPlan> TopologicalSort(Dictionary<PlanKey, TaskPlan> AllTaskPlans)
        {
            var ExecutionOrder = new List<TaskPlan>();
            var InDegree = new Dictionary<PlanKey, int>();
            var Queue = new Queue<TaskPlan>();

            // 初始化入度为0
            foreach (var PlanKey in AllTaskPlans.Keys)
            {
                InDegree[PlanKey] = 0;
            }

            // 计算入度：如果Plan依赖于其他任务，则Plan的入度增加
            foreach (var PlanKVP in AllTaskPlans)
            {
                var PlanKey = PlanKVP.Key;
                var Plan = PlanKVP.Value;
                InDegree[PlanKey] = Plan.DirectDependencies.Count(dep => AllTaskPlans.ContainsKey(dep));
            }

            // 入度为0的任务入队
            foreach (var PlanKVP in AllTaskPlans)
            {
                if (InDegree[PlanKVP.Key] == 0)
                {
                    Queue.Enqueue(PlanKVP.Value);
                }
            }

            // 拓扑排序
            while (Queue.Count > 0)
            {
                var Plan = Queue.Dequeue();
                ExecutionOrder.Add(Plan);

                // 对于所有依赖于当前Plan的任务，减少其入度
                foreach (var OtherPlan in AllTaskPlans.Values)
                {
                    if (OtherPlan.DirectDependencies.Contains(Plan.Key))
                    {
                        InDegree[OtherPlan.Key]--;
                        if (InDegree[OtherPlan.Key] == 0)
                        {
                            Queue.Enqueue(OtherPlan);
                        }
                    }
                }
            }

            if (ExecutionOrder.Count != AllTaskPlans.Count)
            {
                throw new TaskFatalError("Circular dependency detected in task plans!");
            }

            return ExecutionOrder;
        }

        // 第二阶段：执行发射器任务，内部充分利用双调度器
        private static void ExecuteEmitterTasksWithSchedulers(List<TaskPlan> ExecutionOrder)
        {
            var RunningPlans = new ConcurrentDictionary<PlanKey, Task>();
            var AllTasks = new List<Task>();

            // 计算任务总数（用于进度显示）
            uint AllTaskCount = 0;
            uint FileTaskCount = 0;
            foreach (var Plan in ExecutionOrder)
            {
                if (Plan.Emitter.EmitTargetTask(Plan.Target))
                    AllTaskCount++;
                foreach (var FL in Plan.Target.FileLists.Where(FL => Plan.Emitter.EmitFileTask(Plan.Target, FL)))
                {
                    foreach (var File in FL.Files)
                    {
                        FileTaskCount++;
                        AllTaskCount++;
                    }
                }
            }

            _AllTaskCounter = 0;
            _FileTaskCounter = 0;

            // 启动所有发射器任务
            foreach (var Plan in ExecutionOrder)
            {
                var TaskExecution = Task.Run(async () =>
                {
                    // 等待所有直接依赖完成
                    foreach (var DepKey in Plan.DirectDependencies)
                    {
                        if (RunningPlans.TryGetValue(DepKey, out var DepTask))
                        {
                            await DepTask;
                        }
                    }

                    // 执行发射器任务，内部使用双调度器
                    await ExecuteEmitterTaskWithSchedulers(Plan, AllTaskCount, FileTaskCount);
                });

                RunningPlans[Plan.Key] = TaskExecution;
                AllTasks.Add(TaskExecution);
            }

            // 等待所有发射器任务完成
            Task.WaitAll(AllTasks.ToArray());
        }

        private static async Task ExecuteEmitterTaskWithSchedulers(TaskPlan Plan, uint AllTaskCount, uint FileTaskCount)
        {
            var Target = Plan.Target;
            var Emitter = Plan.Emitter;

            List<Task> FileTasks = new();
            Task PerTargetEmitterTask = Task.CompletedTask;

            // PerTarget任务 - 提交到TQTS调度器
            if (Emitter.EmitTargetTask(Target))
            {
                PerTargetEmitterTask = TaskManager.Run(
                    new TaskFingerprint { TargetName = Target.Name, File = "", TaskName = Emitter.Name },
                    async () =>
                    {
                        var TaskIndex = Interlocked.Increment(ref _AllTaskCounter);
                        var Percentage = 100.0f * TaskIndex / AllTaskCount;

                        using (Profiler.BeginZone($"{Emitter.Name} | {Target.Name}", color: (uint)Profiler.ColorType.Green1))
                        {
                            Stopwatch sw = new();
                            sw.Start();
                            var TargetTaskArtifact = Emitter.PerTargetTask(Target);
                            sw.Stop();

                            // Log.Verbose("[{Percentage:00.0}%] {EmitterName} {TargetName}", Percentage, Emitter.Name, Target.Name);
                            if (TargetTaskArtifact is not null)
                            {
                                Artifacts.Add(TargetTaskArtifact);
                                if (!TargetTaskArtifact.IsRestored)
                                {
                                    var CostTime = sw.ElapsedMilliseconds;
                                    Log.Information("[{Percentage:00.0}%]: {EmitterName} {TargetName}, cost {CostTime:00.00}s",
                                        Percentage, Emitter.Name, Target.Name, CostTime / 1000.0f);
                                }
                            }
                        }
                        return await Task.FromResult(true);
                    }, TQTS);
            }

            await PerTargetEmitterTask;

            foreach (var FL in Target.FileLists.ToArray().Where(FL => Emitter.EmitFileTask(Target, FL)))
            {
                var FilesCopy = FL.Files.ToArray();
                foreach (var File in FilesCopy)
                {
                    // 为每个文件创建一个任务，提交到FQTS调度器
                    var FileTask = TaskManager.Run(
                        new TaskFingerprint { TargetName = Target.Name, File = File, TaskName = Emitter.Name },
                        async () =>
                        {
                            var FileTaskIndex = Interlocked.Increment(ref _FileTaskCounter);
                            var TaskIndex = Interlocked.Increment(ref _AllTaskCounter);
                            var Percentage = 100.0f * TaskIndex / AllTaskCount;

                            using (Profiler.BeginZone($"{Emitter.Name} | {Target.Name} | {File}", color: (uint)Profiler.ColorType.Yellow1))
                            {
                                Stopwatch sw = new();
                                sw.Start();
                                var FileTaskArtifact = Emitter.PerFileTask(Target, FL, FL.GetFileOptions(File), File);
                                sw.Stop();

                                // Log.Verbose("[{Percentage:00.0}%][{FileTaskIndex}/{FileTaskCount}]: {EmitterName} {TargetName}: {FileName}", Percentage, FileTaskIndex, FileTaskCount, Emitter.Name, Target.Name, File);
                                if (FileTaskArtifact is not null)
                                {
                                    Artifacts.Add(FileTaskArtifact);
                                    if (!FileTaskArtifact.IsRestored)
                                    {
                                        var CostTime = sw.ElapsedMilliseconds;
                                        Log.Information("[{Percentage:00.0}%][{FileTaskIndex}/{FileTaskCount}]: {EmitterName} {TargetName}: {FileName}, cost {CostTime:00.00}s",
                                            Percentage, FileTaskIndex, FileTaskCount, Emitter.Name, Target.Name, File, CostTime / 1000.0f);
                                    }
                                }
                            }
                            return await Task.FromResult(true);
                        }, FQTS);

                    FileTasks.Add(FileTask);
                }
            }

            // 等待所有子任务完成
            await PerTargetEmitterTask;
            await Task.WhenAll(FileTasks);
        }

        private static uint _AllTaskCounter = 0;
        private static uint _FileTaskCounter = 0;

        public static ConcurrentBag<IArtifact> Artifacts = new();
        private static Dictionary<string, TaskEmitter> TaskEmitters = new();
        public static Dictionary<string, Target> AllTargets { get; } = new();
        protected static Dictionary<string, Package> AllPackages { get; } = new();
        internal static Dictionary<string, Target> _AllTargets => AllTargets;
        public delegate void TargetDelegate(Target Target);
        public static TargetDelegate TargetDefaultSettings = new(Target =>
        {
            Target.SetAttribute(new CppCompileAttribute());
            Target.SetAttribute(new CppLinkAttribute());
        });
        public static DependDatabase CppCompileDepends(Target Target) => Target.IsFromPackage ? pkgCompileDepends.Value : targetCompileDepends.Value;
        public static Lazy<DependDatabase> pkgCompileDepends = new Lazy<DependDatabase>(
            () => new DependDatabase(PackageBuildPath!, "CppCompile.Paks." + BuildSystem.GlobalConfiguration)
        );
        public static Lazy<DependDatabase> targetCompileDepends = new Lazy<DependDatabase>(
            () => new DependDatabase(BuildPath!, "CppCompile.Targets." + BuildSystem.GlobalConfiguration)
        );
    }

    internal static class TargetTaskExtensions
    {
        public static void CallAllActions(this Target Target, IList<Action<Target>> Actions)
        {
            foreach (var Action in Actions)
            {
                using (Profiler.BeginZone($"Action | {Target.Name}", color: (uint)Profiler.ColorType.Green)) 
                {
                    Action(Target);
                }
            }
        }
    }
}