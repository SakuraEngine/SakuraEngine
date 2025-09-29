using System.Reflection;

namespace Cli;


#region Attributes
/// <summary>
/// Attribute to mark a property as a command-line option
/// </summary>
[AttributeUsage(AttributeTargets.Property)]
public class OptionAttribute : Attribute
{
    public required string Name { get; set; }
    public char ShortName { get; set; } = '\0';
    public required string Help { get; set; }
    public bool IsRequired { get; set; } = false;
    public string[]? Selections { get; set; } = null;
}

public delegate IEnumerable<string> OptionSelectionProviderDelegate();
[AttributeUsage(AttributeTargets.Method)]
public class OptionSelectionProviderAttribute : Attribute
{
    public string OptionName { get; set; }
    public OptionSelectionProviderAttribute(string optionName)
    {
        OptionName = optionName;
    }
}

/// <summary>
/// Attribute to mark a property as a sub-command
/// </summary>
[AttributeUsage(AttributeTargets.Property)]
public class SubCmdAttribute : Attribute
{
    public required string Name { get; set; }
    public char ShortName { get; set; } = '\0';
    public required string Help { get; set; } = string.Empty;
    public required string Usage { get; set; } = string.Empty;
}

/// <summary>
/// Attribute to mark a property as the rest options, which collects all unparsed arguments
/// </summary>
[AttributeUsage(AttributeTargets.Property)]
public class RestOptionsAttribute : Attribute
{
    public required string Help { get; set; }
    public bool IsRequired { get; set; } = false;

    /// <summary>
    /// 是否支持混合解析，如果为 false，在在遇到第一个未被 Option 消耗的参数时，将停止解析，后续参数全部归 RestOptions 处理
    /// </summary>
    public bool AllowMixed { get; set; } = false;
    /// <summary>
    /// 是否要求必须有 -- 分隔符才能开始收集 RestOptions，如果为 true，任何 -- 之前的未识别参数都会报错
    /// </summary>
    public bool RequireDoubleDash { get; set; } = false;
}

/// <summary>
/// Attribute to mark a method as the command execution handler
/// </summary>
[AttributeUsage(AttributeTargets.Method)]
public class ExecCmdAttribute : Attribute
{
}

/// <summary>
/// Attribute to mark a class to register commands to the default parser
/// </summary>
/// <returns></returns>
[AttributeUsage(AttributeTargets.Method)]
public class RegisterCmdAttribute : Attribute
{
    public required string Name { get; set; }
    public char ShortName { get; set; } = '\0';
    public required string Help { get; set; } = string.Empty;
    public required string Usage { get; set; } = string.Empty;
}
#endregion

#region cmd parser


public class ReflOption : IOption
{
    public OptionSelectionProviderDelegate? SelectionProvider { get; set; } = null;
    private PropertyInfo _Property;
    private object _Instance;
    private OptionAttribute _Attribute;
    public ReflOption(PropertyInfo prop, object instance, OptionAttribute attr)
    {
        _Property = prop;
        _Instance = instance;
        _Attribute = attr;

        // check property type
        if (!IsBool && !IsInt && !isUint && !IsFloat && !IsDouble && !IsString)
        {
            throw new InvalidOperationException($"Option property '{prop.Name}' has unsupported type '{prop.PropertyType}'");
        }
    }

    #region impl IOption
    public string Name => _Attribute.Name;
    public char? ShortName => _Attribute.ShortName == '\0' ? null : _Attribute.ShortName;
    public string Help => _Attribute.Help;
    public bool IsRequired => _Attribute.IsRequired;
    public IEnumerable<string>? Selections => _Attribute.Selections ?? SelectionProvider?.Invoke();
    public bool IsToggle => IsBool;
    public string DefaultValue
    {
        get
        {
            var value = _Property.GetValue(_Instance);
            if (value == null)
            {
                return "null";
            }
            else if (IsBool)
            {
                return ((bool)value) ? "true" : "false";
            }
            else if (IsInt)
            {
                return ((int)value).ToString();
            }
            else if (isUint)
            {
                return ((uint)value).ToString();
            }
            else if (IsFloat)
            {
                return ((float)value).ToString();
            }
            else if (IsDouble)
            {
                return ((double)value).ToString();
            }
            else if (IsString)
            {
                return $"\"{(string)value}\"";
            }
            else
            {
                return "unknown";
            }
        }
    }
    public string DumpTypeName()
    {
        if (IsBool)
        {
            return "bool";
        }
        else if (IsInt)
        {
            return "int";
        }
        else if (isUint)
        {
            return "uint";
        }
        else if (IsFloat || IsDouble)
        {
            return "float";
        }
        else if (IsString)
        {
            return "string";
        }
        else
        {
            return $"unknown";
        }
    }
    public void Assign(ParseContext ctx, string value)
    {
        if (IsBool)
        {
            throw new InvalidOperationException("Bool options cannot be assigned a value, use Toggle instead");
        }
        else if (IsInt)
        {
            if (int.TryParse(value, out int intValue))
            {
                _Property.SetValue(_Instance, intValue);
            }
            else
            {
                ctx.Error($"Option '{Name}' requires an integer value, but got '{value}'");
            }
        }
        else if (isUint)
        {
            if (uint.TryParse(value, out uint uintValue))
            {
                _Property.SetValue(_Instance, uintValue);
            }
            else
            {
                ctx.Error($"Option '{Name}' requires an unsigned integer value, but got '{value}'");
            }
        }
        else if (IsFloat)
        {
            if (float.TryParse(value, out float floatValue))
            {
                _Property.SetValue(_Instance, floatValue);
            }
            else if (double.TryParse(value, out double doubleValue))
            {
                _Property.SetValue(_Instance, doubleValue);
            }
            else
            {
                ctx.Error($"Option '{Name}' requires a float value, but got '{value}'");
            }
        }
        else if (IsDouble)
        {
            if (double.TryParse(value, out double doubleValue))
            {
                _Property.SetValue(_Instance, doubleValue);
            }
            else
            {
                ctx.Error($"Option '{Name}' requires a double value, but got '{value}'");
            }
        }
        else if (IsString)
        {
            _Property.SetValue(_Instance, value);
        }
        else
        {
            throw new InvalidOperationException($"Option property '{_Property.Name}' has unsupported type '{_Property.PropertyType}'");
        }
    }
    public void Toggle(ParseContext ctx)
    {
        if (!IsBool)
        {
            throw new InvalidOperationException("Toggle can only be used on bool options");
        }
        _Property.SetValue(_Instance, true);
    }
    #endregion

    #region OptionType
    public bool IsBool => _Property.PropertyType == typeof(bool) || _Property.PropertyType == typeof(bool?);
    public bool IsInt => _Property.PropertyType == typeof(int) || _Property.PropertyType == typeof(int?);
    public bool isUint => _Property.PropertyType == typeof(uint) || _Property.PropertyType == typeof(uint?);
    public bool IsFloat => _Property.PropertyType == typeof(float) || _Property.PropertyType == typeof(float?) || _Property.PropertyType == typeof(double) || _Property.PropertyType == typeof(double?);
    public bool IsDouble => _Property.PropertyType == typeof(double) || _Property.PropertyType == typeof(double?);
    public bool IsString => _Property.PropertyType == typeof(string);
    #endregion

    #region OptionConfig

    #endregion
}
public class ReflRestOptions : IRestOption
{
    private PropertyInfo _Property;
    private object _Instance;
    private RestOptionsAttribute _Attribute;
    public ReflRestOptions(PropertyInfo prop, object instance, RestOptionsAttribute attr)
    {
        _Property = prop;
        _Instance = instance;
        _Attribute = attr;

        // check property type
        if (_Property.PropertyType != typeof(List<string>) && _Property.PropertyType != typeof(string[]))
        {
            throw new InvalidOperationException($"RestOptions property '{prop.Name}' must be of type List<string> or string[]");
        }
    }

    #region impl IRestOptions
    public string Help => _Attribute.Help;
    public bool AllowMixed => _Attribute.AllowMixed;
    public bool RequireDoubleDash => _Attribute.RequireDoubleDash;
    public void Assign(ParseContext ctx, List<string> values)
    {
        if (_Property.PropertyType == typeof(List<string>))
        {
            _Property.SetValue(_Instance, values);
        }
        else if (_Property.PropertyType == typeof(string[]))
        {
            _Property.SetValue(_Instance, values.ToArray());
        }
        else
        {
            throw new InvalidOperationException($"RestOptions property '{_Property.Name}' must be of type List<string> or string[]");
        }
    }
    #endregion
}
public class ReflCommand : Command
{
    public ReflCommand(object instance)
    {
        var type = instance.GetType();

        // solve options and sub commands
        foreach (var prop in type.GetProperties(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
        {
            // add options
            var optionAttr = prop.GetCustomAttribute<OptionAttribute>();
            if (optionAttr != null)
            {
                Options.Add(new ReflOption(prop, instance, optionAttr));
                continue;
            }

            // add sub command
            var subCmdAttr = prop.GetCustomAttribute<SubCmdAttribute>();
            if (subCmdAttr != null)
            {
                var node = new ReflCommand(prop.GetValue(instance)!, subCmdAttr);
                SubCommands.Add(node);
            }

            // add rest options
            var restAttr = prop.GetCustomAttribute<RestOptionsAttribute>();
            if (restAttr != null)
            {
                if (RestOption != null)
                {
                    throw new InvalidOperationException("A command can only have one RestOptions");
                }
                RestOption = new ReflRestOptions(prop, instance, restAttr);
            }
        }

        // find selection providers
        foreach (var method in type.GetMethods(BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
        {
            var selectionAttr = method.GetCustomAttribute<OptionSelectionProviderAttribute>();
            if (selectionAttr != null)
            {
                // check signature
                if (method.GetParameters().Length != 0 || method.ReturnType != typeof(IEnumerable<string>))
                {
                    throw new InvalidOperationException("OptionSelectionProvider method must have no parameters and return IEnumerable<string>");
                }

                // check is static
                if (!method.IsStatic)
                {
                    throw new InvalidOperationException("OptionSelectionProvider method must be static");
                }

                // find option
                var option = Options.FirstOrDefault(o => o.Name == selectionAttr.OptionName) as ReflOption;
                if (option == null)
                {
                    throw new InvalidOperationException($"OptionSelectionProvider method '{method.Name}' refers to unknown option '{selectionAttr.OptionName}'");
                }

                // create delegate
                option.SelectionProvider = (OptionSelectionProviderDelegate)Delegate.CreateDelegate(typeof(OptionSelectionProviderDelegate), method);
            }
        }

        // find exec method
        foreach (var method in type.GetMethods())
        {
            var execAttr = method.GetCustomAttribute<ExecCmdAttribute>();
            if (execAttr != null)
            {
                // check signature
                if (method.GetParameters().Length != 0 || method.ReturnType != typeof(int))
                {
                    throw new InvalidOperationException("ExecCmd method must have no parameters and return int");
                }

                // check is static
                if (method.IsStatic)
                {
                    throw new InvalidOperationException("ExecCmd method cannot be static");
                }

                // check duplicate
                if (Execute != null)
                {
                    throw new InvalidOperationException("A command can only have one ExecCmd method");
                }

                // create delegate
                Execute = (ExecuteDelegate)Delegate.CreateDelegate(typeof(ExecuteDelegate), instance, method);
            }
        }

        // final check
        CheckSubCommands();
        CheckOptions();
    }
    public ReflCommand(object instance, SubCmdAttribute attr) : this(instance)
    {
        SetupConfigFromAttribute(attr);
    }


    public void SetupConfigFromAttribute(SubCmdAttribute attr)
    {
        Name = attr.Name;
        ShortName = attr.ShortName == '\0' ? null : attr.ShortName;
        Help = attr.Help;
        Usage = attr.Usage;
    }

    public static List<ReflCommand> CollectFromAssembly(Assembly assembly)
    {
        List<ReflCommand> commands = new List<ReflCommand>();
        foreach (var type in assembly.GetTypes())
        {
            foreach (var method in type.GetMethods(BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic))
            {
                var globalCmdAttr = method.GetCustomAttribute<RegisterCmdAttribute>();
                if (globalCmdAttr != null)
                {
                    // check signature
                    if (method.GetParameters().Length != 0 || method.ReturnType != typeof(object))
                    {
                        throw new InvalidOperationException("GlobalCmd method must have no parameters and return an instance of command class");
                    }

                    // check is static
                    if (!method.IsStatic)
                    {
                        throw new InvalidOperationException("GlobalCmd method must be static");
                    }

                    // create Command
                    var instance = method.Invoke(null, null);
                    var command = new ReflCommand(instance!);
                    command.Name = globalCmdAttr.Name;
                    command.ShortName = globalCmdAttr.ShortName == '\0' ? null : globalCmdAttr.ShortName;
                    command.Help = globalCmdAttr.Help;
                    command.Usage = globalCmdAttr.Usage;
                    commands.Add(command);
                }
            }
        }
        return commands;
    }
    public static List<ReflCommand> CollectFromDomain(AppDomain domain)
    {
        List<ReflCommand> commands = new List<ReflCommand>();
        foreach (var assembly in domain.GetAssemblies())
        {
            commands.AddRange(CollectFromAssembly(assembly));
        }
        return commands;
    }
    public static List<ReflCommand> CollectFromCurrentDomain()
    {
        return CollectFromDomain(AppDomain.CurrentDomain);
    }

    public static Command CreateDefaultRootCommandFromDomain(AppDomain domain, Command cmd)
    {
        // collect commands from current domain
        var commands = CollectFromDomain(domain);

        // add to root command
        foreach (var command in commands)
        {
            cmd.SubCommands.Add(command);
        }

        cmd.CheckSubCommands();

        return cmd;
    }

    public static int InvokeDefaultCommandFromDomain(AppDomain domain, Command cmd, string[] args, string banner)
    {
        CreateDefaultRootCommandFromDomain(domain, cmd);
        var parser = new CommandParser { RootCommand = cmd, Banner = banner };
        parser.Invoke(args);
        return parser.ExitCode;
    }
}

#endregion