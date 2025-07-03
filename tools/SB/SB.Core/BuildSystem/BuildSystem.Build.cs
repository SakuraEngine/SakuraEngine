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

        public static void RunBuild()
        {
            try
            {
                Log.Verbose("Run Build... ");
                using (Profiler.BeginZone("RunBuild", color: (uint)Profiler.ColorType.WebPurple))
                {
                    var RunBuildTask = new Task(() => { 
                        using (Profiler.BeginZone("RunBuild", color: (uint)Profiler.ColorType.WebPurple))
                        {
                            RunBuildImpl();
                        }
                    }, TaskManager.RootCTS.Token);
                    RunBuildTask.Start(TaskManager.SchedulerTS);
                    RunBuildTask.Wait(TaskManager.RootCTS.Token);
                }
            }
            catch (OperationCanceledException)
            {
                TaskManager.ForceQuit();

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

        // 任务计划结构
        private class TaskPlan
        {
            public TaskFingerprint Fingerprint { get; set; }
            public Target Target { get; set; }
            public TaskEmitter Emitter { get; set; }
            public HashSet<TaskFingerprint> DirectDependencies { get; set; } = new();
        }

        private static TaskScheduler TQTS = TaskManager.BuildQTS.ActivateNewQueue(0);
        private static TaskScheduler FQTS = TaskManager.BuildQTS.ActivateNewQueue(1);
        
        public static void RunBuildImpl()
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

            using (Profiler.BeginZone($"CallAfterLoads", color: (uint)Profiler.ColorType.Green))
            {
                foreach (var TargetKVP in AllTargets)
                    TargetKVP.Value.CallAllActions(TargetKVP.Value.AfterLoadActions);
            }

            Log.Verbose("Resolving Arguments... ");
            using (Profiler.BeginZone($"ResolveArguments", color: (uint)Profiler.ColorType.Pink))
            {
                Parallel.ForEach(AllTargets.Values,
                new ParallelOptions { TaskScheduler = TQTS },
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
                SortedTargets = AllTargets.Values.OrderBy(T => T.Dependencies.Count).ToList();
            }

            using (Profiler.BeginZone($"UpdateTargetDatabase", color: (uint)Profiler.ColorType.Brown))
            {
                // TODO: MOVE THIS TO SOMEWHERE ELES
                UpdateTargetDatabase();
            }

            // == 第一阶段：构建发射器任务计划和依赖图 ==
            Dictionary<TaskFingerprint, TaskPlan> AllTaskPlans;
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
        private static Dictionary<TaskFingerprint, TaskPlan> BuildEmitterPlans(List<Target> SortedTargets)
        {
            var AllTaskPlans = new Dictionary<TaskFingerprint, TaskPlan>();

            foreach (var Target in SortedTargets)
            {
                Target.CallAllActions(Target.BeforeBuildActions);

                foreach (var EmitterKVP in TaskEmitters)
                {
                    var EmitterName = EmitterKVP.Key;
                    var Emitter = EmitterKVP.Value;

                    if (!Emitter.EnableEmitter(Target))
                        continue;

                    // 发射器任务计划 - 与原始代码的EmitterTask对应
                    // 依赖关系都是在这个级别建立的，File字段为空字符串
                    var Fingerprint = new TaskFingerprint
                    {
                        TargetName = Target.Name,
                        File = "", // 发射器任务级别的依赖都是基于File=""
                        TaskName = EmitterName
                    };

                    var Plan = new TaskPlan
                    {
                        Fingerprint = Fingerprint,
                        Target = Target,
                        Emitter = Emitter
                    };

                    AllTaskPlans[Fingerprint] = Plan;
                }
            }

            return AllTaskPlans;
        }

        // 第一阶段：解析发射器任务间的依赖关系（与原始代码逻辑一致）
        private static void ResolvePlanDependencies(Dictionary<TaskFingerprint, TaskPlan> AllTaskPlans)
        {
            foreach (var PlanKVP in AllTaskPlans)
            {
                var Plan = PlanKVP.Value;
                var Target = Plan.Target;
                var Emitter = Plan.Emitter;

                // ExternalTarget依赖：依赖于其他目标的指定发射器
                // 对应原始代码中的AwaitExternalTargetDependencies
                foreach (var DepTarget in Target.Dependencies)
                {
                    foreach (var DepEmitter in Emitter.Dependencies.Where(KVP => KVP.Value.Equals(DependencyModel.ExternalTarget)))
                    {
                        var DepFingerprint = new TaskFingerprint
                        {
                            TargetName = DepTarget,
                            File = "", // 原始代码中依赖的是发射器任务级别
                            TaskName = DepEmitter.Key
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
                    var DepFingerprint = new TaskFingerprint
                    {
                        TargetName = Target.Name,
                        File = "", // 原始代码中依赖的是发射器任务级别
                        TaskName = Dependency.Key
                    };

                    if (AllTaskPlans.ContainsKey(DepFingerprint))
                    {
                        Plan.DirectDependencies.Add(DepFingerprint);
                    }
                }

                // PerFile依赖：提升为发射器级别依赖以简化实现
                // 这确保了依赖关系的正确性，虽然可能降低一些并行度
                foreach (var Dependency in Emitter.Dependencies.Where(KVP => KVP.Value.Equals(DependencyModel.PerFile)))
                {
                    var DepFingerprint = new TaskFingerprint
                    {
                        TargetName = Target.Name,
                        File = "", // 提升为发射器级别依赖
                        TaskName = Dependency.Key
                    };

                    if (AllTaskPlans.ContainsKey(DepFingerprint))
                    {
                        Plan.DirectDependencies.Add(DepFingerprint);
                    }
                }
            }
        }

        // 第一阶段：拓扑排序获得执行顺序
        private static List<TaskPlan> TopologicalSort(Dictionary<TaskFingerprint, TaskPlan> AllTaskPlans)
        {
            var ExecutionOrder = new List<TaskPlan>();
            var InDegree = new Dictionary<TaskFingerprint, int>();
            var Queue = new Queue<TaskPlan>();

            // 初始化入度为0
            foreach (var Plan in AllTaskPlans.Values)
            {
                InDegree[Plan.Fingerprint] = 0;
            }

            // 计算入度：如果Plan依赖于其他任务，则Plan的入度增加
            foreach (var Plan in AllTaskPlans.Values)
            {
                InDegree[Plan.Fingerprint] = Plan.DirectDependencies.Count(dep => AllTaskPlans.ContainsKey(dep));
            }

            // 入度为0的任务入队
            foreach (var Plan in AllTaskPlans.Values)
            {
                if (InDegree[Plan.Fingerprint] == 0)
                {
                    Queue.Enqueue(Plan);
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
                    if (OtherPlan.DirectDependencies.Contains(Plan.Fingerprint))
                    {
                        InDegree[OtherPlan.Fingerprint]--;
                        if (InDegree[OtherPlan.Fingerprint] == 0)
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
            var RunningTasks = new ConcurrentDictionary<TaskFingerprint, Task>();
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
                    foreach (var DepFingerprint in Plan.DirectDependencies)
                    {
                        if (RunningTasks.TryGetValue(DepFingerprint, out var DepTask))
                        {
                            await DepTask;
                        }
                    }

                    // 执行发射器任务，内部使用双调度器
                    await ExecuteEmitterTaskWithSchedulers(Plan, AllTaskCount, FileTaskCount);
                });

                RunningTasks[Plan.Fingerprint] = TaskExecution;
                AllTasks.Add(TaskExecution);
            }

            // 等待所有发射器任务完成
            Task.WaitAll(AllTasks.ToArray());
        }

        private static async Task ExecuteEmitterTaskWithSchedulers(TaskPlan Plan, uint AllTaskCount, uint FileTaskCount)
        {
            var Target = Plan.Target;
            var Emitter = Plan.Emitter;

            // 发射器任务内部逻辑，模拟原始代码但使用双调度器优化
            List<Task> FileTasks = new();
            Task PerTargetEmitterTask = Task.CompletedTask;

            // PerTarget任务 - 提交到TQTS调度器
            if (Emitter.EmitTargetTask(Target))
            {
                PerTargetEmitterTask = Task.Factory.StartNew(() =>
                {
                    var TaskIndex = Interlocked.Increment(ref _AllTaskCounter);
                    var Percentage = 100.0f * TaskIndex / AllTaskCount;

                    using (Profiler.BeginZone($"{Emitter.Name} | {Target.Name}", color: (uint)Profiler.ColorType.Green1))
                    {
                        Stopwatch sw = new();
                        sw.Start();
                        var TargetTaskArtifact = Emitter.PerTargetTask(Target);
                        sw.Stop();

                        Log.Verbose("[{Percentage:00.0}%] {EmitterName} {TargetName}", Percentage, Emitter.Name, Target.Name);
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
                }, CancellationToken.None, TaskCreationOptions.None, TQTS);
            }

            // 处理PerFile任务 - 提交到FQTS调度器
            foreach (var FL in Target.FileLists.ToArray().Where(FL => Emitter.EmitFileTask(Target, FL)))
            {
                foreach (var File in FL.Files)
                {
                    // 为每个文件创建一个任务，提交到FQTS调度器
                    var FileTask = Task.Factory.StartNew(async () =>
                    {
                        // 等待PerFile依赖（在文件任务内部处理）
                        await WaitForPerFileDependencies(Emitter, Target, File);

                        // 等待PerTarget任务完成（如果有的话）
                        await PerTargetEmitterTask;

                        var FileTaskIndex = Interlocked.Increment(ref _FileTaskCounter);
                        var TaskIndex = Interlocked.Increment(ref _AllTaskCounter);
                        var Percentage = 100.0f * TaskIndex / AllTaskCount;

                        using (Profiler.BeginZone($"{Emitter.Name} | {Target.Name} | {File}", color: (uint)Profiler.ColorType.Yellow1))
                        {
                            Stopwatch sw = new();
                            sw.Start();
                            var FileTaskArtifact = Emitter.PerFileTask(Target, FL, FL.GetFileOptions(File), File);
                            sw.Stop();

                            Log.Verbose("[{Percentage:00.0}%][{FileTaskIndex}/{FileTaskCount}]: {EmitterName} {TargetName}: {FileName}", 
                                      Percentage, FileTaskIndex, FileTaskCount, Emitter.Name, Target.Name, File);
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
                    }, CancellationToken.None, TaskCreationOptions.None, FQTS).Unwrap();
                    
                    FileTasks.Add(FileTask);
                }
            }

            // 等待所有子任务完成
            await PerTargetEmitterTask;
            await Task.WhenAll(FileTasks);
        }

        // 简化的PerFile依赖处理（依赖关系已在发射器级别处理）
        private static async Task WaitForPerFileDependencies(TaskEmitter Emitter, Target Target, string File)
        {
            // 由于PerFile依赖已经提升为发射器级别依赖，
            // 这里不需要额外的等待逻辑
            await Task.CompletedTask;
        }

        private static uint _AllTaskCounter = 0;
        private static uint _FileTaskCounter = 0;

        public static ConcurrentBag<IArtifact> Artifacts = new();
        private static Dictionary<string, TaskEmitter> TaskEmitters = new();
        public static Dictionary<string, Target> AllTargets { get; } = new();
        protected static Dictionary<string, Package> AllPackages { get; } = new();
        internal static Dictionary<string, Target> _AllTargets => AllTargets;
        public delegate void TargetDelegate(Target Target);
        public static TargetDelegate TargetDefaultSettings = new(Target => {
            Target.SetAttribute(new CppCompileAttribute());
            Target.SetAttribute(new CppLinkAttribute());
        });
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