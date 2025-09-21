using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Mono.Cecil;
using Mono.Cecil.Cil;

// 测试代码
var sourceCode = @"
using System.Collections.Generic;

System.Console.WriteLine(1 + 2);
System.Console.WriteLine(1 + 2);
var Set = new Dictionary<string, string>();
var Set2 = new string("""");

";

var syntaxTree = CSharpSyntaxTree.ParseText(sourceCode);
var references = new[] {
    MetadataReference.CreateFromFile(typeof(object).Assembly.Location),
    MetadataReference.CreateFromFile(typeof(Console).Assembly.Location),
    MetadataReference.CreateFromFile(typeof(System.Runtime.AssemblyTargetedPatchBandAttribute).Assembly.Location),
    MetadataReference.CreateFromFile(System.Reflection.Assembly.Load("System.Runtime").Location)
};

var compilation = CSharpCompilation.Create(
    "TestAssembly",
    new[] { syntaxTree },
    references,
    new CSharpCompilationOptions(OutputKind.ConsoleApplication));

// 编译到内存流
using var peStream = new MemoryStream();
var emitResult = compilation.Emit(peStream);

if (!emitResult.Success)
{
    Console.WriteLine("编译失败:");
    foreach (var diagnostic in emitResult.Diagnostics.Where(d => d.Severity == DiagnosticSeverity.Error))
    {
        Console.WriteLine($"  {diagnostic}");
    }
}
else
{
    // 重置流位置
    peStream.Position = 0;

    // 使用 Mono.Cecil 读取
    var assembly = AssemblyDefinition.ReadAssembly(peStream);
    
    Console.WriteLine($"程序集: {assembly.Name.Name}");
    
    foreach (var module in assembly.Modules)
    {
        foreach (var type in module.Types)
        {
            // 跳过编译器生成的类型
            if (type.Name.StartsWith("<")) continue;
            
            Console.WriteLine($"\n类型: {type.FullName}");
            
            foreach (var method in type.Methods)
            {
                Console.WriteLine($"  方法: {method.Name}");
                
                if (method.HasBody && method.Body.Instructions.Count > 0)
                {
                    foreach (var instruction in method.Body.Instructions)
                    {
                        string operand = "";
                        if (instruction.Operand != null)
                        {
                            operand = instruction.Operand switch
                            {
                                string s => $"\"{s}\"",
                                MethodReference mr => $"{mr.DeclaringType.Name}::{mr.Name}",
                                FieldReference fr => $"{fr.DeclaringType.Name}::{fr.Name}",
                                TypeReference tr => tr.FullName,
                                Instruction target => $"IL_{target.Offset:X4}",
                                int i => i.ToString(),
                                _ => instruction.Operand?.ToString() ?? ""
                            };
                        }
                        
                        Console.WriteLine($"      IL_{instruction.Offset:X4}: {instruction.OpCode} {operand}");
                    }
                }
            }
        }
    }
}
