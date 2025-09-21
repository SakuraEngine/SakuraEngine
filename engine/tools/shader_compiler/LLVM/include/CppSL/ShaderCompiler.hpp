#pragma once
#include <filesystem>

namespace skr::CppSL {
struct AST;

struct ShaderCompiler
{
    static ShaderCompiler* Create(int argc, const char **argv);
    static void Destroy(ShaderCompiler* compiler);

    virtual void SetPermutationId(std::string_view filename) = 0;
    virtual int Run() = 0;
    virtual std::filesystem::path GetSourceFile() const = 0;

    virtual ~ShaderCompiler() = default;
};

} // namespace skr::CppSL