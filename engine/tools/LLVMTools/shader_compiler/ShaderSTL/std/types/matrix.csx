using System;
using System.IO;
using System.Linq;
using System.Text;

static class MatrixHeaderGenerator
{
    public static void Main()
    {
        var outputDirectory = Environment.CurrentDirectory;
        var header = GenerateHeader();
        File.WriteAllText(Path.Combine(outputDirectory, "matrix.hxx"), header, new UTF8Encoding(false));
        Console.WriteLine("Generated: matrix.hxx");
    }

    private static string GenerateHeader()
    {
        var sb = new StringBuilder();
        sb.AppendLine("#pragma once");
        sb.AppendLine("#include \"array.hxx\"");
        sb.AppendLine("#include \"vec.hxx\"");
        sb.AppendLine();

        // Generate specializations for all used sizes (2..4 for both axes)
        foreach (var x in new[] { 2, 3, 4 })
        {
            foreach (var y in new[] { 2, 3, 4 })
            {
                AppendMatrixSpecialization(sb, x, y);
                sb.AppendLine();
            }
        }

        // using aliases
        sb.AppendLine($"using float2x2 = matrix<2, 2>;");
        sb.AppendLine($"using float2x3 = matrix<2, 3>;");
        sb.AppendLine($"using float2x4 = matrix<2, 4>;");
        sb.AppendLine();
        sb.AppendLine($"using float3x2 = matrix<3, 2>;");
        sb.AppendLine($"using float3x3 = matrix<3, 3>;");
        sb.AppendLine($"using float3x4 = matrix<3, 4>;");
        sb.AppendLine();
        sb.AppendLine($"using float4x2 = matrix<4, 2>;");
        sb.AppendLine($"using float4x3 = matrix<4, 3>;");
        sb.AppendLine($"using float4x4 = matrix<4, 4>;");
        sb.AppendLine();

        // Keep existing mul declarations as in the reference
        sb.AppendLine("[[binop(\"MUL\")]] float2 mul(float2 v, float2x2 m);");
        sb.AppendLine("[[binop(\"MUL\")]] float3 mul(float3 v, float3x3 m);");
        sb.AppendLine("[[binop(\"MUL\")]] float4 mul(float4 v, float4x4 m);");
        sb.AppendLine();
        sb.AppendLine("[[binop(\"MUL\")]] float2 mul(float2x2 m, float2 v);");
        sb.AppendLine("[[binop(\"MUL\")]] float3 mul(float3x3 m, float3 v);");
        sb.AppendLine("[[binop(\"MUL\")]] float4 mul(float4x4 m, float4 v);");
        sb.AppendLine();
        sb.AppendLine("[[binop(\"MUL\")]] float2x2 mul(float2x2 m1, float2x2 m2);");
        sb.AppendLine("[[binop(\"MUL\")]] float3x3 mul(float3x3 m1, float3x3 m2);");
        sb.AppendLine("[[binop(\"MUL\")]] float4x4 mul(float4x4 m1, float4x4 m2);");

        return sb.ToString();
    }

    private static void AppendMatrixSpecialization(StringBuilder sb, int x, int y)
    {
        string vecType = $"float{y}"; // column vector type has length y

        sb.AppendLine($"template<>");
        sb.AppendLine($"struct [[builtin(\"matrix\")]] matrix<{x}, {y}> {{");
        sb.AppendLine($"    using ThisType = matrix<{x}, {y}>;");
        sb.AppendLine($"    using T = float;");
        sb.AppendLine($"    static constexpr uint32 X = {x};");
        sb.AppendLine($"    static constexpr uint32 Y = {y};");
        sb.AppendLine();

        // column-vector ctor
        sb.Append($"    constexpr matrix(");
        for (int c = 0; c < x; c++)
        {
            sb.Append($"{vecType} col{c}");
            sb.Append(c == x - 1 ? ")\n" : ", ");
        }
        sb.AppendLine($"        : _v({string.Join(", ", Enumerable.Range(0, x).Select(c => $"col{c}"))}) {{}}");

        // scalar ctor (column-major, default 0.f)
        sb.Append($"    constexpr matrix(");
        for (int c = 0; c < x; c++)
        {
            for (int r = 0; r < y; r++)
            {
                sb.Append($"float m{c}{r} = 0.f");
                bool last = (c == x - 1) && (r == y - 1);
                sb.Append(last ? ")\n" : ", ");
            }
        }
        // initializer: _v(floatY(m00, m01, ...), floatY(m10, m11, ...), ...)
        sb.Append("        : _v(");
        for (int c = 0; c < x; c++)
        {
            sb.Append($"{vecType}(");
            for (int r = 0; r < y; r++)
            {
                sb.Append($"m{c}{r}");
                bool lastInVec = r == y - 1;
                sb.Append(lastInVec ? ")" : ", ");
            }
            bool lastCol = c == x - 1;
            sb.Append(lastCol ? ") {}\n" : ", ");
        }
        sb.AppendLine();

        // identity for square matrices only
        if (x == y)
        {
            sb.AppendLine($"    static constexpr matrix<{x}, {y}> identity() {{");
            sb.AppendLine("        // clang-format off");
            sb.AppendLine($"        return matrix<{x}, {y}>(");
            for (int c = 0; c < x; c++)
            {
                sb.Append("            ");
                for (int r = 0; r < y; r++)
                {
                    string v = (c == r) ? "1.f" : "0.f";
                    bool lastElement = (c == x - 1) && (r == y - 1);
                    bool lastInCol = r == y - 1;
                    sb.Append(v);
                    if (!lastElement) sb.Append(lastInCol ? "," : ", ");
                }
                sb.Append($"  // col {c}\n");
            }
            sb.AppendLine($"        );");
            sb.AppendLine("        // clang-format on");
            sb.AppendLine($"    }}");
            sb.AppendLine();
        }

        sb.AppendLine($"#include \"ops/mat_ops.inl\"");
        sb.AppendLine();
        sb.AppendLine($"private:");
        sb.AppendLine($"    // DONT EDIT THIS FIELD LAYOUT");
        sb.AppendLine($"    Array<vec<float, {y}>, {x}> _v;");
        sb.AppendLine($"}};");
    }
}

MatrixHeaderGenerator.Main();


