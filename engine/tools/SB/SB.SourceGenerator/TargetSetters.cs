using System;
using Microsoft.CodeAnalysis;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.CodeAnalysis.Text;

namespace SB.Generators
{
    public static class TypeHelper
    {
        public static string GetFullTypeName(this ITypeSymbol Symbol) => Symbol.ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat);
        public static bool GetUnderlyingTypeIfIsArgumentList(this ITypeSymbol Symbol, out ITypeSymbol? ElementType)
        {
            ElementType = null;
            if (Symbol is INamedTypeSymbol)
                if (Symbol.MetadataName.Equals("ArgumentList`1"))
                {
                    ElementType = (Symbol as INamedTypeSymbol)!.TypeArguments[0];
                    return true;
                }
            return false;
        }
    }

    public struct MethodSignatureComparer : IEqualityComparer<IMethodSymbol>
    {
        public bool Equals(IMethodSymbol X, IMethodSymbol Y)
        {
            return X.Name == Y.Name;
        }

        public int GetHashCode(IMethodSymbol X)
        {
            return X.Name.GetHashCode();
        }
    }

    [Generator]
    public class TargetSetterGenerator : IIncrementalGenerator
    {
        public static void RecursiveGetAllTypes(Compilation Compile, INamespaceSymbol Namespace, ref List<INamedTypeSymbol> Types)
        {
            foreach (var Type in Namespace.GetTypeMembers())
            {
                if (SymbolEqualityComparer.Default.Equals(Type.ContainingAssembly, Compile.Assembly))
                    Types.Add(Type);
            }
            foreach (var NS in Namespace.GetNamespaceMembers())
            {
                RecursiveGetAllTypes(Compile, NS, ref Types);
            }
        }

        public void Initialize(IncrementalGeneratorInitializationContext initContext)
        {
            // define the execution pipeline here via a series of transformations:

            // find all additional files that end with .txt
            IncrementalValueProvider<Compilation> AsyncCompile = initContext.CompilationProvider;
            var MethodsProvider = AsyncCompile.Select((Compile, Cancel) =>
            {
                var Methods = new Dictionary<IMethodSymbol, AttributeData>(SymbolEqualityComparer.Default);
                var Types = new List<INamedTypeSymbol>();
                RecursiveGetAllTypes(Compile, Compile.GlobalNamespace, ref Types);
                
                var ArgumentDrivers = Types.Where(T => T.Interfaces.Any(I => I.GetFullTypeName().Equals("global::SB.Core.IArgumentDriver")));
                // sort to avoid IDE bugs
                var SortedDrivers = ArgumentDrivers.ToList();
                SortedDrivers.Sort((A, B) => A.Name.CompareTo(B.Name));
                var ArgumentDriverMembers = SortedDrivers.SelectMany(T => T.GetMembers());
                var ArgumentDriverMethods = ArgumentDriverMembers.Where(M => M.Kind == SymbolKind.Method);
                // sort to avoid IDE bugs
                var SortedMethods = ArgumentDriverMethods.ToList();
                SortedMethods.Sort((A, B) => A.Name.CompareTo(B.Name));
                foreach (var Method in SortedMethods)
                {
                    AttributeData? TargetProperty = null;
                    foreach(var A in Method.GetAttributes())
                        TargetProperty = A.AttributeClass!.GetFullTypeName().Equals("global::SB.Core.TargetProperty") ? A : null;

                    if (TargetProperty != null && !Methods.Any(KVP => KVP.Key.Name == Method.Name))
                    {
                        Methods.Add((IMethodSymbol)Method, TargetProperty);
                    }
                }
                return Methods;
            });
            
            // generate a class that contains their values as const strings
            initContext.RegisterSourceOutput(MethodsProvider, (spc, Methods) =>
            {
                StringBuilder sourceBuilder = new StringBuilder(@"
using SB.Core;

namespace SB
{
    public static partial class ArgumentDictionaryExtensions
    {
");
                foreach (var MethodAndProperty in Methods)
                {
                    var Method = MethodAndProperty.Key;
                    var TargetProperty = MethodAndProperty.Value;
                    var Param = Method.Parameters[0];
                    var MethodName = Method.Name;
                    {
                        bool InheritBehavior = false;
                        bool PathBehavior = false;
                        foreach (var Arg in TargetProperty.NamedArguments)
                        {
                            if (Arg.Key == "InheritBehavior" && Arg.Value.Value!.Equals(true))
                            {
                                InheritBehavior = true;
                                continue;
                            }
                            if (Arg.Key == "PathBehavior" && Arg.Value.Value!.Equals(true))
                            {
                                PathBehavior = true;
                                continue;
                            }
                        }
                        var ArgumentsContainer = InheritBehavior ? "GetArgumentsContainer(Visibility)" : "FinalArguments";
                        var PropertyP = $"params {Param.Type.GetFullTypeName()} {Param.Name}";
                        var ParamP = PathBehavior ? $"@this.HandlePath({Param.Name})" : Param.Name;
                        if (Param.Type.GetUnderlyingTypeIfIsArgumentList(out var ElementType))
                        {
                            sourceBuilder.Append($@"
        public static void {MethodName}(this ArgumentDictionary @this, params {ElementType!.GetFullTypeName()}[] {Param.Name}) {{ @this.AppendToArgumentList<{ElementType!.GetFullTypeName()}>(""{MethodName}"", {ParamP}); }}
");
                        }
                        else
                        {
                            if (InheritBehavior)
                                throw new Exception($"{MethodName} fails: Single param setters should not have inherit behavior!");
                            sourceBuilder.Append($@"
        public static void {MethodName}(this ArgumentDictionary @this, {Param.Type.GetFullTypeName()} {Param.Name}) {{ @this.Override(""{MethodName}"", {ParamP}); }}"
);
                        }
                    }
                }

                sourceBuilder.Append(@"
    }

    public static partial class TargetSettersExtensions
    {
");

                foreach (var MethodAndProperty in Methods)
                {
                    var Method = MethodAndProperty.Key;
                    var TargetProperty = MethodAndProperty.Value;
                    var Param = Method.Parameters[0];
                    var MethodName = Method.Name;
                    {
                        bool InheritBehavior = false;
                        foreach (var Arg in TargetProperty.NamedArguments)
                        {
                            if (Arg.Key == "InheritBehavior" && Arg.Value.Value!.Equals(true))
                            {
                                InheritBehavior = true;
                                break;
                            }
                        }
                        var FlagsP = InheritBehavior ? $"Visibility Visibility, " : "";
                        var ArgumentsContainer = InheritBehavior ? "GetArgumentsContainer(Visibility)" : "PrivateArguments";
                        var PropertyP = $"params {Param.Type.GetFullTypeName()} {Param.Name}";

                        if (Param.Type.GetUnderlyingTypeIfIsArgumentList(out var ElementType))
                        {
                            sourceBuilder.Append($@"
        public static SB.Target {MethodName}(this TargetSetters @this, {FlagsP}params {ElementType!.GetFullTypeName()}[] {Param.Name}) {{ @this.{ArgumentsContainer}.{MethodName}({Param.Name}); return (SB.Target)@this; }}
");
                        }
                        else
                        {
                            if (InheritBehavior)
                                throw new Exception($"{MethodName} fails: Single param setters should not have inherit behavior!");
                            sourceBuilder.Append($@"
        public static SB.Target {MethodName}(this TargetSetters @this, {FlagsP}{Param.Type.GetFullTypeName()} {Param.Name}) {{ @this.{ArgumentsContainer}.{MethodName}({Param.Name}); return (SB.Target)@this; }}"
);
                        }
                    }
                }

                sourceBuilder.Append(@"
    }
}");
                spc.AddSource("TargetSetters.cs", SourceText.From(sourceBuilder.ToString(), Encoding.UTF8));
            });
        }
    }
}