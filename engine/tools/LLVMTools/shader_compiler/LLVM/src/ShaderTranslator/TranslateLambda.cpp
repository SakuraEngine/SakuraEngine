#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"

namespace skr::CppSL
{
class MemberExprAnalyzer : public clang::RecursiveASTVisitor<MemberExprAnalyzer>
{
public:
    bool VisitMemberExpr(clang::MemberExpr* memberExpr)
    {
        // 检查是否是通过 this 访问的成员
        if (auto base = memberExpr->getBase())
        {
            if (isThisAccess(base))
            {
                if (auto fieldDecl = llvm::dyn_cast<clang::FieldDecl>(memberExpr->getMemberDecl()))
                    member_redirects[fieldDecl].emplace_back(memberExpr);
            }
        }
        return true;
    }
    std::map<const clang::FieldDecl*, std::vector<const clang::MemberExpr*>> member_redirects;

private:
    bool isThisAccess(const clang::Expr* expr)
    {
        if (llvm::isa<clang::CXXThisExpr>(expr))
            return true;
        if (auto implicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(expr))
            return isThisAccess(implicitCast->getSubExpr());
        return false;
    }
};

const CppSL::TypeDecl* KernelTranslator::TranslateLambda(const clang::LambdaExpr* x)
{
    if (!_lambda_proxy)
    {
        auto lambda_proxy = AST.DeclareStructure(std::format(L"lambda_proxy", next_lambda_id++), {});
        lambda_proxy->add_ctor(AST.DeclareConstructor(lambda_proxy, L"lambda_ctor", {}, AST.Block({})));
        _lambda_proxy = lambda_proxy;
    }
    if (!getType(x->getType()))
    {
        addType(x->getType(), _lambda_proxy);
    }
    return _lambda_proxy;
}

void KernelTranslator::TranslateLambdaCapturesToParams(const clang::LambdaExpr* expr)
{
    auto translateCaptureToParam = [&](clang::QualType _type, const CppSL::String& name, bool byref) {
        return TranslateParam(current_stack->_captured_params, byref ? EVariableQualifier::Inout : EVariableQualifier::None, _type, L"cap_" + name);
    };

    current_stack->_captured_params.reserve(expr->capture_size() + current_stack->_captured_params.size()); // reserve space for captures
    for (auto capture : expr->captures())
    {
        bool isThis = capture.capturesThis();
        if (!isThis)
        {
            // 1.1  = 生成传值，& 生成 inout
            auto byRef = capture.getCaptureKind() == clang::LambdaCaptureKind::LCK_ByRef;
            byRef &= !capture.getCapturedVar()->getType().isConstQualified(); // const&
            auto newParam = translateCaptureToParam(
                capture.getCapturedVar()->getType(),
                ToText(capture.getCapturedVar()->getName()),
                byRef
            );
            FunctionStack::CapturedParamInfo info = {
                .owner = expr,
                .asVar = clang::dyn_cast<clang::VarDecl>(capture.getCapturedVar()),
                .asCaptureThisField = nullptr
            };
            current_stack->_captured_infos.emplace(newParam, info);
            current_stack->_captured_maps.emplace(info, newParam);
            current_stack->_value_redirects[clang::dyn_cast<clang::VarDecl>(capture.getCapturedVar())] = newParam;
        }
        else
        {
            // 1.2 this 需要把内部访问到的变量拆解开，再按 1.1 传入
            MemberExprAnalyzer analyzer;
            analyzer.TraverseStmt(expr->getBody());
            for (auto&& [field, exprs] : analyzer.member_redirects)
            {
                auto newParam = translateCaptureToParam(
                    field->getType(),
                    ToText(field->getName()),
                    true
                );
                FunctionStack::CapturedParamInfo info = {
                    .owner = expr,
                    .asVar = nullptr,
                    .asCaptureThisField = field
                };
                current_stack->_captured_infos.emplace(newParam, info);
                current_stack->_captured_maps.emplace(info, newParam);
                for (auto expr : exprs)
                {
                    current_stack->_member_redirects[expr] = newParam->ref();
                }
            }
        }
    }
}

} // namespace skr::CppSL