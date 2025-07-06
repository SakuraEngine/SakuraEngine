namespace SB;

/// <summary>
/// Provides ANSI escape codes for styling terminal output, matching SkrCore's cli_style namespace
/// </summary>
public static class CliStyle
{
    // Clear all formatting
    public const string Clear = "\x1b[0m";

    // Style attributes
    public const string Bold = "\x1b[1m";
    public const string NoBold = "\x1b[22m";
    public const string Underline = "\x1b[4m";
    public const string NoUnderline = "\x1b[24m";
    public const string Reverse = "\x1b[7m";
    public const string NoReverse = "\x1b[27m";

    // Foreground colors
    public const string FrontGray = "\x1b[30m";
    public const string FrontRed = "\x1b[31m";
    public const string FrontGreen = "\x1b[32m";
    public const string FrontYellow = "\x1b[33m";
    public const string FrontBlue = "\x1b[34m";
    public const string FrontMagenta = "\x1b[35m";
    public const string FrontCyan = "\x1b[36m";
    public const string FrontWhite = "\x1b[37m";
}

/// <summary>
/// A builder for creating styled CLI output, matching SkrCore's CliOutputBuilder
/// </summary>
public class CliOutputBuilder
{
    private string _content = string.Empty;
    private int _indentCache = 0;

    // Style methods
    public CliOutputBuilder StyleClear()
    {
        _content += CliStyle.Clear;
        return this;
    }

    public CliOutputBuilder StyleBold()
    {
        _content += CliStyle.Bold;
        return this;
    }

    public CliOutputBuilder StyleNoBold()
    {
        _content += CliStyle.NoBold;
        return this;
    }

    public CliOutputBuilder StyleUnderline()
    {
        _content += CliStyle.Underline;
        return this;
    }

    public CliOutputBuilder StyleNoUnderline()
    {
        _content += CliStyle.NoUnderline;
        return this;
    }

    public CliOutputBuilder StyleReverse()
    {
        _content += CliStyle.Reverse;
        return this;
    }

    public CliOutputBuilder StyleNoReverse()
    {
        _content += CliStyle.NoReverse;
        return this;
    }

    public CliOutputBuilder StyleFrontGray()
    {
        _content += CliStyle.FrontGray;
        return this;
    }

    public CliOutputBuilder StyleFrontRed()
    {
        _content += CliStyle.FrontRed;
        return this;
    }

    public CliOutputBuilder StyleFrontGreen()
    {
        _content += CliStyle.FrontGreen;
        return this;
    }

    public CliOutputBuilder StyleFrontYellow()
    {
        _content += CliStyle.FrontYellow;
        return this;
    }

    public CliOutputBuilder StyleFrontBlue()
    {
        _content += CliStyle.FrontBlue;
        return this;
    }

    public CliOutputBuilder StyleFrontMagenta()
    {
        _content += CliStyle.FrontMagenta;
        return this;
    }

    public CliOutputBuilder StyleFrontCyan()
    {
        _content += CliStyle.FrontCyan;
        return this;
    }

    public CliOutputBuilder StyleFrontWhite()
    {
        _content += CliStyle.FrontWhite;
        return this;
    }

    // Content methods
    public CliOutputBuilder Write(string format, params object[] args)
    {
        var str = string.Format(format, args);
        SolveIndent(str, ref _indentCache);
        _content += str;
        return this;
    }

    public CliOutputBuilder Line(string format, params object[] args)
    {
        _content += string.Format(format, args);
        NextLine();
        return this;
    }

    public CliOutputBuilder NextLine()
    {
        _content += "\n";
        _indentCache = 0;
        return this;
    }

    public CliOutputBuilder WriteIndent(string format, params object[] args)
    {
        var str = string.Format(format, args);
        int newIndent = 0;
        SolveIndent(str, ref newIndent);
        
        if (_indentCache > 0)
        {
            var indentStr = "\n" + new string(' ', _indentCache);
            str = str.Replace("\n", indentStr);
        }
        
        _indentCache = newIndent;
        _content += str;
        return this;
    }

    // Dump the content to console
    public void Dump()
    {
        Console.Write(_content);
    }

    public string Content => _content;

    // Helper method to calculate indentation
    private static void SolveIndent(string str, ref int indent)
    {
        var lastNewline = str.LastIndexOf('\n');
        if (lastNewline >= 0)
        {
            indent = str.Length - lastNewline - 1;
        }
        else
        {
            indent += str.Length;
        }
    }
}