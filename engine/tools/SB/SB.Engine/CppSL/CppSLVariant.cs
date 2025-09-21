using System.Security.Cryptography;
using System.Text;
using System.Text.Json.Serialization;

namespace SB
{
    public enum CppSLMacroType
    {
        Level,
        Value,
        Select
    }

    public class CppSLVariantOption
    {
        public CppSLVariantOption(CppSLMacroType Type, string Key, List<string> ValueSelections)
        {
            this.Key = Key;
            this.Type = Type;
            this.ValueSelections = ValueSelections;
        }
        public string Key { get; set; } = "";
        
        public CppSLMacroType Type { get; set; }
        public List<string> ValueSelections { get; set; } = new();
    }

    public class CppSLVariantTemplate
    {
        public CppSLVariantTemplate(string Name)
        {
            this.Name = Name;
        }

        public CppSLVariantTemplate Level(string Key, params string[] ValueSelections)
        {
            Options.Add(new(CppSLMacroType.Level, Key, ValueSelections.ToList()));
            return this;
        }

        public CppSLVariantTemplate Value(string Key, params string[] ValueSelections)
        {
            Options.Add(new(CppSLMacroType.Value, Key, ValueSelections.ToList()));
            return this;
        }

        public CppSLVariantTemplate Switch(string Key)
        {
            return this.Value(Key, "off", "on");
        }

        public CppSLVariantTemplate Select(string Key, params string[] ValueSelections)
        {
            Options.Add(new(CppSLMacroType.Select, Key, ValueSelections.ToList()));
            return this;
        }
        
        public Dictionary<string, Dictionary<CppSLVariantOption, string>> GetAllVariants()
        {
            Dictionary<string, Dictionary<CppSLVariantOption, string>> Variants = new();
            var AllValueSelections = Options.Select(O => O.ValueSelections);
            foreach (var Cartesian in AllValueSelections.CartesianProduct())
            {
                Dictionary<CppSLVariantOption, string> Defines = new();
                int Index = 0;
                foreach (var V in Cartesian)
                {
                    Defines.Add(Options[Index], V);
                    Index += 1;
                }
                var MD5Code = MD5.HashData(Encoding.UTF8.GetBytes(string.Join("", Defines.Values)));
                Variants.Add(Convert.ToHexString(MD5Code), Defines);
            }
            return Variants;
        }

        public string GetIdentityString()
        {
            var sig = $"{Name}|{Options.Count}|";
            sig += string.Join("|", Options.Select(o => $"{(int)o.Type}:{o.Key}:{o.ValueSelections.Count}:{string.Join(",", o.ValueSelections)}"));
            return sig;
        }

        public string EmitVariantJSON(string OutputDir)
        {
            var jsonPath = Path.Combine(OutputDir, $"{Name}.variants.json");
            var json = Json.Serialize(this);
            File.WriteAllText(jsonPath, json);
            return jsonPath;
        }
        public string Name { get; set; }
        public List<CppSLVariantOption> Options { get; set; } = new();
    }

    internal static class Cartesian
    {
        public static IEnumerable<IEnumerable<T>> CartesianProduct<T>(this IEnumerable<IEnumerable<T>> sequences)
        {
            IEnumerable<IEnumerable<T>> emptyProduct = new[] { Enumerable.Empty<T>() };
            return sequences.Aggregate(
                emptyProduct,
                (accumulator, sequence) =>
                    from accseq in accumulator
                    from item in sequence
                    select accseq.Concat(new[] { item }));
        }
    }
}