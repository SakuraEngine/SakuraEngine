#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"

namespace skr::CppSL
{
const skr::CppSL::TypeDecl* FunctionStack::methodThisType() const
{
    if (auto method = llvm::dyn_cast<clang::CXXMethodDecl>(func))
    {
        if (method->isInstance())
        {
            return pKernelTranslator->getType(method->getThisType()->getPointeeType());
        }
    }
    return nullptr;
}

std::string KernelTranslator::GetFunctionName(const clang::FunctionDecl* func)
{
    // 仅当函数与模板变体有关时才做唯一化
    auto IsTemplateRelated = [](const clang::FunctionDecl* f) -> bool {
        using namespace clang;
        // 函数模板或特化
        if (f->getDescribedFunctionTemplate() != nullptr)
            return true;
        if (f->getTemplateSpecializationInfo() != nullptr)
            return true;
        // 类模板实例中的方法
        if (auto m = llvm::dyn_cast<CXXMethodDecl>(f))
        {
            auto parent = m->getParent();
            if (llvm::isa<ClassTemplateSpecializationDecl>(parent))
                return true;
            if (parent->getDescribedClassTemplate() != nullptr)
                return true;
        }
        return false;
    };

    std::string base = func->getNameAsString();
    if (IsShaderReserveWorld(base))
        base = base + "_f";

    if (!IsTemplateRelated(func))
        return base;

    auto& bucket = func_names_[base];
    auto it = std::find(bucket.begin(), bucket.end(), func);
    size_t index = 0;
    if (it == bucket.end())
    {
        bucket.emplace_back(func);
        index = bucket.size() - 1;
    }
    else
    {
        index = static_cast<size_t>(std::distance(bucket.begin(), it));
    }

    if (index == 0)
        return base;
    return base + "_" + std::to_string(index);
}

void KernelTranslator::CheckStageInputs(const clang::FunctionDecl* x, skr::CppSL::ShaderStage stage)
{
    auto CheckParamIsBuiltin = [](const clang::ParmVarDecl* p) -> bool { return IsBuiltin(p); };
    auto CheckParamTypeIsStageInout = [this](const clang::ParmVarDecl* p) -> bool {
        auto Type = Decay(p->getType()).getTypePtr()->getAsRecordDecl();
        return Type ? (IsStageInout(Type) != nullptr) : false;
    };

    for (auto param : x->parameters())
    {
        bool IsBuiltin = CheckParamIsBuiltin(param);
        bool IsStageInout = CheckParamTypeIsStageInout(param);
        if ((stage == skr::CppSL::ShaderStage::Compute) && !IsBuiltin)
            ReportFatalError(x, "Compute shader function has non-builtin parameter: {}", x->getNameAsString());
        else if ((stage == skr::CppSL::ShaderStage::Vertex || stage == skr::CppSL::ShaderStage::Fragment) && !IsBuiltin && !IsStageInout)
            ReportFatalError(x, "Vertex/Fragment shader function has non-builtin and non-stage-inout parameter: {}", param->getNameAsString());
    }
}

bool KernelTranslator::TranslateStageEntry(const clang::FunctionDecl* x)
{
    bool NeedTranslate = false;
    std::string StageName;
    std::string FunctionName;
    if (auto StageInfo = IsStage(x))
    {
        NeedTranslate = true;
        StageName = GetArgumentAt<clang::StringRef>(StageInfo, 1);
        FunctionName = GetArgumentAt<clang::StringRef>(StageInfo, 2);
    }
    else if (auto KernelInfo = IsKernel(x))
    {
        NeedTranslate = true;
        StageName = "compute";
        FunctionName = x->getNameAsString();
    }
    if (NeedTranslate)
    {
        root_stack = nullptr;
        current_stack = nullptr;

        auto Kernel = TranslateFunction(x, FunctionName);
        if (StageName == "compute")
        {
            if (auto KernelInfo = IsKernel(x))
            {
                CheckStageInputs(x, ShaderStage::Compute);
                Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Compute));

                uint32_t KernelX = GetArgumentAt<uint32_t>(KernelInfo, 1);
                uint32_t KernelY = GetArgumentAt<uint32_t>(KernelInfo, 2);
                uint32_t KernelZ = GetArgumentAt<uint32_t>(KernelInfo, 3);
                Kernel->add_attr(AST.DeclareAttr<KernelSizeAttr>(KernelX, KernelY, KernelZ));
            }
            else
                ReportFatalError("Compute shader function must have kernel size attributes: " + std::string(x->getNameAsString()));
        }
        else if (StageName == "vertex")
        {
            CheckStageInputs(x, ShaderStage::Vertex);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Vertex));
        }
        else if (StageName == "fragment")
        {
            CheckStageInputs(x, ShaderStage::Fragment);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Fragment));
        }
        else if (StageName == "raygeneration")
        {
            CheckStageInputs(x, ShaderStage::RayGen);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::RayGen));
        }
        else if (StageName == "closesthit")
        {
            CheckStageInputs(x, ShaderStage::ClosestHit);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::ClosestHit));
        }
        else if (StageName == "miss")
        {
            CheckStageInputs(x, ShaderStage::Miss);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Miss));
        }
        else if (StageName == "anyhit")
        {
            CheckStageInputs(x, ShaderStage::AnyHit);
            Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::AnyHit));
        }
        else
        {
            ReportFatalError(x, "Unsupported stage function: {}", std::string(x->getNameAsString()));
        }

        // translate noignore functions
        for (auto func : _noignore_funcs)
        {
            TranslateFunction(func);
        }
        return true;
    }
    return false;
}


CppSL::ParamVarDecl* KernelTranslator::TranslateParam(std::vector<CppSL::ParamVarDecl*>& params, skr::CppSL::EVariableQualifier qualifier, clang::QualType type, const skr::CppSL::Name& name)
{
    auto cppslType = getType(type);
    auto _param = AST.DeclareParam(qualifier, cppslType, name);
    params.emplace_back(_param);
    auto NonRefType = type.getNonReferenceType();
    auto CanonType = NonRefType.getCanonicalType();
    auto AsRecordDecl = CanonType->getAsCXXRecordDecl();
    if (AsRecordDecl && AsRecordDecl->isLambda())
    {
        TranslateLambdaCapturesToParams(_lambda_map[AsRecordDecl]);
    }
    return _param;
}

void KernelTranslator::TranslateParams(std::vector<CppSL::ParamVarDecl*>& params, const clang::FunctionDecl* func)
{
    params.reserve(params.size() + func->param_size());

    for (auto param : func->parameters())
    {
        auto iter = _vars.find(param);
        if (iter != _vars.end())
            continue;

        const bool isRef = param->getType()->isReferenceType() && !param->getType()->isRValueReferenceType();
        const auto ParamQualType = param->getType().getNonReferenceType();
        const bool isConst = ParamQualType.isConstQualified();

        const auto qualifier =
            (isRef && isConst)  ? CppSL::EVariableQualifier::Const :
            (isRef && !isConst) ? CppSL::EVariableQualifier::Inout :
                                  CppSL::EVariableQualifier::None;

        if (auto _paramType = getType(ParamQualType))
        {
            auto paramName = param->getName().str();
            if (paramName.empty())
                paramName = std::format("param_{}", param->getFunctionScopeIndex());
            else
                paramName = std::format("{}_{}", paramName, param->getFunctionScopeIndex());

            auto _param = TranslateParam(params, qualifier, ParamQualType, ToText(paramName));
            addVar(param, _param);

            if (auto BuiltinInfo = IsBuiltin(param))
            {
                auto BuiltinName = GetArgumentAt<clang::StringRef>(BuiltinInfo, 1);
                auto SemanticType = AST.GetSemanticTypeFromString(BuiltinName.str().c_str());
                _param->add_attr(AST.DeclareAttr<SemanticAttr>(SemanticType));
            }
        }
        else
        {
            ReportFatalError(param, "Unknown parameter type: {} for parameter: {}", ParamQualType.getAsString(), std::string(param->getName()));
        }
    }

    auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(func);
    if (AsMethod && AsMethod->getParent()->isLambda())
    {
        TranslateLambdaCapturesToParams(_lambda_map[AsMethod->getParent()]);
    }
}

CppSL::FunctionDecl* KernelTranslator::TranslateFunction(const clang::FunctionDecl* x, llvm::StringRef override_name)
{
    if (IsDump(x))
        x->dump();

    if (auto Existed = getFunc(x))
        return Existed;
    if (LanguageRule_UseAssignForImplicitCopyOrMove(x))
        return nullptr;

    auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(x);

    appendStack(x);
    DeferGuard deferGuard([this]() { popStack(); });

    std::string OVERRIDE_NAME = "OP_OVERLOAD";
    if (bool AsOpOverload = LanguageRule_UseMethodForOperatorOverload(x, &OVERRIDE_NAME);
        AsOpOverload && override_name.empty())
    {
        override_name = OVERRIDE_NAME;
    }

    std::vector<CppSL::ParamVarDecl*> params;
    TranslateParams(params, x);
    params.insert(params.end(), current_stack->_captured_params.begin(), current_stack->_captured_params.end());

    CppSL::FunctionDecl* F = nullptr;
    if (AsMethod && !LanguageRule_UseFunctionInsteadOfMethod(AsMethod))
    {
        auto parentType = AsMethod->getParent();
        auto _parentType = getType(parentType->getTypeForDecl()->getCanonicalTypeInternal());
        if (!_parentType)
        {
            ReportFatalError(x, "Method {} has no owner type", AsMethod->getNameAsString());
        }
        else if (auto AsCtor = llvm::dyn_cast<clang::CXXConstructorDecl>(AsMethod))
        {
            if (_parentType->is_builtin())
                return nullptr;

            // Process member initializers
            std::vector<std::pair<CppSL::FieldDecl*, CppSL::Expr*>> inits;
            for (auto ctor_init : AsCtor->inits())
            {
                if (auto F = ctor_init->getMember())
                {
                    auto N = ToText(F->getDeclName().getAsString());
                    auto field = _parentType->get_field(N);
                    auto init_expr = (CppSL::Expr*)TranslateStmt(ctor_init->getInit());
                    inits.emplace_back(field, init_expr);
                }
                else
                {
                    ReportFatalError(x, "Derived class is currently unsupported!");
                }
            }

            // Create constructor
            auto ctor = AST.DeclareConstructor(
                _parentType,
                ConstructorDecl::kSymbolName,
                params,
                TranslateStmt<CppSL::CompoundStmt>(x->getBody())
            );

            for (auto [field, init_expr] : inits)
                ((CppSL::ConstructorDecl*)ctor)->add_member_init(field, init_expr);

            F = ctor;
            _parentType->add_ctor((CppSL::ConstructorDecl*)F);
        }
        else
        {
            auto CxxMethodName = override_name.empty() ? GetFunctionName(AsMethod) : override_name.str();
            auto M = AST.DeclareMethod(
                _parentType,
                ToText(CxxMethodName),
                getType(x->getReturnType()),
                params,
                TranslateStmt<CppSL::CompoundStmt>(x->getBody())
            );
            M->set_const(AsMethod->isConst());
            _parentType->add_method(M);
            F = M;
        }
    }
    else
    {
        auto CxxFunctionName = override_name.empty() ? GetFunctionName(x) : override_name.str();
        if (AsMethod)
        {
            if (!AsMethod->isStatic())
            {
                auto _t = getType(AsMethod->getThisType()->getPointeeType());
                if (_t == nullptr)
                {
                    ReportFatalError(x, "Method {} has no owner type", AsMethod->getNameAsString());
                }
                const auto qualifier = AsMethod->isConst() ? EVariableQualifier::Const : EVariableQualifier::Inout;
                auto _this = AST.DeclareParam(qualifier, _t, L"_this");
                params.emplace(params.begin(), _this);
                current_stack->_this_redirect = _this->ref();
                CxxFunctionName += std::to_string((uint64_t)AsMethod);
            }
            CxxFunctionName += std::to_string((uint64_t)AsMethod);
        }

        F = AST.DeclareFunction(ToText(CxxFunctionName), getType(x->getReturnType()), params, TranslateStmt<CppSL::CompoundStmt>(x->getBody()));

        current_stack->_this_redirect = nullptr;
    }
    addFunc(x, F);
    return F;
}

CppSL::Stmt* KernelTranslator::TranslateCall(const clang::Decl* _funcDecl, const clang::Stmt* callExpr)
{
    auto funcDecl = llvm::dyn_cast<clang::FunctionDecl>(_funcDecl);
    auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(_funcDecl);
    auto AsConstruct = llvm::dyn_cast<clang::CXXConstructExpr>(callExpr);
    auto AsCall = llvm::dyn_cast<clang::CallExpr>(callExpr);
    auto AsMethodCall = llvm::dyn_cast<clang::CXXMemberCallExpr>(callExpr);
    auto AsCXXOperatorCall = llvm::dyn_cast<clang::CXXOperatorCallExpr>(callExpr);
    if (AsConstruct && !getType(AsConstruct->getType()))
    {
        ReportFatalError(AsConstruct, "!");
    }
    auto AsConstructorForBuiltin = AsConstruct && getType(AsConstruct->getType())->is_builtin();

    if (LanguageRule_UseAssignForImplicitCopyOrMove(funcDecl))
        return TranslateStmt(AsConstruct ? AsConstruct->getArg(0) : AsCall->getArg(0));

    // some args carray types that function shall use (like lambdas, etc.)
    // so we translate all args before translate & call the function
    std::vector<CppSL::Expr*> _args;
    _args.reserve(AsCall ? AsCall->getNumArgs() : AsConstruct->getNumArgs());
    for (auto arg : AsCall ? AsCall->arguments() : AsConstruct->arguments())
        _args.emplace_back(TranslateStmt<CppSL::Expr>(arg));

    // translate function declaration
    if (!AsConstructorForBuiltin && !TranslateFunction(llvm::dyn_cast<clang::FunctionDecl>(funcDecl)))
        ReportFatalError(callExpr, "Function declaration failed!");

    // deal with capture-bypasses
    if (_stacks.contains(funcDecl))
    {
        auto& functionStack = _stacks[funcDecl];
        for (auto bypass : functionStack->_captured_params)
        {
            auto info = functionStack->_captured_infos[bypass];
            if (current_stack->_captured_maps.contains(info))
                _args.emplace_back(current_stack->_captured_maps[info]->ref());
            else // capture from stack
            {
                if (auto AsVar = info.asVar)
                {
                    _args.emplace_back(getVar(AsVar)->ref());
                }
                else
                {
                    _args.emplace_back(AST.Field(AST.This(current_stack->methodThisType()), current_stack->methodThisType()->get_field(ToText(info.asCaptureThisField->getName()))));
                }
            }
        }
    }

    if (AsConstruct != nullptr)
    {
        auto CppSLType = getType(AsConstruct->getType());
        return AST.Construct(CppSLType, _args);
    }
    else if (auto AsMethod = clang::dyn_cast<clang::CXXMethodDecl>(funcDecl);
             AsMethod && !LanguageRule_UseFunctionInsteadOfMethod(AsMethod))
    {
        CppSL::MemberExpr* _callee = nullptr;
        if (auto cxxMemberCall = llvm::dyn_cast<clang::CXXMemberCallExpr>(callExpr))
        {
            _callee = TranslateStmt<CppSL::MemberExpr>(cxxMemberCall->getCallee());
        }
        else if (auto cxxOperatorCall = llvm::dyn_cast<clang::CXXOperatorCallExpr>(callExpr))
        {
            auto _caller = TranslateStmt<CppSL::DeclRefExpr>(cxxOperatorCall->getArg(0));
            _args.erase(_args.begin());
            _callee = AST.Method(_caller, (CppSL::MethodDecl*)getFunc(AsMethod));
        }
        else
            ReportFatalError(callExpr, "Unsupported method call expression: {}", callExpr->getStmtClassName());
        return AST.CallMethod(_callee, std::span<CppSL::Expr*>(_args));
    }
    else
    {
        if (AsMethod && LanguageRule_UseFunctionInsteadOfMethod(AsMethod))
        {
            auto _callee = getFunc(AsMethod)->ref();
            if (AsMethodCall)
            {
                _args.emplace(_args.begin(), (CppSL::Expr*)TranslateStmt<CppSL::MemberExpr>(AsMethodCall->getCallee())->owner());
            }
            else if (AsCXXOperatorCall)
            {
                // do nothing because 'this' is already the first argument
            }
            return AST.CallFunction(_callee, _args);
        }
        else
        {
            auto _callee = TranslateStmt<CppSL::DeclRefExpr>(AsCall->getCallee());
            return AST.CallFunction(_callee, _args);
        }
    }
}

bool KernelTranslator::addFunc(const clang::FunctionDecl* func, skr::CppSL::FunctionDecl* decl)
{
    if (!_funcs.emplace(func, decl).second)
    {
        ReportFatalError("Duplicate function declaration: " + std::string(func->getName()));
        return false;
    }
    return true;
}

skr::CppSL::FunctionDecl* KernelTranslator::getFunc(const clang::FunctionDecl* func) const
{
    auto it = _funcs.find(func);
    if (it != _funcs.end())
        return it->second;
    return nullptr;
}

} // namespace skr::CppSL