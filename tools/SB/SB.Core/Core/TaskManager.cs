using System.Collections.Concurrent;
using System.Threading.Tasks.Schedulers;

namespace SB.Core
{
    public struct TaskFingerprint
    {
        public string TargetName { get; init; }
        public string File { get; init; }
        public string TaskName { get; init; }
    }

    public class TaskFatalError : Exception
    {
        public TaskFatalError(string tidy, string what)
            : base(what)
        {
            Tidy = tidy;
        }
        public TaskFatalError(string what)
            : base(what)
        {
            Tidy = what;
        }
        public string Tidy { get; private set; }
    }

    public static class TaskManager
    {
        public static async Task Run(TaskFingerprint Fingerprint, Func<Task<bool>> Function, TaskScheduler? TS = null)
        {
            if (AllTasks.TryGetValue(Fingerprint, out var _))
                throw new ArgumentException($"Task with fingerprint {Fingerprint} already exists! Fingerprint should be unique to every task!");

            var NewTask = new Task<Task<bool>>(
                async () =>
                {
                    try
                    {
                        if (StopAll)
                            return await Task.FromResult(false);
                        return await Function();
                    }
                    catch (TaskFatalError fatal)
                    {
                        FatalErrors.Enqueue(fatal);
                        RootCTS.Cancel();
                        return await Task.FromResult(false);
                    }
                }, TaskCreationOptions.AttachedToParent);
            NewTask.Start(TS is not null ? TS : TaskScheduler.Default);

            if (!AllTasks.TryAdd(Fingerprint, await NewTask))
            {
                throw new ArgumentException($"Failed to add task with fingerprint {Fingerprint}! Are you adding same tasks in parallel?");
            }
        }

        public static void WaitAll(IEnumerable<Task> tasks)
        {
            Task.WaitAll(tasks);
        }

        public static void WaitAll()
        {
            Task.WaitAll(AllTasks.Values);
        }

        public static void ForceQuit()
        {
            StopAll = true;
            WaitAll(AllTasks.Values);
        }

        public static QueuedTaskScheduler BuildQTS = new(Environment.ProcessorCount, "BuildWorker", false, ThreadPriority.AboveNormal, ApartmentState.Unknown, 0);
        public static QueuedTaskScheduler IOQTS = new(1, "I/O Worker", false, ThreadPriority.BelowNormal, ApartmentState.Unknown, 0);

        internal static bool StopAll = false;
        internal static ConcurrentDictionary<TaskFingerprint, Task<bool>> AllTasks = new();
        internal static CancellationTokenSource RootCTS = new();
        internal static ConcurrentQueue<TaskFatalError> FatalErrors = new();
    }
}
