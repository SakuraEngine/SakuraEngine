using System.Text;

namespace Cli;

public static class CliStyleStrings
{
    public const string Clear = "\u001b[0m";

    // style
    public const string Bold = "\u001b[1m";
    public const string NoBold = "\u001b[22m";
    public const string Underline = "\u001b[4m";
    public const string NoUnderline = "\u001b[24m";
    public const string Reverse = "\u001b[7m";
    public const string NoReverse = "\u001b[27m";

    // front colors
    public const string FrontGray = "\u001b[30m";
    public const string FrontRed = "\u001b[31m";
    public const string FrontGreen = "\u001b[32m";
    public const string FrontYellow = "\u001b[33m";
    public const string FrontBlue = "\u001b[34m";
    public const string FrontMagenta = "\u001b[35m";
    public const string FrontCyan = "\u001b[36m";
    public const string FrontWhite = "\u001b[37m";

    // back colors
    public const string BackGray = "\u001b[40m";
    public const string BackRed = "\u001b[41m";
    public const string BackGreen = "\u001b[42m";
    public const string BackYellow = "\u001b[43m";
    public const string BackBlue = "\u001b[44m";
    public const string BackMagenta = "\u001b[45m";
    public const string BackCyan = "\u001b[46m";
    public const string BackWhite = "\u001b[47m";
}
public class CliWriter
{
    #region Style
    public CliWriter StyleClear()
    {
        _Content.Append(CliStyleStrings.Clear);
        return this;
    }

    // style
    public CliWriter StyleBold()
    {
        _Content.Append(CliStyleStrings.Bold);
        return this;
    }
    public CliWriter StyleNoBold()
    {
        _Content.Append(CliStyleStrings.NoBold);
        return this;
    }
    public CliWriter StyleUnderline()
    {
        _Content.Append(CliStyleStrings.Underline);
        return this;
    }
    public CliWriter StyleNoUnderline()
    {
        _Content.Append(CliStyleStrings.NoUnderline);
        return this;
    }
    public CliWriter StyleReverse()
    {
        _Content.Append(CliStyleStrings.Reverse);
        return this;
    }
    public CliWriter StyleNoReverse()
    {
        _Content.Append(CliStyleStrings.NoReverse);
        return this;
    }

    // front colors
    public CliWriter StyleFrontGray()
    {
        _Content.Append(CliStyleStrings.FrontGray);
        return this;
    }
    public CliWriter StyleFrontRed()
    {
        _Content.Append(CliStyleStrings.FrontRed);
        return this;
    }
    public CliWriter StyleFrontGreen()
    {
        _Content.Append(CliStyleStrings.FrontGreen);
        return this;
    }
    public CliWriter StyleFrontYellow()
    {
        _Content.Append(CliStyleStrings.FrontYellow);
        return this;
    }
    public CliWriter StyleFrontBlue()
    {
        _Content.Append(CliStyleStrings.FrontBlue);
        return this;
    }
    public CliWriter StyleFrontMagenta()
    {
        _Content.Append(CliStyleStrings.FrontMagenta);
        return this;
    }
    public CliWriter StyleFrontCyan()
    {
        _Content.Append(CliStyleStrings.FrontCyan);
        return this;
    }
    public CliWriter StyleFrontWhite()
    {
        _Content.Append(CliStyleStrings.FrontWhite);
        return this;
    }

    // back colors
    public CliWriter StyleBackGray()
    {
        _Content.Append(CliStyleStrings.BackGray);
        return this;
    }
    public CliWriter StyleBackRed()
    {
        _Content.Append(CliStyleStrings.BackRed);
        return this;
    }
    public CliWriter StyleBackGreen()
    {
        _Content.Append(CliStyleStrings.BackGreen);
        return this;
    }
    public CliWriter StyleBackYellow()
    {
        _Content.Append(CliStyleStrings.BackYellow);
        return this;
    }
    public CliWriter StyleBackBlue()
    {
        _Content.Append(CliStyleStrings.BackBlue);
        return this;
    }
    public CliWriter StyleBackMagenta()
    {
        _Content.Append(CliStyleStrings.BackMagenta);
        return this;
    }
    public CliWriter StyleBackCyan()
    {
        _Content.Append(CliStyleStrings.BackCyan);
        return this;
    }
    public CliWriter StyleBackWhite()
    {
        _Content.Append(CliStyleStrings.BackWhite);
        return this;
    }
    #endregion

    #region Build Content
    public CliWriter Write(string content)
    {
        _Content.Append(content);
        _SolveIndent(content, ref _IndentCache);
        return this;
    }
    public CliWriter WriteLine(string content)
    {
        _Content.AppendLine(content);
        _IndentCache = 0;
        return this;
    }
    public CliWriter NextLine()
    {
        _Content.AppendLine();
        _IndentCache = 0;
        return this;
    }
    public CliWriter WriteKeepIndent(string content)
    {
        // solve indent
        uint newIndent = 0;
        _SolveIndent(content, ref newIndent);

        // apply indent
        if (_IndentCache > 0)
        {
            var indentStr = "\n" + new string(' ', (int)_IndentCache);
            content = content.Replace("\n", indentStr);
        }

        // update state
        _Content.Append(content);
        _IndentCache += newIndent;
        return this;
    }
    #endregion

    #region Output
    public void Dump()
    {
        Console.Write(_Content.ToString());
    }
    #endregion

    #region Helpers
    private void _SolveIndent(string str, ref uint indent)
    {
        var lastNewLine = str.LastIndexOf('\n');
        if (lastNewLine >= 0)
        {
            indent = (uint)(str.Length - lastNewLine - 1);
        }
        else
        {
            indent += (uint)str.Length;
        }
    }
    #endregion


    private StringBuilder _Content = new StringBuilder();
    private uint _IndentCache = 0;
};
