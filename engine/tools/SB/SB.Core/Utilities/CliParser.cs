using System.Reflection;
using System.Text;

namespace SB.Cli;

#region Token
public class Token
{
    public enum EKind
    {
        Argument, // any token not starting with '-'
        Option, // --xxx, --xxx=yyy
        ShortOption, // -a, -a=xxx, -abc(means -a -b -c), -abc=false
        DoubleDash, // --, means end of parse, and push all command into rest
        Unparsed,// any token after --
    };

    #region getters
    public EKind Kind => _Kind;
    public string Raw => _Raw;
    public bool IsArgument => _Kind == EKind.Argument;
    public bool IsOption => _Kind == EKind.Option;
    public bool IsShortOption => _Kind == EKind.ShortOption;
    public bool IsAnyOption => _Kind == EKind.Option || _Kind == EKind.ShortOption;
    public bool IsDoubleDash => _Kind == EKind.DoubleDash;
    public bool IsUnparsed => _Kind == EKind.Unparsed;
    public string Argument
    {
        get
        {
            if (_Kind != EKind.Argument) throw new InvalidOperationException("Not an Argument token");
            return _Raw;
        }
    }
    public string OptionName
    {
        get
        {
            if (_Kind != EKind.Option) throw new InvalidOperationException("Not an Option token");

            var eqIdx = _Raw.IndexOf('=');
            if (eqIdx >= 0)
            {
                return _Raw.Substring(2, eqIdx - 2);
            }
            else
            {
                return _Raw.Substring(2);
            }
        }
    }
    public string? OptionValue
    {
        get
        {
            if (_Kind != EKind.Option) throw new InvalidOperationException("Not an Option token");

            var eqIdx = _Raw.IndexOf('=');
            if (eqIdx >= 0)
            {
                return _Raw.Substring(eqIdx + 1);
            }
            else
            {
                return null;
            }
        }
    }
    public List<char> ShortOptionNames
    {
        get
        {
            if (_Kind != EKind.ShortOption) throw new InvalidOperationException("Not a ShortOption token");

            var eqIdx = _Raw.IndexOf('=');
            var namesPart = eqIdx >= 0 ? _Raw.Substring(1, eqIdx - 1) : _Raw.Substring(1);
            var names = new List<char>();
            foreach (var c in namesPart)
            {
                names.Add(c);
            }
            return names;
        }
    }
    public string? ShortOptionValue
    {
        get
        {
            if (_Kind != EKind.ShortOption) throw new InvalidOperationException("Not a ShortOption token");

            var eqIdx = _Raw.IndexOf('=');
            if (eqIdx >= 0)
            {
                return _Raw.Substring(eqIdx + 1);
            }
            else
            {
                return null;
            }
        }
    }
    public bool IsOptionOf(string name)
    {
        return _Kind == EKind.Option && OptionName == name;
    }
    public bool IsShortOptionOf(char shortName)
    {
        return _Kind == EKind.ShortOption && ShortOptionNames.Count == 1 && ShortOptionNames.Contains(shortName);
    }
    public bool HasShortOption(char shortName)
    {
        return _Kind == EKind.ShortOption && ShortOptionNames.Contains(shortName);
    }
    #endregion

    private Token() { }
    public static Token Parse(string raw)
    {
        Token result = new Token();

        result._Raw = raw;

        if (raw == "--")
        {
            result._Kind = EKind.DoubleDash;
        }
        else if (raw.StartsWith("--"))
        {
            result._Kind = EKind.Option;
        }
        else if (raw.StartsWith("-") && raw.Length >= 2)
        {
            result._Kind = EKind.ShortOption;
        }
        else
        {
            result._Kind = EKind.Argument;
        }

        return result;
    }
    public static Token Unparsed(string raw)
    {
        Token result = new Token();
        result._Raw = raw;
        result._Kind = EKind.Unparsed;
        return result;
    }

    private EKind _Kind;
    private string _Raw = "";
};
public class TokenList
{
    public TokenList(IEnumerable<Token> tokens)
    {
        _Tokens.AddRange(tokens);
    }
    public TokenList(IEnumerable<string> args)
    {
        bool doNotParse = false;
        foreach (var arg in args)
        {
            if (doNotParse)
            {
                _Tokens.Add(Token.Unparsed(arg));
            }
            else
            {
                var token = Token.Parse(arg);
                if (token.IsDoubleDash)
                {
                    doNotParse = true;
                }
                _Tokens.Add(token);
            }
        }
    }

    // walk tokens
    public bool HasMore() => _Index < _Tokens.Count;
    public Token Peek()
    {
        if (!HasMore()) throw new InvalidOperationException("No more tokens");
        return _Tokens[_Index];
    }
    public Token Match()
    {
        if (!HasMore()) throw new InvalidOperationException("No more tokens");
        var token = _Tokens[_Index];
        _MatchedTokens.Add(token);
        ++_Index;
        return token;
    }
    public Token Rest()
    {
        if (!HasMore()) throw new InvalidOperationException("No more tokens");
        var token = _Tokens[_Index];
        _RestTokens.Add(token);
        ++_Index;
        return token;
    }
    public void ResetAllUnused()
    {
        for (int i = _Index; i < _Tokens.Count; ++i)
        {
            // filter double dash
            if (_Tokens[i].IsDoubleDash) continue;
            _RestTokens.Add(_Tokens[i]);
        }
        _Index = _Tokens.Count;
    }
    public void ResetToHead()
    {
        _Index = 0;
        _MatchedTokens.Clear();
        _RestTokens.Clear();
    }

    // getters
    public IEnumerable<Token> AllTokens => _Tokens;
    public IEnumerable<Token> MatchedTokens => _MatchedTokens;
    public IEnumerable<Token> RestTokens => _RestTokens;
    public IEnumerable<Token> UnusedTokens
    {
        get
        {
            for (int i = _Index; i < _Tokens.Count; ++i)
            {
                yield return _Tokens[i];
            }
        }
    }

    // tokens
    private List<Token> _Tokens = new();

    // parse context
    private List<Token> _MatchedTokens = new();
    private List<Token> _RestTokens = new();
    private int _Index = 0;
}
#endregion

#region Options and Commands
public interface IOption
{
    public string Name { get; }
    public char? ShortName { get; }
    public string Help { get; }
    public bool IsRequired { get; }
    public IEnumerable<string>? Selections { get; }
    public bool IsToggle { get; }
    public string DefaultValue { get; }
    public string DumpTypeName();
    public void Assign(ParseContext ctx, string value);
    public void Toggle(ParseContext ctx);
}
public interface IRestOption
{
    public string Help { get; }
    public bool AllowMixed { get; }
    public bool RequireDoubleDash { get; }
    public void Assign(ParseContext ctx, List<string> values);
}
public class Command
{
    /// <summary>
    /// Command 执行器
    /// </summary>
    public delegate int ExecuteDelegate();


    #region Config
    /// <summary>
    /// Command 的名称
    /// </summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Command 的简写名称
    /// </summary>
    public char? ShortName { get; set; } = null;

    /// <summary>
    /// Command 的帮助信息
    /// </summary>
    public string Help { get; set; } = string.Empty;

    /// <summary>
    /// Command 的使用说明
    /// </summary>
    public string Usage { get; set; } = string.Empty;
    #endregion

    #region exec info

    /// <summary>
    /// Command 的执行函数
    /// </summary>
    public ExecuteDelegate? Execute { get; set; }

    /// <summary>
    /// Command 的选项
    /// </summary>
    public List<IOption> Options { get; set; } = new();

    /// <summary>
    /// 用于处理剩余参数的选项
    /// </summary>
    public IRestOption? RestOption { get; set; } = null;

    /// <summary>
    /// Command 的子命令
    /// </summary>
    public List<Command> SubCommands { get; set; } = new();
    #endregion

    public void CheckOptions()
    {
        Dictionary<string, IOption> seenOptions = new();
        Dictionary<char, IOption> seenShortOptions = new();

        foreach (var option in Options)
        {
            // check name
            if (seenOptions.ContainsKey(option.Name))
            {
                throw new InvalidOperationException($"Duplicate option name: {option.Name}");
            }
            seenOptions[option.Name] = option;

            // check short name
            if (option.ShortName != null)
            {
                if (seenShortOptions.ContainsKey(option.ShortName.Value))
                {
                    throw new InvalidOperationException($"Duplicate short option name: {option.ShortName}");
                }
                seenShortOptions[option.ShortName.Value] = option;
            }
        }
    }
    public void CheckSubCommands()
    {
        Dictionary<string, Command> seenSubCommands = new();
        Dictionary<char, Command> seenShortSubCommands = new();

        foreach (var subCmd in SubCommands)
        {
            // check name
            if (seenSubCommands.ContainsKey(subCmd.Name))
            {
                throw new InvalidOperationException($"Duplicate sub command name: {subCmd.Name}");
            }
            seenSubCommands[subCmd.Name] = subCmd;

            // check short name
            if (subCmd.ShortName != null)
            {
                if (seenShortSubCommands.ContainsKey(subCmd.ShortName.Value))
                {
                    throw new InvalidOperationException($"Duplicate short sub command name: {subCmd.ShortName}");
                }
                seenShortSubCommands[subCmd.ShortName.Value] = subCmd;
            }
        }
    }

    public void WriteHelp(CliWriter writer)
    {
        // print sakura build logo
        writer
            .StyleBold().StyleFrontGreen()
            .WriteLine(@"")
            .WriteLine(@"")
            .WriteLine(@"    _____         _                        ____          _  _      _ ")
            .WriteLine(@"   / ____|       | |                      |  _ \        (_)| |    | |")
            .WriteLine(@"  | (___    __ _ | | __ _   _  _ __  __ _ | |_) | _   _  _ | |  __| |")
            .WriteLine(@"   \___ \  / _` || |/ /| | | || '__|/ _` ||  _ < | | | || || | / _` |")
            .WriteLine(@"   ____) || (_| ||   < | |_| || |  | (_| || |_) || |_| || || || (_| |")
            .WriteLine(@"  |_____/  \__,_||_|\_\ \__,_||_|   \__,_||____/  \__,_||_||_| \__,_|")
            .WriteLine(@"")
            .WriteLine(@"")
            .StyleClear();


        // print help
        writer
            .StyleFrontGreen()
            .StyleBold()
            .WriteKeepIndent(Help)
            .StyleClear()
            .NextLine();

        // print self usage
        writer
            // usage:
            .StyleBold()
            .Write("Usage: ")
            .StyleClear()
            // content
            .StyleFrontBlue()
            .WriteKeepIndent(Usage)
            .StyleClear()
            .NextLine();

        // print sub commands
        if (SubCommands.Count != 0)
        {
            writer
                .StyleBold()
                .Write("Sub Commands:")
                .StyleClear()
                .NextLine();

            // solve max sub command length
            int maxSubCmdLength = 0;
            foreach (var subCmd in SubCommands)
            {
                int subCmdLen = 0;
                subCmdLen += 3; // 'X, '
                subCmdLen += subCmd.Name.Length; // 'XXXX'
                maxSubCmdLength = Math.Max(subCmdLen, maxSubCmdLength);
            }

            // print sub commands
            foreach (var subCmd in SubCommands)
            {
                // solve sub command str
                string subCmdStr;
                if (subCmd.ShortName != null)
                {
                    subCmdStr = $"{subCmd.ShortName}, {subCmd.Name}";
                }
                else
                {
                    subCmdStr = $"   {subCmd.Name}";
                }
                subCmdStr = subCmdStr.PadRight(maxSubCmdLength);

                writer
                    // write sub command
                    .StyleFrontMagenta()
                    .StyleBold()
                    .Write($"  {subCmdStr} : ")
                    .StyleClear()
                    // write help
                    .StyleClear()
                    .WriteKeepIndent(subCmd.Help)
                    .NextLine();
            }
        }

        // print options
        if (Options.Count != 0)
        {
            writer
                .StyleBold()
                .WriteLine("Options:")
                .StyleClear();

            // solve max option length
            int maxOptionLength = 0;
            int maxTypeLength = 0;
            int maxDefaultLength = 0;
            foreach (var option in Options)
            {
                int optionLen = 0;
                optionLen += 4; // '-X, '
                optionLen += 2 + option.Name.Length; // '--XXXX'
                maxOptionLength = Math.Max(optionLen, maxOptionLength);

                int typeLen = option.DumpTypeName().Length;
                maxTypeLength = Math.Max(typeLen, maxTypeLength);

                int defaultLen = option.DefaultValue.Length;
                maxDefaultLength = Math.Max(defaultLen, maxDefaultLength);
            }

            // print options
            foreach (var option in Options)
            {
                // solve type str
                string typeStr = $"<{option.DumpTypeName()}>".PadRight(maxTypeLength + 2);

                // solve default str
                string defaultStr = $"[{option.DefaultValue}]".PadRight(maxDefaultLength + 2);

                // solve options str
                string optionStr;
                if (option.ShortName != null)
                {
                    optionStr = $"-{option.ShortName}, --{option.Name}";
                }
                else
                {
                    optionStr = $"    --{option.Name}";
                }
                optionStr = optionStr.PadRight(maxOptionLength);

                // solve selections str
                string selectionsStr = "";
                {
                    var selections = option.Selections;
                    if (selections != null)
                    {
                        selectionsStr = "\n" + string.Join("\n", selections.Select(s => $"  - {s}"));
                    }
                }

                string requiredStr = option.IsRequired ? "[REQUIRED]" : "";

                writer
                    .Write("  ")
                    // write type
                    .StyleFrontYellow()
                    .Write($"  {typeStr} ")
                    .StyleClear()
                    // write option
                    .StyleFrontGreen()
                    .StyleBold()
                    .Write($"{optionStr} ")
                    .StyleClear()
                    // write default
                    .StyleFrontGray()
                    .StyleBold()
                    .Write($"{defaultStr}: ")
                    .StyleClear()
                    // write help
                    .StyleClear()
                    .WriteKeepIndent($"{requiredStr} {option.Help}{selectionsStr}")
                    .NextLine();
            }

            // print rest option help
            if (RestOption != null)
            {
                writer
                    .StyleBold()
                    .WriteLine("Rest Arguments:")
                    .StyleClear()
                    .Write("  ")
                    .WriteKeepIndent(RestOption.Help)
                    .NextLine();
            }
        }
    }
}
#endregion

#region Parse
public class ParseContext
{
    public required TokenList Tokens { get; init; }
    public TokenList? CommandTokens { get; private set; }
    public CliWriter Writer { get; init; } = new CliWriter();
    public bool AnyError { get; private set; } = false;
    public bool AnyWarning { get; private set; } = false;
    public void Error(string error)
    {
        AnyError = true;
        Writer
            .StyleFrontRed()
            .StyleBold()
            .Write("Error: ")
            .StyleClear()
            .WriteLine(error);
    }
    public void Warning(string warning)
    {
        AnyWarning = true;
        Writer
            .StyleFrontYellow()
            .StyleBold()
            .Write("Warning: ")
            .StyleClear()
            .WriteLine(warning);
    }
    public void StoreCommandTokens()
    {
        if (CommandTokens == null)
        {
            CommandTokens = new TokenList(Tokens.UnusedTokens);
        }
    }
}
// TODO. check required options
public class CommandParser
{
    /// <summary>
    /// 解析的根命令
    /// </summary>
    public required Command RootCommand { get; set; }

    /// <summary>
    /// 在解析到无法识别的 argument 时，是否阻止调用并报错，如果为 false 则只进行警告
    /// </summary>
    public bool FailedWhenUnrecognizedArg { get; set; } = true;

    /// <summary>
    /// 在解析到无法识别的 option 时，是否阻止调用并报错，如果为 false 则只进行警告
    /// </summary>
    public bool FailedWhenUnrecognizedOption { get; set; } = true;

    /// <summary>
    /// 在遇到需要参数的 option 但没有提供参数时，是否阻止调用并报错，如果为 false 则只进行警告
    /// </summary>
    public bool ErrorWhenMissingArgument { get; set; } = true;

    /// <summary>
    /// 当命令没有执行任何操作时，是否打印帮助信息
    /// </summary>
    public bool PrintHelpWhenCommandNoExecute { get; set; } = true;

    /// <summary>
    /// 如果发生调用，返回命令的返回值，否则为 -1
    /// </summary>
    public int ExitCode { get; private set; } = -1;


    /// <summary>
    /// 进行解析并调用命令
    /// </summary>
    /// <param name="args">来自命令行的参数</param>
    /// <returns>被成功 Invoke 的 Command</returns>
    public Command? Invoke(IEnumerable<string> args)
    {
        // parse args
        ParseContext ctx = new ParseContext()
        {
            Tokens = new TokenList(args)
        };

        // solve sub command
        var cmd = _SolveCommand(ctx);
        ctx.StoreCommandTokens();

        // invoke command
        if (_InvokeCommand(cmd, ctx))
        {
            return cmd;
        }
        return null;
    }

    #region Parse stage
    private Command _SolveCommand(ParseContext ctx)
    {
        Command currentNode = RootCommand!;
        while (ctx.Tokens.HasMore())
        {
            // peek token
            var token = ctx.Tokens.Peek();
            if (!token.IsArgument) break;

            // find command
            string cmdName = token.Argument;
            var nextNode = currentNode.SubCommands.Find((cmd) => cmd.Name == cmdName || (cmd.ShortName != null && cmd.ShortName.ToString() == cmdName));
            if (nextNode == null) break;

            // match command
            currentNode = nextNode;
            ctx.Tokens.Match();
        }
        return currentNode;
    }
    private bool _PrintHelpIfMeetHelpOption(Command cmd, Token token, ParseContext ctx)
    {
        if (token.IsOptionOf("help") || token.IsShortOptionOf('h'))
        {
            // dump unused tokens
            var unusedTokens = ctx.CommandTokens!.AllTokens.Where(t => t != token).ToList();
            if (unusedTokens.Count > 0)
            {
                string unusedStr = string.Join(", ", unusedTokens
                    .Select(t => $"'{t.Raw}'"));
                ctx.Warning($"these tokens are ignored when printing help: {unusedStr}");
            }

            // print help
            cmd.WriteHelp(ctx.Writer);

            // dump
            ctx.Writer.Dump();
            return true;
        }
        return false;
    }
    private bool _InvokeCommand(Command cmd, ParseContext ctx)
    {
        var tokenList = ctx.Tokens;

        // solve options
        while (tokenList.HasMore())
        {
            var token = tokenList.Peek();

            switch (token.Kind)
            {
                case Token.EKind.Argument: // handle unrecognized argument
                    if (cmd.RestOption != null)
                    {
                        if (cmd.RestOption.AllowMixed)
                        { // process as rest option
                            tokenList.Rest();
                        }
                        else if (!cmd.RestOption.RequireDoubleDash)
                        { // treat all unused options as rest
                            tokenList.ResetAllUnused();
                        }
                        else
                        { // illegal rest argument, before '--'
                            _HandleUnrecognizedArgument(token, ctx);
                            tokenList.Match();
                        }
                    }
                    else
                    { // no rest option, treat as unrecognized
                        _HandleUnrecognizedArgument(token, ctx);
                        tokenList.Match();
                    }
                    break;

                case Token.EKind.DoubleDash: // process rest arguments
                    if (cmd.RestOption != null)
                    { // process as rest option
                        tokenList.Match();
                        tokenList.ResetAllUnused();
                    }
                    else
                    { // no rest option, treat as unrecognized
                        tokenList.Match();
                        _HandleUnrecognizedArgument(token, ctx);
                        while (tokenList.HasMore())
                        {
                            _HandleUnrecognizedArgument(tokenList.Match(), ctx);
                        }
                    }
                    break;
                case Token.EKind.Option:
                    if (_PrintHelpIfMeetHelpOption(cmd, token, ctx))
                    {
                        ExitCode = 0;
                        return false;
                    }
                    else
                    {
                        _ParseOption(cmd, token, ctx);
                    }
                    break;
                case Token.EKind.ShortOption:
                    if (_PrintHelpIfMeetHelpOption(cmd, token, ctx))
                    {
                        ExitCode = 0;
                        return false;
                    }
                    else
                    {
                        _ParseShortOption(cmd, token, ctx);
                    }
                    break;
            }
        }

        // process rest option
        if (cmd.RestOption != null)
        {
            List<string> restArgs = tokenList.RestTokens.Select(t => t.Raw).ToList();
            cmd.RestOption.Assign(ctx, restArgs);
        }

        // invoke
        if (!ctx.AnyError)
        {
            if (cmd.Execute != null)
            {
                ExitCode = cmd.Execute.Invoke();
            }
            else if (PrintHelpWhenCommandNoExecute)
            { // no execute, print help
                cmd.WriteHelp(ctx.Writer);
                ctx.Writer.Dump();
            }
            return true;
        }
        else
        { // Error happened
            cmd.WriteHelp(ctx.Writer);
            ctx.Writer.Dump();
        }

        return false;
    }
    #endregion

    #region Parse helpers
    private void _ParseOption(Command cmd, Token token, ParseContext ctx)
    {
        var tokenList = ctx.Tokens;
        var optionName = token.OptionName;
        var optionValue = token.OptionValue;

        tokenList.Match();

        // find option
        var option = cmd.Options.Find(o => o.Name == optionName);
        if (option == null)
        {
            _HandleUnrecognizedOption(token, ctx);
        }
        else
        {
            if (option.IsToggle)
            {
                _AssignToggle(option, optionValue, token, ctx);
            }
            else
            {
                optionValue ??= _TryTakeNextAsValue(tokenList)?.Argument;
                _AssignOption(option, optionValue, token, ctx);
            }
        }
    }
    private void _ParseShortOption(Command cmd, Token token, ParseContext ctx)
    {
        var tokenList = ctx.Tokens;
        var optionNames = token.ShortOptionNames;
        var optionValue = token.ShortOptionValue;

        tokenList.Match();

        if (optionNames.Count > 1)
        { // means toggle group
            if (optionValue != null)
            {
                ctx.Error($"Short option '{token.Raw}' cannot have a value when use toggle group");
            }

            foreach (var shortName in optionNames)
            {
                var option = cmd.Options.Find(o => o.ShortName == shortName);
                if (option == null)
                {
                    _HandleUnrecognizedShortOption(token, shortName, ctx);
                }
                else
                {
                    if (!option.IsToggle)
                    {
                        ctx.Error($"Short option '{shortName}' in '{token.Raw}' is not a toggle option, cannot be used in toggle group");
                    }
                    else
                    {
                        _AssignToggle(option, null, token, ctx);
                    }
                }
            }
        }
        else
        {
            // find option
            var shortName = optionNames[0];
            var option = cmd.Options.Find(o => o.ShortName == shortName);
            if (option == null)
            {
                _HandleUnrecognizedShortOption(token, shortName, ctx);
            }
            else
            {
                if (option.IsToggle)
                {
                    _AssignToggle(option, optionValue, token, ctx);
                }
                else
                {
                    optionValue ??= _TryTakeNextAsValue(tokenList)?.Argument;
                    _AssignOption(option, optionValue, token, ctx);
                }
            }
        }
    }
    private Token? _TryTakeNextAsValue(TokenList tokenList)
    {
        if (tokenList.HasMore())
        {
            var nextToken = tokenList.Peek();
            if (nextToken.IsArgument)
            {
                return tokenList.Match();
            }
        }

        return null;
    }
    private void _AssignToggle(IOption option, string? optionValue, Token token, ParseContext ctx)
    {
        if (optionValue != null)
        {
            ctx.Error($"Short option '{token.Raw}' is a toggle option, cannot have a value");
        }
        else
        {
            option.Toggle(ctx);
        }
    }
    private void _AssignOption(IOption option, string? optionValue, Token token, ParseContext ctx)
    {
        if (optionValue == null)
        {
            _HandleOptionMissingArgument(option, token, ctx);
        }
        else
        {
            // handle selections
            if (option.Selections != null && !option.Selections.Contains(optionValue))
            {
                ctx.Error($"Option '{option.Name}' with value '{optionValue}' only accepts these values: {string.Join(", ", option.Selections)}");
                return;
            }

            option.Assign(ctx, optionValue);
        }
    }
    #endregion

    #region Error handlers
    private void _HandleUnrecognizedArgument(Token token, ParseContext ctx)
    {
        if (FailedWhenUnrecognizedArg)
        {
            ctx.Error($"Unrecognized argument: {token.Raw}");
        }
        else
        {
            ctx.Warning($"Unrecognized argument: {token.Raw}");
        }
    }
    private void _HandleUnrecognizedShortOption(Token token, char shortOption, ParseContext ctx)
    {
        if (FailedWhenUnrecognizedOption)
        {
            ctx.Error($"Unrecognized short option: -{shortOption} in '{token.Raw}'");
        }
        else
        {
            ctx.Warning($"Unrecognized short option: -{shortOption} in '{token.Raw}'");
        }
    }
    private void _HandleUnrecognizedOption(Token token, ParseContext ctx)
    {
        if (FailedWhenUnrecognizedOption)
        {
            ctx.Error($"Unrecognized option: --{token.Raw}");
        }
        else
        {
            ctx.Warning($"Unrecognized option: --{token.Raw}");
        }
    }
    private void _HandleOptionMissingArgument(IOption option, Token token, ParseContext ctx)
    {
        if (ErrorWhenMissingArgument)
        {
            ctx.Error($"Option '{option.Name}' requires an argument, but not provided in '{token.Raw}'");
        }
        else
        {
            ctx.Warning($"Option '{option.Name}' requires an argument, but not provided in '{token.Raw}'");
        }
    }
    #endregion
}
#endregion