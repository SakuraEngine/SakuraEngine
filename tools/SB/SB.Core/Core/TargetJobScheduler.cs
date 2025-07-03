using System.Threading.Channels;

namespace SB.Core
{
    public class HighPerformanceTaskScheduler : TaskScheduler, IDisposable
    {
        private readonly Channel<Task> _channel;
        private readonly ChannelWriter<Task> _writer;
        private readonly ChannelReader<Task> _reader;
        private readonly Task[] _workers;
        private readonly CancellationTokenSource _cancellation;
        private readonly int _maxConcurrency;

        public HighPerformanceTaskScheduler(int maxConcurrency)
        {
            _maxConcurrency = maxConcurrency;

            var options = new BoundedChannelOptions(10000)
            {
                FullMode = BoundedChannelFullMode.Wait,
                SingleReader = false,
                SingleWriter = false,
                AllowSynchronousContinuations = false
            };

            _channel = Channel.CreateBounded<Task>(options);
            _writer = _channel.Writer;
            _reader = _channel.Reader;
            _cancellation = new CancellationTokenSource();

            _workers = new Task[maxConcurrency];
            for (int i = 0; i < maxConcurrency; i++)
            {
                _workers[i] = Task.Run(WorkerLoop, _cancellation.Token);
            }
        }

        protected override void QueueTask(Task task)
        {
            if (_cancellation.Token.IsCancellationRequested)
                return;

            if (!_writer.TryWrite(task))
            {
                // 如果队列满了，异步写入
                _ = _writer.WriteAsync(task, _cancellation.Token);
            }
        }

        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
            // 如果任务之前已经入队，不允许内联执行
            if (taskWasPreviouslyQueued)
                return false;

            // 如果当前线程是我们的工作线程之一，允许内联执行
            if (Thread.CurrentThread.Name?.StartsWith("HPTaskScheduler") == true)
                return TryExecuteTask(task);

            return false;
        }

        protected override IEnumerable<Task> GetScheduledTasks()
        {
            // 返回空集合，因为Channel不提供枚举功能
            return Enumerable.Empty<Task>();
        }

        public override int MaximumConcurrencyLevel => _maxConcurrency;

        private async Task WorkerLoop()
        {
            Thread.CurrentThread.Name = $"HPTaskScheduler-{Thread.CurrentThread.ManagedThreadId}";

            try
            {
                await foreach (var task in _reader.ReadAllAsync(_cancellation.Token))
                {
                    TryExecuteTask(task);
                }
            }
            catch (OperationCanceledException) when (_cancellation.Token.IsCancellationRequested)
            {
                // 正常关闭
            }
        }

        public void Dispose()
        {
            _cancellation.Cancel();
            _writer.Complete();

            try
            {
                Task.WaitAll(_workers, TimeSpan.FromSeconds(5));
            }
            catch (AggregateException)
            {
                // 忽略取消异常
            }

            _cancellation.Dispose();
        }
    }
}