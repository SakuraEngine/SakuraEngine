#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"

namespace skr::CppSL
{

CppSL::NamespaceDecl* KernelTranslator::TranslateNamespaceDecl(const clang::NamespaceDecl* namespaceDecl)
{
    using namespace clang;

    if (IsDump(namespaceDecl))
        namespaceDecl->dump();

    // Use canonical declaration to handle namespace redeclarations
    const auto* CanonicalNS = namespaceDecl->getCanonicalDecl();

    // Check if already processed using canonical declaration
    if (auto Existed = _namespaces.find(CanonicalNS); Existed != _namespaces.end())
        return Existed->second;

    if (IsIgnore(namespaceDecl)) return nullptr; // skip ignored namespaces

    // Get namespace name and parent
    auto NamespaceName = CanonicalNS->getName().str();
    CppSL::NamespaceDecl* ParentNamespace = nullptr;

    // Handle nested namespaces using canonical declarations
    if (auto ParentNS = llvm::dyn_cast<clang::NamespaceDecl>(CanonicalNS->getParent()))
    {
        ParentNamespace = TranslateNamespaceDecl(ParentNS);
    }

    // Create new namespace declaration (structure only, content will be assigned later)
    auto NewNamespace = AST.DeclareNamespace(ToText(NamespaceName), ParentNamespace);
    _namespaces[CanonicalNS] = NewNamespace;

    // Process all redeclarations to collect nested namespace structures
    for (auto redecl : CanonicalNS->redecls())
    {
        for (auto subDecl : redecl->decls())
        {
            if (auto SubNamespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(subDecl))
            {
                auto NestedNS = TranslateNamespaceDecl(SubNamespaceDecl);
                if (NestedNS) NewNamespace->add_nested(NestedNS);
            }
        }
    }

    return NewNamespace;
}

const clang::NamespaceDecl* KernelTranslator::GetDeclNamespace(const clang::Decl* decl) const
{
    using namespace clang;

    if (!decl)
        return nullptr;

    // Walk up the declaration context chain to find the namespace
    const DeclContext* ctx = decl->getDeclContext();
    while (ctx && !ctx->isTranslationUnit())
    {
        if (auto nsDecl = dyn_cast<clang::NamespaceDecl>(ctx))
            return nsDecl->getCanonicalDecl(); // Return canonical declaration
        ctx = ctx->getParent();
    }

    return nullptr; // Global scope
}

void KernelTranslator::AssignDeclsToNamespaces()
{
    using namespace clang;

    // Assign types to namespaces
    for (const auto& [clangDecl, cppslType] : _tag_types)
    {
        if (auto nsDecl = GetDeclNamespace(clangDecl))
        {
            if (auto cppslNS = _namespaces.find(nsDecl); cppslNS != _namespaces.end())
            {
                cppslNS->second->add_type(cppslType);
            }
        }
    }

    // Assign functions to namespaces (but exclude methods)
    for (const auto& [clangFunc, cppslFunc] : _funcs)
    {
        const auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(clangFunc);
        const auto FunctionInsteadOfMethod = AsMethod && LanguageRule_UseFunctionInsteadOfMethod(AsMethod);

        // Only assign non-member functions to namespaces
        // Methods belong to their types, not namespaces
        if (!AsMethod || FunctionInsteadOfMethod)
        {
            if (auto nsDecl = GetDeclNamespace(clangFunc))
            {
                if (auto cppslNS = _namespaces.find(nsDecl); cppslNS != _namespaces.end())
                {
                    cppslNS->second->add_function(cppslFunc);
                }
            }
        }
    }

    // Assign global variables to namespaces
    for (const auto& [clangVar, cppslVar] : _vars)
    {
        // Only process global variables (not local/parameter variables)
        if (cppslVar->is_global())
        {
            auto globalVar = static_cast<CppSL::GlobalVarDecl*>(cppslVar);
            if (auto nsDecl = GetDeclNamespace(clangVar))
            {
                if (auto cppslNS = _namespaces.find(nsDecl); cppslNS != _namespaces.end())
                {
                    cppslNS->second->add_global_var(globalVar);
                }
            }
        }
    }

    // Assign enum constants to namespaces
    for (const auto& [clangEnumConst, cppslGlobalVar] : _enum_constants)
    {
        if (auto nsDecl = GetDeclNamespace(clangEnumConst))
        {
            if (auto cppslNS = _namespaces.find(nsDecl); cppslNS != _namespaces.end())
            {
                cppslNS->second->add_global_var(cppslGlobalVar);
            }
        }
    }
}

} // namespace skr::CppSL