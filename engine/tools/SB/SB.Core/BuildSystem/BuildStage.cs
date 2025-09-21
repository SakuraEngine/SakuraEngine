using System.Reflection;

namespace SB;

public enum EBuildStage
{
    Init,
    /// <summary>
    /// 设置工作路径后调用
    /// </summary>
    SetupPaths,
    /// <summary>
    /// 构建流程中最早的阶段，开始调用 CommandLine 前调用
    /// </summary>
    PrepareCommandline,
    /// <summary>
    /// 命令行配置完 configure 后调用
    /// </summary>
    SetupConfigure,
    /// <summary>
    /// 命令行配置完 toolchain 后调用
    /// </summary>
    LoadedToolChain,
    /// <summary>
    /// 构建完毕后调用
    /// </summary>
    Build,
};

[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
public class AfterStageAttribute : Attribute
{
    public EBuildStage Stage { get; private set; }
    public int Order { get; private set; }
    public AfterStageAttribute(EBuildStage stage, int order = 0)
    {
        Stage = stage;
        Order = order;
    }
}

public delegate void BuildStageDelegate();

public static class BuildStage
{
    public struct StageInfo
    {
        public BuildStageDelegate Callback;
        public int Order;
    }

    private static EBuildStage _CurrentStage = EBuildStage.Init;
    private static Dictionary<EBuildStage, List<StageInfo>> _AfterStageCallbacks = new();

    static BuildStage()
    {
        // collect stage methods from attributes
        foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            foreach (var type in assembly.GetTypes())
            {
                foreach (var method in type.GetMethods(BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic))
                {
                    var attrs = method.GetCustomAttributes<AfterStageAttribute>();
                    foreach (var attr in attrs)
                    {
                        if (method.GetParameters().Length == 0 && method.ReturnType == typeof(void))
                        {
                            // Console.WriteLine($"Register BuildStage method: {type.FullName}.{method.Name} for stage {attr.Stage} order {attr.Order}");
                            AfterStage(attr.Stage, () => method.Invoke(null, null), attr.Order);
                        }
                        else
                        {
                            throw new InvalidOperationException($"BuildStage method {type.FullName}.{method.Name} must have no parameters and return void");
                        }
                    }
                }
            }
        }
    }

    // register
    public static void AfterStage(EBuildStage stage, BuildStageDelegate callback, int order = 0)
    {
        // call immediately if already passed this stage
        if (stage <= _CurrentStage)
        {
            callback();
            return;
        }

        // create list if not exists
        if (!_AfterStageCallbacks.ContainsKey(stage))
        {
            _AfterStageCallbacks[stage] = new();
        }

        // add to list
        _AfterStageCallbacks[stage].Add(new StageInfo
        {
            Callback = callback,
            Order = order
        });
    }

    // update stage
    public static void UpdateStage(EBuildStage stage)
    {
        // bad update
        if (stage <= _CurrentStage)
            throw new InvalidOperationException($"Cannot move BuildStage from {_CurrentStage} to {stage}");

        // update should be sequential
        if (stage != _CurrentStage + 1)
            throw new InvalidOperationException($"Cannot skip BuildStage from {_CurrentStage} to {stage}");

        // update stage
        _CurrentStage = stage;

        // call callbacks
        if (_AfterStageCallbacks.ContainsKey(stage))
        {
            var callbacks = _AfterStageCallbacks[stage];
            callbacks.Sort((a, b) => b.Order - a.Order);
            foreach (var info in callbacks)
            {
                info.Callback();
                // Console.WriteLine($"BuildStage {_CurrentStage}: called callback with order {info.Order}");
            }
        }
    }
}