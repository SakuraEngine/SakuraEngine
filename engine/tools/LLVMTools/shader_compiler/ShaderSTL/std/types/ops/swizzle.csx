using System.Text;

var outputDirectory = Environment.CurrentDirectory;

var swizzle2 = SwizzleGenerator.Generate2();
var swizzle3 = SwizzleGenerator.Generate3();
var swizzle4 = SwizzleGenerator.Generate4();

File.WriteAllText(Path.Combine(outputDirectory, "swizzle2.inl"), swizzle2, new UTF8Encoding(false));
File.WriteAllText(Path.Combine(outputDirectory, "swizzle3.inl"), swizzle3, new UTF8Encoding(false));
File.WriteAllText(Path.Combine(outputDirectory, "swizzle4.inl"), swizzle4, new UTF8Encoding(false));

Console.WriteLine("Generated: swizzle2.inl, swizzle3.inl, swizzle4.inl");

static class SwizzleGenerator
{
    private const string Indent = "    ";

    public static string Generate2()
    {
        var sb = new StringBuilder();

        // xyz style
        sb.AppendLine("[[swizzle]] T x, y;");
        sb.AppendLine("[[swizzle]] vec<T, 2> ");
        AppendList(sb, new[] { 'x', 'y' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3> ");
        AppendList(sb, new[] { 'x', 'y' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4> ");
        AppendList(sb, new[] { 'x', 'y' }, 4);
        sb.AppendLine();

        // rgba style
        sb.AppendLine("[[swizzle]] T r, g;");
        sb.AppendLine("[[swizzle]] vec<T, 2> ");
        AppendList(sb, new[] { 'r', 'g' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3> ");
        AppendList(sb, new[] { 'r', 'g' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4> ");
        AppendList(sb, new[] { 'r', 'g' }, 4);
        sb.AppendLine();

        return sb.ToString();
    }

    public static string Generate3()
    {
        var sb = new StringBuilder();

        // xyz style
        sb.AppendLine("[[swizzle]] T x, y, z;");
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 2>");
        AppendList(sb, new[] { 'x', 'y', 'z' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3>");
        AppendList(sb, new[] { 'x', 'y', 'z' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4>");
        AppendList(sb, new[] { 'x', 'y', 'z' }, 4);
        sb.AppendLine();

        // rgba style
        sb.AppendLine("[[swizzle]] T r, g, b;");
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 2>");
        AppendList(sb, new[] { 'r', 'g', 'b' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3>");
        AppendList(sb, new[] { 'r', 'g', 'b' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4>");
        AppendList(sb, new[] { 'r', 'g', 'b' }, 4);

        return sb.ToString();
    }

    public static string Generate4()
    {
        var sb = new StringBuilder();

        // xyzw style
        sb.AppendLine("[[swizzle]] T x, y, z, w;");
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 2>");
        AppendList(sb, new[] { 'x', 'y', 'z', 'w' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3>");
        AppendList(sb, new[] { 'x', 'y', 'z', 'w' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4>");
        AppendList(sb, new[] { 'x', 'y', 'z', 'w' }, 4);
        sb.AppendLine();

        // rgba style
        sb.AppendLine("[[swizzle]] T r, g, b, a;");
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 2>");
        AppendList(sb, new[] { 'r', 'g', 'b', 'a' }, 2);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 3>");
        AppendList(sb, new[] { 'r', 'g', 'b', 'a' }, 3);
        sb.AppendLine();
        sb.AppendLine("[[swizzle]] vec<T, 4>");
        AppendList(sb, new[] { 'r', 'g', 'b', 'a' }, 4);

        return sb.ToString();
    }

    private static void AppendList(StringBuilder sb, char[] components, int length)
    {
        var total = (int)Math.Pow(components.Length, length);
        for (int index = 0; index < total; index++)
        {
            var name = BuildName(components, length, index);
            bool isLast = index == total - 1;
            sb.Append(Indent);
            sb.Append('&');
            sb.Append(name);
            sb.Append(isLast ? ';' : ',');
            sb.AppendLine();
        }
    }

    private static string BuildName(char[] components, int length, int index)
    {
        var buffer = new char[length];
        int baseN = components.Length;
        for (int pos = length - 1; pos >= 0; pos--)
        {
            int digit = index % baseN;
            buffer[pos] = components[digit];
            index /= baseN;
        }
        return new string(buffer);
    }
}