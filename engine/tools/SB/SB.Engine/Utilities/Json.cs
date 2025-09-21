using System.Collections.Immutable;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.Json.Serialization.Metadata;

namespace SB
{
    [JsonSerializable(typeof(CppSLMacroType))]
    [JsonSerializable(typeof(CppSLVariantOption))]
    [JsonSerializable(typeof(CppSLVariantTemplate))]
    [JsonSerializable(typeof(CodegenRenderConfig))]
    [JsonSerializable(typeof(CodegenGenerator))]
    [JsonSerializable(typeof(CodegenModuleInfo))]
    [JsonSerializable(typeof(List<CodegenModuleInfo>))]
    [JsonSerializable(typeof(List<CodegenGenerator>))]
    internal partial class JsonContext : JsonSerializerContext
    {

    }

    public static class Json
    {
        public static TValue? Deserialize<TValue>(string json)
        {
            var sourceGenOptions = new JsonSerializerOptions
            {
                TypeInfoResolver = JsonContext.Default,
                Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
            };
            return JsonSerializer.Deserialize<TValue>(json, sourceGenOptions);
        }

        public static string Serialize<TValue>(TValue value)
        {
            var sourceGenOptions = new JsonSerializerOptions
            {
                TypeInfoResolver = JsonContext.Default,
                Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
            };
            return JsonSerializer.Serialize(value, typeof(TValue), sourceGenOptions);
        }
    }
}
