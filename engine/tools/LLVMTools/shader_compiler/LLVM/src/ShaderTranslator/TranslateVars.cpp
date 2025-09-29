#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"
#include <clang/Lex/Lexer.h>
#include <llvm/Support/raw_ostream.h>

namespace skr::CppSL
{

std::string KernelTranslator::GetVarName(const clang::VarDecl* var)
{
    // 仅当变量与模板变体有关时才做唯一化
    auto IsTemplateRelated = [](const clang::VarDecl* v) -> bool {
        using namespace clang;
        // 变量模板特化
        if (llvm::isa<VarTemplateSpecializationDecl>(v))
            return true;
        // 位于类模板（或其实例化）中的静态成员等
        if (auto ctx = llvm::dyn_cast_or_null<CXXRecordDecl>(v->getDeclContext()))
        {
            if (llvm::isa<ClassTemplateSpecializationDecl>(ctx))
                return true;
            if (ctx->getDescribedClassTemplate() != nullptr)
                return true;
        }
        return false;
    };

    std::string base = var->getNameAsString();
    if (IsShaderReserveWorld(base))
        base = base + "_v";

    if (!IsTemplateRelated(var))
        return base;

    auto& bucket = var_names_[base];
    auto it = std::find(bucket.begin(), bucket.end(), var);
    size_t index = 0;
    if (it == bucket.end())
    {
        bucket.emplace_back(var);
        index = bucket.size() - 1;
    }
    else
    {
        index = static_cast<size_t>(std::distance(bucket.begin(), it));
    }
    return base + "_" + std::to_string(index);
}

enum class ENormFlag
{
    None = 0,
    UNorm = 1,
    SNorm = 2
};

inline static ENormFlag GetTextureNormFlag(const clang::VarDecl* VD, const clang::ASTContext& Ctx)
{
    if (auto* __TSI = VD->getTypeSourceInfo())
    {
        auto __TL = __TSI->getTypeLoc();
        const auto& __SM = Ctx.getSourceManager();
        const auto& __LO = Ctx.getLangOpts();
        auto __B = __TL.getBeginLoc();
        auto __E = clang::Lexer::getLocForEndOfToken(__TL.getEndLoc(), 0, __SM, __LO);
        if (__B.isValid() && __E.isValid())
        {
            auto __T = clang::Lexer::getSourceText(
                clang::CharSourceRange::getCharRange(__B, __E), 
                __SM, __LO
            );
            if (__T.contains("unorm"))
                return ENormFlag::UNorm;
            else if (__T.contains("snorm"))
                return ENormFlag::SNorm;
        }
    }
    return ENormFlag::None;
}

CppSL::GlobalVarDecl* KernelTranslator::TranslateGlobalVariable(const clang::VarDecl* Var)
{
    auto _type = getType(Var->getType());
    if (_type->is_resource())
    {
        if (auto Existed = getVar(Var, false))
            return (CppSL::GlobalVarDecl*)Existed;

        auto ShaderResource = AST.DeclareGlobalResource(getType(Var->getType()), ToText(GetVarName(Var)));
        uint32_t group = ~0, binding = ~0;
        if (auto ResourceBind = IsResourceBind(Var))
        {
            binding = GetArgumentAt<uint32_t>(ResourceBind, 1);
            group = GetArgumentAt<uint32_t>(ResourceBind, 2);
        }
        ShaderResource->add_attr(AST.DeclareAttr<ResourceBindAttr>(group, binding));
        if (auto PushConstant = IsPushConstant(Var))
        {
            ShaderResource->add_attr(AST.DeclareAttr<PushConstantAttr>());
            if (auto cbv = (ConstantBufferTypeDecl*)(_type))
            {
                if (cbv->element_type()->size() > sizeof(uint32_t) * 64)
                {
                    ReportFatalError(Var, "push constant size must be under 64 DWORDs for D3D12 compatibility.");
                }
            }
        }
        if (auto GlobalCoherent = IsGloballyCoherent(Var))
        {
            ShaderResource->add_attr(AST.DeclareAttr<GloballyCoherentAttr>());
        }
        if (auto Decl = Var->getType()->getAsCXXRecordDecl())
        {
            if (clang::AnnotateAttr* BuiltinAttr = IsBuiltin(Decl))
            {
                auto What = GetArgumentAt<clang::StringRef>(BuiltinAttr, 1);
                if (What.starts_with("texture"))
                {
                    // 识别 RWTexture* 等资源的元素类型是否带有 unorm（通过源码拼写层匹配）
                    auto normFlag = GetTextureNormFlag(Var, *pASTContext);
                    if (normFlag != ENormFlag::None)
                    {
                        ShaderResource->add_attr(AST.DeclareAttr<NormFloatAccessAttr>(normFlag == ENormFlag::SNorm));
                    }
                }
            }
        }
        addVar(Var, ShaderResource);
        return ShaderResource;
    }
    else if (!_vars.contains(Var))
    {
        auto _init = TranslateStmt<CppSL::Expr>(Var->getInit());
        if (!getType(Var->getType()))
            TranslateType(Var->getType());

        // groupshared!
        if (IsGroupShared(Var))
        {
            auto _groupshared = AST.DeclareGroupShared(
                getType(Var->getType()),
                ToText(GetVarName(Var)),
                _init
            );
            addVar(Var, _groupshared);
            return _groupshared;
        }
        else
        {
            auto _const = AST.DeclareGlobalConstant(
                getType(Var->getType()),
                ToText(GetVarName(Var)),
                _init
            );
            addVar(Var, _const);
            return _const;
        }
    }
    return nullptr;
}

bool KernelTranslator::addVar(const clang::VarDecl* var, skr::CppSL::VarDecl* _var)
{
    var = var->getCanonicalDecl();
    if (!_vars.emplace(var, _var).second)
    {
        ReportFatalError(var, "Duplicate variable declaration: {}", std::string(var->getName()));
        return false;
    }
    return true;
}

skr::CppSL::VarDecl* KernelTranslator::getVar(const clang::VarDecl* var, bool restrict) const
{
    var = var->getCanonicalDecl();
    if (current_stack && current_stack->_value_redirects.contains(var))
        return current_stack->_value_redirects[var];

    auto it = _vars.find(var);
    if (it != _vars.end())
        return it->second;

    if (restrict)
    {
        ReportFatalError(var, "getVar with unfound variable: [{}]", var->getNameAsString());
    }
    return nullptr;
}

} // namespace skr::CppSL