#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"
#include "InitListAnalyzer.hpp"

namespace skr::CppSL
{

Stmt* KernelTranslator::TranslateStmt(const clang::Stmt* x)
{
    using namespace clang;
    using namespace skr;

    if (x == nullptr)
        return nullptr;

    if (auto stmtWithAttr = llvm::dyn_cast<clang::AttributedStmt>(x))
    {
        auto stmt = TranslateStmt(stmtWithAttr->getSubStmt());
        for (const auto* attr : stmtWithAttr->getAttrs())
        {
            if (auto* loopHint = llvm::dyn_cast<clang::LoopHintAttr>(attr))
            {
                auto option = loopHint->getOption();
                if (option == clang::LoopHintAttr::Unroll || option == clang::LoopHintAttr::UnrollCount)
                {
                    auto state = loopHint->getState();
                    if (state == clang::LoopHintAttr::Disable)
                    {
                        stmt->add_attr(AST.DeclareAttr<LoopAttr>());
                    }
                    else if (state == clang::LoopHintAttr::Enable)
                    {
                        stmt->add_attr(AST.DeclareAttr<UnrollAttr>(UINT32_MAX));
                    }
                    else if (state == clang::LoopHintAttr::Numeric)
                    {
                        auto count = loopHint->getValue()->EvaluateKnownConstInt(*pASTContext);
                        stmt->add_attr(AST.DeclareAttr<UnrollAttr>(count.getLimitedValue()));
                    }
                }
            }
        }
        return stmt;
    }
    else if (auto cxxBranch = llvm::dyn_cast<clang::IfStmt>(x))
    {
        CppSL::Stmt* InnerVar = nullptr;
        if (auto innerVar = cxxBranch->getConditionVariableDeclStmt())
        {
            InnerVar = TranslateStmt(innerVar);
        }

        auto cxxCond = cxxBranch->getCond();
        auto ifConstVar = cxxCond->getIntegerConstantExpr(*pASTContext);
        if (ifConstVar)
        {
            if (ifConstVar->getExtValue() != 0)
            {
                if (cxxBranch->getThen())
                {
                    if (InnerVar)
                        return AST.Block({ InnerVar, TranslateStmt(cxxBranch->getThen()) });
                    else
                        return TranslateStmt(cxxBranch->getThen());
                }
                else
                    return AST.Comment(L"c++: here is an optimized if constexpr false branch");
            }
            else
            {
                if (cxxBranch->getElse())
                {
                    if (InnerVar)
                        return AST.Block({ InnerVar, TranslateStmt(cxxBranch->getElse()) });
                    else
                        return TranslateStmt(cxxBranch->getElse());
                }
                else
                    return AST.Comment(L"c++: here is an optimized if constexpr true branch");
            }
        }
        else
        {
            auto cxxThen = cxxBranch->getThen();
            auto cxxElse = cxxBranch->getElse();
            auto _cond = TranslateStmt<CppSL::Expr>(cxxCond);
            auto _then = TranslateStmt(cxxThen);
            auto _else = TranslateStmt(cxxElse);
            CppSL::CompoundStmt* _then_body = cxxThen ? llvm::dyn_cast<clang::CompoundStmt>(cxxThen) ? (CppSL::CompoundStmt*)_then : AST.Block({ _then }) : nullptr;
            CppSL::CompoundStmt* _else_body = cxxElse ? llvm::dyn_cast<clang::CompoundStmt>(cxxElse) ? (CppSL::CompoundStmt*)_else : AST.Block({ _else }) : nullptr;
            if (InnerVar)
                return AST.Block({ InnerVar, AST.If(_cond, _then_body, _else_body) });
            else
                return AST.If(_cond, _then_body, _else_body);
        }
    }
    else if (auto cxxSwitch = llvm::dyn_cast<clang::SwitchStmt>(x))
    {
        std::vector<CppSL::CaseStmt*> cases;
        std::vector<const clang::SwitchCase*> cxxCases;
        if (auto caseList = cxxSwitch->getSwitchCaseList())
        {
            while (caseList)
            {
                cxxCases.emplace_back(caseList);
                caseList = caseList->getNextSwitchCase();
            }
            std::reverse(cxxCases.begin(), cxxCases.end());
            cases.reserve(cxxCases.size());
            for (auto cxxCase : cxxCases)
                cases.emplace_back(TranslateStmt<CppSL::CaseStmt>(cxxCase));
        }
        return AST.Switch(TranslateStmt<CppSL::Expr>(cxxSwitch->getCond()), cases);
    }
    else if (auto cxxCase = llvm::dyn_cast<clang::CaseStmt>(x))
    {
        return AST.Case(TranslateStmt<CppSL::Expr>(cxxCase->getLHS()), TranslateStmt<CppSL::CompoundStmt>(cxxCase->getSubStmt()));
    }
    else if (auto cxxDefault = llvm::dyn_cast<clang::DefaultStmt>(x))
    {
        auto _body = TranslateStmt<CppSL::CompoundStmt>(cxxDefault->getSubStmt());
        return AST.Default(_body);
    }
    else if (auto cxxContinue = llvm::dyn_cast<clang::ContinueStmt>(x))
    {
        return AST.Continue();
    }
    else if (auto offset_of = llvm::dyn_cast<clang::OffsetOfExpr>(x))
    {
        auto&& location = offset_of->getBeginLoc();
        auto&& fields = offset_of->getComponent(0).getField()->getParent()->fields();
        auto field_idx = offset_of->getComponent(0).getField()->getFieldIndex();
        auto iter = fields.begin();
        uint32_t offset = 0;
        for (uint32_t i = 0; i < field_idx; ++i)
        {
            auto astType = getType(iter->getType());
            auto align = astType->alignment() - 1;
            offset = (offset + align) & (~align);
            offset += astType->size();
            iter++;
        }
        {
            auto astType = getType(iter->getType());
            auto align = astType->alignment() - 1;
            offset = (offset + align) & (~align);
        }
        return AST.Constant(IntValue(offset));
    }
    else if (auto cxxBreak = llvm::dyn_cast<clang::BreakStmt>(x))
    {
        return AST.Break();
    }
    else if (auto cxxWhile = llvm::dyn_cast<clang::WhileStmt>(x))
    {
        if (auto innerVar = cxxWhile->getConditionVariableDeclStmt())
            return TranslateStmt(innerVar);

        auto _cond = TranslateStmt<CppSL::Expr>(cxxWhile->getCond());
        return AST.While(_cond, TranslateStmt<CppSL::CompoundStmt>(cxxWhile->getBody()));
    }
    else if (auto cxxFor = llvm::dyn_cast<clang::ForStmt>(x))
    {
        auto _init = TranslateStmt(cxxFor->getInit());
        auto _cond = TranslateStmt<CppSL::Expr>(cxxFor->getCond());
        auto _inc = TranslateStmt(cxxFor->getInc());
        auto _body = TranslateStmt<CppSL::CompoundStmt>(cxxFor->getBody());
        return AST.For(_init, _cond, _inc, _body);
    }
    else if (auto cxxCompound = llvm::dyn_cast<clang::CompoundStmt>(x))
    {
        std::vector<CppSL::Stmt*> stmts;
        stmts.reserve(cxxCompound->size());
        for (auto sub : cxxCompound->body())
            stmts.emplace_back(TranslateStmt(sub));
        return AST.Block(std::move(stmts));
    }
    else if (auto substNonType = llvm::dyn_cast<clang::SubstNonTypeTemplateParmExpr>(x))
    {
        return TranslateStmt(substNonType->getReplacement());
    }
    else if (auto cxxExprWithCleanup = llvm::dyn_cast<clang::ExprWithCleanups>(x))
    {
        return TranslateStmt(cxxExprWithCleanup->getSubExpr());
    }
    ///////////////////////////////////// STMTS ///////////////////////////////////////////
    else if (auto cxxDecl = llvm::dyn_cast<clang::DeclStmt>(x))
    {
        const DeclGroupRef declGroup = cxxDecl->getDeclGroup();
        std::vector<CppSL::DeclStmt*> var_decls;
        std::vector<CppSL::CommentStmt*> comments;

        for (auto decl : declGroup)
        {
            if (!decl)
                continue;

            if (auto* varDecl = dyn_cast<clang::VarDecl>(decl))
            {
                const auto Ty = varDecl->getType();

                if (Ty->isReferenceType())
                    ReportFatalError(x, "VarDecl as reference type is not supported: [{}]", Ty.getAsString());

                if (auto AsLambda = Ty->getAsRecordDecl(); AsLambda && AsLambda->isLambda())
                {
                    TranslateLambda(clang::dyn_cast<clang::LambdaExpr>(varDecl->getInit()));
                    return AST.Comment(L"c++: this line is a lambda decl");
                }

                const bool isConst = varDecl->getType().isConstQualified();
                if (auto CppSLType = getType(Ty.getCanonicalType()))
                {
                    auto _init = TranslateStmt<CppSL::Expr>(varDecl->getInit());
                    auto _name = CppSL::String(varDecl->getName().begin(), varDecl->getName().end());
                    if (_name.empty())
                    {
                        _name = L"anonymous" + std::to_wstring(next_anonymous_id++);
                    }
                    auto v = AST.Variable(isConst ? CppSL::EVariableQualifier::Const : CppSL::EVariableQualifier::None, CppSLType, _name, _init);
                    addVar(varDecl, (CppSL::VarDecl*)v->decl());
                    var_decls.emplace_back(v);
                }
                else
                {
                    ReportFatalError("VarDecl with unfound type: [{}]", Ty.getAsString());
                }
            }
            else if (auto aliasDecl = dyn_cast<clang::TypeAliasDecl>(decl))
            { // ignore
                comments.emplace_back(AST.Comment(L"c++: this line is a typedef"));
            }
            else if (auto staticAssertDecl = dyn_cast<clang::StaticAssertDecl>(decl))
            { // ignore
                comments.emplace_back(AST.Comment(L"c++: this line is a static_assert"));
            }
            else if (auto UsingDirectiveDecl = dyn_cast<clang::UsingDirectiveDecl>(decl))
            {
                comments.emplace_back(AST.Comment(L"c++: this line is a using decl"));
            }
            else
            {
                ReportFatalError(x, "unsupported decl stmt: {}", cxxDecl->getStmtClassName());
            }
        }
        if (var_decls.size() == 1)
            return var_decls[0]; // single variable declaration, return it directly
        else if (var_decls.size() > 1)
            return AST.DeclGroup(var_decls);
        else if (comments.size() > 0)
            return comments[0];
        else
            return AST.Comment(L"c++: this line is a decl stmt with no variables");
    }
    else if (auto cxxReturn = llvm::dyn_cast<clang::ReturnStmt>(x))
    {
        if (auto retExpr = cxxReturn->getRetValue())
            return AST.Return(TranslateStmt<CppSL::Expr>(retExpr));
        return AST.Return(nullptr);
    }
    ///////////////////////////////////// EXPRS ///////////////////////////////////////////
    else if (auto cxxDeclRef = llvm::dyn_cast<clang::DeclRefExpr>(x))
    {
        auto _cxxDecl = cxxDeclRef->getDecl();
        if (auto Function = llvm::dyn_cast<clang::FunctionDecl>(_cxxDecl))
        {
            return AST.Ref(getFunc(Function));
        }
        else if (auto Binding = llvm::dyn_cast<clang::BindingDecl>(_cxxDecl))
        {
            return TranslateStmt(Binding->getBinding());
        }
        else if (auto Var = llvm::dyn_cast<clang::VarDecl>(_cxxDecl))
        {
            const bool NonOdrUse = cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Unevaluated || cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Discarded;
            if (NonOdrUse)
            {
                if (!_vars.contains(Var))
                {
                    if (auto NewGlobal = TranslateGlobalVariable(Var))
                    {
                        return AST.Ref(NewGlobal);
                    }
                    else if (Var->isConstexpr())
                    {
                        if (auto Decompressed = Var->getPotentiallyDecomposedVarDecl())
                        {
                            if (auto existed = getVar(Decompressed))
                            {
                                return AST.Ref(existed);
                            }
                            else if (auto Evaluated = Decompressed->getEvaluatedValue())
                            {
                                if (Evaluated->isInt())
                                {
                                    return AST.Constant(IntValue(Evaluated->getInt().getLimitedValue()));
                                }
                                else if (Evaluated->isFloat())
                                {
                                    return AST.Constant(FloatValue(Evaluated->getFloat().convertToDouble()));
                                }
                            }
                        }
                    }
                    ReportFatalError(cxxDeclRef, "Variable {} failed to instantiate", Var->getNameAsString());
                }
                else
                {
                    return AST.Ref(getVar(Var));
                }
            }
        }
        else if (auto EnumConstant = llvm::dyn_cast<clang::EnumConstantDecl>(_cxxDecl))
        {
            return _enum_constants[EnumConstant]->ref();
        }
    }
    else if (auto cxxConditional = llvm::dyn_cast<clang::ConditionalOperator>(x))
    {
        return AST.Conditional(TranslateStmt<CppSL::Expr>(cxxConditional->getCond()), TranslateStmt<CppSL::Expr>(cxxConditional->getTrueExpr()), TranslateStmt<CppSL::Expr>(cxxConditional->getFalseExpr()));
    }
    else if (auto cxxLambda = llvm::dyn_cast<LambdaExpr>(x))
    {
        current_stack->_local_lambdas.insert(cxxLambda);
        if (TranslateLambda(cxxLambda))
        {
            return AST.Construct(getType(cxxLambda->getType()), {});
        }
        return AST.Comment(L"lambda declare here");
    }
    else if (auto cxxParenExpr = llvm::dyn_cast<clang::ParenExpr>(x))
    {
        return TranslateStmt<CppSL::Expr>(cxxParenExpr->getSubExpr());
    }
    else if (auto cxxDefaultArg = llvm::dyn_cast<clang::CXXDefaultArgExpr>(x))
    {
        return TranslateStmt(cxxDefaultArg->getExpr());
    }
    else if (auto cxxTypeExpr = llvm::dyn_cast<clang::UnaryExprOrTypeTraitExpr>(x))
    {
        auto Type = getType(cxxTypeExpr->getArgumentType());
        if (cxxTypeExpr->getKind() == clang::UETT_SizeOf)
            return AST.Constant(IntValue(Type->size()));
        else if (cxxTypeExpr->getKind() == clang::UETT_PreferredAlignOf || cxxTypeExpr->getKind() == clang::UETT_AlignOf)
            return AST.Constant(IntValue(Type->alignment()));
        else
            ReportFatalError(x, "Unsupportted UnaryExprOrTypeTraitExpr!");
    }
    else if (auto cxxExplicitCast = llvm::dyn_cast<clang::ExplicitCastExpr>(x))
    {
        if (cxxExplicitCast->getType()->isFunctionPointerType())
            return TranslateStmt<CppSL::DeclRefExpr>(cxxExplicitCast->getSubExpr());
        auto CppSLType = getType(cxxExplicitCast->getType());
        if (!CppSLType)
            ReportFatalError(cxxExplicitCast, "Explicit cast with unfound type: [{}]", cxxExplicitCast->getType().getAsString());
        return AST.StaticCast(CppSLType, TranslateStmt<CppSL::Expr>(cxxExplicitCast->getSubExpr()));
    }
    else if (auto cxxImplicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(x))
    {
        if (cxxImplicitCast->getCastKind() == clang::CK_ArrayToPointerDecay)
            return TranslateStmt<CppSL::Expr>(cxxImplicitCast->getSubExpr());
        if (cxxImplicitCast->getType()->isFunctionPointerType())
            return TranslateStmt<CppSL::DeclRefExpr>(cxxImplicitCast->getSubExpr());
        auto RHS = TranslateStmt<CppSL::Expr>(cxxImplicitCast->getSubExpr());
        auto CppSLType = getType(cxxImplicitCast->getType());
        if (!CppSLType)
            ReportFatalError(cxxImplicitCast, "Implicit cast with unfound type: [{}]", cxxImplicitCast->getType().getAsString());
        return AST.ImplicitCast(CppSLType, RHS);
    }
    else if (auto cxxConstructor = llvm::dyn_cast<clang::CXXConstructExpr>(x))
    {
        return TranslateCall(cxxConstructor->getConstructor(), x);
    }
    else if (auto ImplicitValueInit = llvm::dyn_cast<clang::ImplicitValueInitExpr>(x))
    {
        // ImplicitValueInitExpr represents default initialization like {} or T()
        auto type = ImplicitValueInit->getType();
        auto typeDecl = getType(type.getCanonicalType());

        // Generate a default construct expression
        return AST.Construct(typeDecl, {});
    }
    else if (auto InitList = llvm::dyn_cast<clang::InitListExpr>(x))
    {
        // 分析 InitList 的语义类型
        auto semantic = AnalyzeInitListSemantic(InitList);

        // 根据语义类型决定如何处理
        switch (semantic)
        {
        case EInitListSemantic::Constructor:
        case EInitListSemantic::Vector:
        case EInitListSemantic::Matrix:
            // 对于构造函数、向量、矩阵，转换为 Construct 表达式
            {
                auto type = getType(InitList->getType());
                if (type)
                {
                    std::vector<CppSL::Expr*> args;
                    for (auto init : InitList->inits())
                        args.emplace_back(TranslateStmt<CppSL::Expr>(init));
                    return AST.Construct(type, args);
                }
            }
            break;

        case EInitListSemantic::DefaultInit:
            // 空初始化列表，生成默认构造
            {
                auto type = getType(InitList->getType());
                if (type)
                    return AST.Construct(type, {});
            }
            break;

        case EInitListSemantic::ScalarWrapper:
            // 单元素标量包装，直接返回内部表达式
            if (InitList->getNumInits() == 1)
                return TranslateStmt<CppSL::Expr>(InitList->getInit(0));
            break;

        case EInitListSemantic::Aggregate:
        case EInitListSemantic::Array:
        case EInitListSemantic::Unknown:
        default:
            // 保持原有的 InitList 处理方式
            {
                std::vector<CppSL::Expr*> exprs;
                for (auto init : InitList->inits())
                    exprs.emplace_back(TranslateStmt<CppSL::Expr>(init));
                return AST.InitList(exprs);
            }
        }

        // 默认处理
        std::vector<CppSL::Expr*> exprs;
        for (auto init : InitList->inits())
            exprs.emplace_back(TranslateStmt<CppSL::Expr>(init));
        return AST.InitList(exprs);
    }
    else if (auto cxxCall = llvm::dyn_cast<clang::CallExpr>(x))
    {
        auto funcDecl = cxxCall->getCalleeDecl();
        if (LanguageRule_UseAssignForImplicitCopyOrMove(cxxCall->getCalleeDecl()))
        {
            auto lhs = TranslateStmt<CppSL::Expr>(cxxCall->getArg(0));
            auto rhs = TranslateStmt<CppSL::Expr>(cxxCall->getArg(1));
            return AST.Assign(lhs, rhs);
        }
        else if (auto AsUnaOp = IsUnaOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsUnaOp, 1);
            if (name == "PLUS")
                return AST.Unary(CppSL::UnaryOp::PLUS, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "MINUS")
                return AST.Unary(CppSL::UnaryOp::MINUS, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "NOT")
                return AST.Unary(CppSL::UnaryOp::NOT, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "BIT_NOT")
                return AST.Unary(CppSL::UnaryOp::BIT_NOT, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "PRE_INC")
                return AST.Unary(CppSL::UnaryOp::PRE_INC, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "PRE_DEC")
                return AST.Unary(CppSL::UnaryOp::PRE_DEC, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "POST_INC")
                return AST.Unary(CppSL::UnaryOp::POST_INC, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            else if (name == "POST_DEC")
                return AST.Unary(CppSL::UnaryOp::POST_DEC, TranslateStmt<CppSL::Expr>(cxxCall->getArg(0)));
            ReportFatalError(x, "Unsupported unary operator: {}", name.str());
        }
        else if (auto AsBinOp = IsBinOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsBinOp, 1);
            auto&& iter = _bin_ops.find(name.str());
            if (iter == _bin_ops.end())
                ReportFatalError(x, "Unsupported binary operator: {}", name.str());
            CppSL::BinaryOp op = iter->second;
            auto lhs = TranslateStmt<CppSL::Expr>(cxxCall->getArg(0));
            auto rhs = TranslateStmt<CppSL::Expr>(cxxCall->getArg(1));
            return AST.Binary(op, lhs, rhs);
        }
        else if (IsAccess(funcDecl))
        {
            if (auto AsMethod = llvm::dyn_cast<clang::CXXMemberCallExpr>(cxxCall))
            {
                auto caller = llvm::dyn_cast<clang::MemberExpr>(AsMethod->getCallee())->getBase();
                return AST.Access(TranslateStmt<CppSL::Expr>(caller), TranslateStmt<CppSL::Expr>(AsMethod->getArg(0)));
            }
            else if (auto AsOperator = llvm::dyn_cast<clang::CXXOperatorCallExpr>(cxxCall))
            {
                return AST.Access(TranslateStmt<CppSL::Expr>(AsOperator->getArg(0)), TranslateStmt<CppSL::Expr>(AsOperator->getArg(1)));
            }
            ReportFatalError(x, "Unsupported access operator on function declaration");
        }
        else if (auto AsCallOp = IsCallOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsCallOp, 1);
            if (auto Intrin = AST.FindIntrinsic(name.str().c_str()))
            {
                const bool IsMethod = llvm::dyn_cast<clang::CXXMemberCallExpr>(cxxCall);
                const TypeDecl* _ret_type = nullptr;
                std::vector<const TypeDecl*> _arg_types;
                std::vector<EVariableQualifier> _arg_qualifiers;
                std::vector<CppSL::Expr*> _args;
                _args.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                _arg_types.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                _arg_qualifiers.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                if (IsMethod)
                {
                    auto _clangMember = llvm::dyn_cast<clang::MemberExpr>(llvm::dyn_cast<clang::CXXMemberCallExpr>(x)->getCallee());
                    auto _caller = TranslateStmt<CppSL::DeclRefExpr>(_clangMember->getBase());
                    _arg_types.emplace_back(_caller->type());
                    _arg_qualifiers.emplace_back(EVariableQualifier::Inout);
                    _args.emplace_back(_caller);
                }
                for (size_t i = 0; i < cxxCall->getNumArgs(); ++i)
                {
                    _arg_types.emplace_back(getType(cxxCall->getArg(i)->getType()));
                    _arg_qualifiers.emplace_back(EVariableQualifier::None);
                    _args.emplace_back(TranslateStmt<CppSL::Expr>(cxxCall->getArg(i)));
                }
                _ret_type = getType(cxxCall->getCallReturnType(*pASTContext));
                // TODO: CACHE THIS
                if (auto Spec = AST.SpecializeTemplateFunction(Intrin, _arg_types, _arg_qualifiers, _ret_type))
                    return AST.CallFunction(Spec->ref(), _args);
                else
                    ReportFatalError(x, "Failed to specialize template function: {}", name.str());
            }
            else
                ReportFatalError(x, "Unsupported call operator: {}", name.str());
        }
        else
        {
            return TranslateCall(funcDecl, x);
        }
    }
    else if (auto cxxUnaryOp = llvm::dyn_cast<clang::UnaryOperator>(x))
    {
        const auto cxxOp = cxxUnaryOp->getOpcode();
        if (cxxOp == clang::UO_Deref)
        {
            if (auto _this = llvm::dyn_cast<CXXThisExpr>(cxxUnaryOp->getSubExpr()))
                return TranslateStmt(cxxUnaryOp->getSubExpr());
            else
                ReportFatalError(x, "Unsupported deref operator on non-'this' expression: {}", cxxUnaryOp->getStmtClassName());
        }
        else
        {
            CppSL::UnaryOp op = TranslateUnaryOp(cxxUnaryOp->getOpcode());
            return AST.Unary(op, TranslateStmt<CppSL::Expr>(cxxUnaryOp->getSubExpr()));
        }
    }
    else if (auto cxxBinOp = llvm::dyn_cast<clang::BinaryOperator>(x))
    {
        CppSL::BinaryOp op = TranslateBinaryOp(cxxBinOp->getOpcode());
        return AST.Binary(op, TranslateStmt<CppSL::Expr>(cxxBinOp->getLHS()), TranslateStmt<CppSL::Expr>(cxxBinOp->getRHS()));
    }
    else if (auto arrayAccess = llvm::dyn_cast<clang::ArraySubscriptExpr>(x))
    {
        auto _array = TranslateStmt<CppSL::Expr>(arrayAccess->getBase());
        auto index = TranslateStmt<CppSL::Expr>(arrayAccess->getIdx());
        return AST.Access(_array, index);
    }
    else if (auto memberExpr = llvm::dyn_cast<clang::MemberExpr>(x))
    {
        auto owner = TranslateStmt<CppSL::DeclRefExpr>(memberExpr->getBase());
        auto memberDecl = memberExpr->getMemberDecl();
        auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(memberDecl);
        auto fieldDecl = llvm::dyn_cast<clang::FieldDecl>(memberDecl);
        if (methodDecl)
        {
            return AST.Method(owner, (CppSL::MethodDecl*)getFunc(methodDecl));
        }
        else if (fieldDecl)
        {
            if (IsSwizzle(fieldDecl))
            {
                auto swizzleResultType = getType(fieldDecl->getType());
                auto swizzleText = fieldDecl->getName();
                uint64_t swizzle_seq[] = { 0u, 0u, 0u, 0u }; /*4*/
                int64_t swizzle_size = 0;
                for (auto iter = swizzleText.begin(); iter != swizzleText.end(); iter++)
                {
                    if (*iter == 'x') swizzle_seq[swizzle_size] = 0u;
                    if (*iter == 'y') swizzle_seq[swizzle_size] = 1u;
                    if (*iter == 'z') swizzle_seq[swizzle_size] = 2u;
                    if (*iter == 'w') swizzle_seq[swizzle_size] = 3u;

                    if (*iter == 'r') swizzle_seq[swizzle_size] = 0u;
                    if (*iter == 'g') swizzle_seq[swizzle_size] = 1u;
                    if (*iter == 'b') swizzle_seq[swizzle_size] = 2u;
                    if (*iter == 'a') swizzle_seq[swizzle_size] = 3u;

                    swizzle_size += 1;
                }
                return AST.Swizzle(owner, swizzleResultType, swizzle_size, swizzle_seq);
            }
            else if (!fieldDecl->isAnonymousStructOrUnion())
            {
                auto ownerType = getType(fieldDecl->getParent()->getTypeForDecl()->getCanonicalTypeInternal());
                if (!ownerType)
                    ReportFatalError(x, "Member expr with unfound owner type: [{}]", memberExpr->getBase()->getType().getAsString());
                auto memberName = ToText(memberExpr->getMemberNameInfo().getName().getAsString());
                if (memberName.empty())
                    ReportFatalError(x, "Member name is empty in member expr: {}", memberExpr->getStmtClassName());
                if (current_stack->_member_redirects.contains(memberExpr))
                    return current_stack->_member_redirects[memberExpr]; // lambda expr redirect
                if (auto field = ownerType->get_field(memberName))
                    return AST.Field(owner, field);
                else
                    ReportFatalError(x, "Member field is empty in member expr: {}", memberExpr->getStmtClassName());
            }
            else
            {
                return owner;
            }
        }
        else
        {
            ReportFatalError(x, "unsupported member expr: {}", memberExpr->getStmtClassName());
        }
    }
    else if (auto matTemp = llvm::dyn_cast<clang::MaterializeTemporaryExpr>(x))
    {
        return TranslateStmt(matTemp->getSubExpr());
    }
    else if (auto THIS = llvm::dyn_cast<clang::CXXThisExpr>(x))
    {
        if (current_stack->_this_redirect)
            return current_stack->_this_redirect;
        return AST.This(getType(THIS->getType().getCanonicalType()));
    }
    else if (auto InitExpr = llvm::dyn_cast<CXXDefaultInitExpr>(x))
    {
        return TranslateStmt(InitExpr->getExpr());
    }
    else if (auto CONSTANT = llvm::dyn_cast<clang::ConstantExpr>(x))
    {
        auto APV = CONSTANT->getAPValueResult();
        switch (APV.getKind())
        {
        case clang::APValue::ValueKind::Int:
            return AST.Constant(CppSL::IntValue(APV.getInt().getLimitedValue()));
        case clang::APValue::ValueKind::Float:
            return AST.Constant(CppSL::FloatValue(APV.getFloat().convertToDouble()));
        case clang::APValue::ValueKind::Struct:
        default:
            ReportFatalError(x, "ConstantExpr with struct value is not supported: {}", CONSTANT->getStmtClassName());
        }
    }
    else if (auto SCALAR = llvm::dyn_cast<clang::CXXScalarValueInitExpr>(x))
    {
        auto astType = getType(SCALAR->getTypeSourceInfo()->getType());
        return AST.Construct(astType, {}); // scalar value init expr is just a default constructor call
    }
    else if (auto BOOL = llvm::dyn_cast<clang::CXXBoolLiteralExpr>(x))
    {
        return AST.Constant(CppSL::IntValue(BOOL->getValue()));
    }
    else if (auto INT = llvm::dyn_cast<clang::IntegerLiteral>(x))
    {
        return AST.Constant(CppSL::IntValue(INT->getValue().getLimitedValue()));
    }
    else if (auto FLOAT = llvm::dyn_cast<clang::FloatingLiteral>(x))
    {
        return AST.Constant(CppSL::FloatValue(FLOAT->getValue().convertToFloat()));
    }
    else if (auto cxxNullStmt = llvm::dyn_cast<clang::NullStmt>(x))
    {
        return AST.Block({});
    }

    ReportFatalError(x, "unsupported stmt: {}", x->getStmtClassName());
    return nullptr;
}

}