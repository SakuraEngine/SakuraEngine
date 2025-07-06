using System.CommandLine;
using System.CommandLine.Builder;
using System.CommandLine.Invocation;
using System.CommandLine.Parsing;
using System.Reflection;

namespace SB;

/// <summary>
/// Attribute to mark a property as a command-line option, matching SkrCore's CmdOption
/// </summary>
[AttributeUsage(AttributeTargets.Property)]
public class CmdOptionAttribute : Attribute
{
    public string Name { get; set; } = string.Empty;
    public char ShortName { get; set; }
    public string Help { get; set; } = string.Empty;
    public bool IsRequired { get; set; } = true;
}

/// <summary>
/// Attribute to mark a method as the command execution handler, matching SkrCore's CmdExec
/// </summary>
[AttributeUsage(AttributeTargets.Method)]
public class CmdExecAttribute : Attribute
{
}

/// <summary>
/// Attribute to mark a property as a sub-command, matching SkrCore's CmdSub
/// </summary>
[AttributeUsage(AttributeTargets.Property)]
public class CmdSubAttribute : Attribute
{
    public string Name { get; set; } = string.Empty;
    public char ShortName { get; set; }
    public string Help { get; set; } = string.Empty;
    public string Usage { get; set; } = string.Empty;
}

/// <summary>
/// A command parser that uses System.CommandLine to provide functionality similar to SkrCore's CmdParser
/// </summary>
public class CommandParser
{
    private RootCommand _rootCommand;
    private readonly Dictionary<Command, object> _commandHandlers = new();
    private readonly Dictionary<Command, MethodInfo> _commandExecMethods = new();

    public CommandParser()
    {
        _rootCommand = new RootCommand();
    }

    /// <summary>
    /// Set the main command configuration
    /// </summary>
    public void MainCmd<T>(T instance, string name = "", string description = "", string usage = "") where T : class
    {
        _rootCommand.Name = string.IsNullOrEmpty(name) ? AppDomain.CurrentDomain.FriendlyName : name;
        _rootCommand.Description = description;
        
        ConfigureCommand(_rootCommand, instance);
    }

    /// <summary>
    /// Add a sub-command
    /// </summary>
    public void SubCmd<T>(T instance, string name, string description = "", string usage = "") where T : class
    {
        var subCommand = new Command(name, description);
        ConfigureCommand(subCommand, instance);
        _rootCommand.AddCommand(subCommand);
    }

    /// <summary>
    /// Parse synchronously
    /// </summary>
    public int ParseSync(string[] args)
    {
        var parser = new CommandLineBuilder(_rootCommand)
            .UseDefaults()
            .UseHelp(ctx => 
            {
                var cmd = ctx.Command;
                PrintHelp(cmd);
            })
            .Build();

        return parser.Invoke(args);
    }

    private void ConfigureCommand(Command command, object instance)
    {
        var type = instance.GetType();
        _commandHandlers[command] = instance;

        // Find and configure options
        foreach (var prop in type.GetProperties())
        {
            var optionAttr = prop.GetCustomAttribute<CmdOptionAttribute>();
            if (optionAttr != null)
            {
                var optionName = string.IsNullOrEmpty(optionAttr.Name) ? prop.Name : optionAttr.Name;
                var currentValue = prop.GetValue(instance);
                var option = CreateOption(prop.PropertyType, optionName, optionAttr, currentValue);
                
                if (optionAttr.ShortName != '\0')
                {
                    option.AddAlias($"-{optionAttr.ShortName}");
                }

                command.AddOption(option);
            }

            // Find sub-commands
            var subCmdAttr = prop.GetCustomAttribute<CmdSubAttribute>();
            if (subCmdAttr != null)
            {
                var subInstance = prop.GetValue(instance);
                if (subInstance != null)
                {
                    var subCmdName = string.IsNullOrEmpty(subCmdAttr.Name) ? prop.Name : subCmdAttr.Name;
                    var subCommand = new Command(subCmdName, subCmdAttr.Help);
                    
                    if (subCmdAttr.ShortName != '\0')
                    {
                        subCommand.AddAlias(subCmdAttr.ShortName.ToString());
                    }

                    ConfigureCommand(subCommand, subInstance);
                    command.AddCommand(subCommand);
                }
            }
        }

        // Find exec method
        var execMethod = type.GetMethods()
            .FirstOrDefault(m => m.GetCustomAttribute<CmdExecAttribute>() != null);
        
        if (execMethod != null)
        {
            _commandExecMethods[command] = execMethod;
            command.SetHandler((InvocationContext context) =>
            {
                // Set option values
                foreach (var prop in type.GetProperties())
                {
                    var optionAttr = prop.GetCustomAttribute<CmdOptionAttribute>();
                    if (optionAttr != null)
                    {
                        var optionName = string.IsNullOrEmpty(optionAttr.Name) ? prop.Name : optionAttr.Name;
                        var optionSymbol = command.Options.FirstOrDefault(o => o.Name == optionName);
                        
                        if (optionSymbol != null)
                        {
                            var value = context.ParseResult.GetValueForOption(optionSymbol);
                            // Only set the value if it was actually provided by the user
                            // This preserves default values set in the property initializer
                            if (value != null)
                            {
                                prop.SetValue(instance, value);
                            }
                        }
                    }
                }

                // Invoke the exec method
                execMethod.Invoke(instance, null);
            });
        }
    }

    private Option CreateOption(Type propertyType, string name, CmdOptionAttribute attr, object? defaultValue)
    {
        Option option;
        
        // Handle different types
        if (propertyType == typeof(bool) || propertyType == typeof(bool?))
        {
            option = new Option<bool>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(int) || propertyType == typeof(int?))
        {
            option = new Option<int>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(long) || propertyType == typeof(long?))
        {
            option = new Option<long>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(float) || propertyType == typeof(float?))
        {
            option = new Option<float>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(double) || propertyType == typeof(double?))
        {
            option = new Option<double>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(string))
        {
            option = new Option<string?>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else if (propertyType == typeof(List<string>) || propertyType == typeof(string[]))
        {
            option = new Option<string[]>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }
        else
        {
            // Default to string
            option = new Option<string?>($"--{name}", attr.Help);
            if (defaultValue != null)
                option.SetDefaultValue(defaultValue);
        }

        option.IsRequired = attr.IsRequired;
        return option;
    }

    private void PrintHelp(Command command)
    {
        var builder = new CliOutputBuilder();
        
        // Print command description
        if (!string.IsNullOrEmpty(command.Description))
        {
            builder.Line(command.Description);
        }

        // Print usage
        builder
            .StyleBold()
            .Write("usage: ")
            .StyleClear()
            .StyleFrontBlue()
            .Write(GetCommandPath(command))
            .StyleClear()
            .NextLine();

        // Print options
        if (command.Options.Any())
        {
            builder
                .StyleBold()
                .Line("options:")
                .StyleClear();

            var maxOptionLength = command.Options.Max(o => GetOptionDisplayName(o).Length);

            foreach (var option in command.Options)
            {
                var optionStr = GetOptionDisplayName(option).PadRight(maxOptionLength);
                var typeStr = GetOptionType(option);
                
                builder
                    .Write("  ")
                    .StyleFrontYellow()
                    .Write($"{typeStr} ")
                    .StyleClear()
                    .StyleBold()
                    .StyleFrontGreen()
                    .Write($"{optionStr} : ")
                    .StyleClear()
                    .WriteIndent($"{(option.IsRequired ? "[REQUIRED] " : "")}{option.Description}")
                    .NextLine();
            }
        }

        // Print subcommands
        if (command.Subcommands.Any())
        {
            builder
                .StyleBold()
                .Write("sub commands:")
                .StyleClear()
                .NextLine();

            var maxSubCmdLength = command.Subcommands.Max(c => GetSubCommandDisplayName(c).Length);

            foreach (var subCmd in command.Subcommands)
            {
                var subCmdStr = GetSubCommandDisplayName(subCmd).PadRight(maxSubCmdLength);
                
                builder
                    .StyleBold()
                    .StyleFrontCyan()
                    .Write($"  {subCmdStr} : ")
                    .StyleClear()
                    .Write(subCmd.Description ?? "")
                    .NextLine();
            }
        }

        builder.Dump();
    }

    private string GetCommandPath(Command command)
    {
        // Since we don't have access to Parent in this version,
        // we'll just use the command name for now
        return command.Name;
    }

    private string GetOptionDisplayName(Option option)
    {
        var parts = new List<string>();
        
        // Get short alias
        var shortAlias = option.Aliases.FirstOrDefault(a => a.StartsWith("-") && !a.StartsWith("--"));
        if (shortAlias != null)
        {
            parts.Add($"{shortAlias},");
        }
        else
        {
            parts.Add("   ");
        }
        
        parts.Add(option.Name);
        return string.Join(" ", parts);
    }

    private string GetSubCommandDisplayName(Command command)
    {
        var parts = new List<string>();
        
        // Get short alias
        var shortAlias = command.Aliases.FirstOrDefault(a => a.Length == 1);
        if (shortAlias != null)
        {
            parts.Add($"{shortAlias},");
        }
        else
        {
            parts.Add("  ");
        }
        
        parts.Add(command.Name);
        return string.Join(" ", parts);
    }

    private string GetOptionType(Option option)
    {
        var valueType = option.ValueType;
        
        if (valueType == typeof(bool))
            return "<bool>     ";
        else if (IsIntegerType(valueType))
            return "<int>      ";
        else if (IsFloatType(valueType))
            return "<float>    ";
        else if (valueType == typeof(string))
            return "<string>   ";
        else if (IsStringArrayType(valueType))
            return "<string...>";
        else
            return "<unknown>  ";
    }

    private bool IsIntegerType(Type type)
    {
        return type == typeof(int) || type == typeof(long) || 
               type == typeof(short) || type == typeof(byte) ||
               type == typeof(uint) || type == typeof(ulong) ||
               type == typeof(ushort) || type == typeof(sbyte);
    }

    private bool IsFloatType(Type type)
    {
        return type == typeof(float) || type == typeof(double) || type == typeof(decimal);
    }

    private bool IsStringArrayType(Type type)
    {
        return type == typeof(string[]) || type == typeof(List<string>) || 
               type == typeof(IEnumerable<string>) || type == typeof(IList<string>);
    }
}