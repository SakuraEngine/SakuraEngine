#include "CppSL/magic_enum/magic_enum.hpp"
#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <format>
#include <filesystem>

namespace skr::CppSL {
class DeferGuard 
{
public:
    template<typename F>
    DeferGuard(F&& f) : func(std::forward<F>(f)) {}
    
    ~DeferGuard() { func(); }
    
private:
    std::function<void()> func;
};

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
    bool isThisAccess(const clang::Expr* expr) {
        if (llvm::isa<clang::CXXThisExpr>(expr))
            return true;
        if (auto implicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(expr))
            return isThisAccess(implicitCast->getSubExpr());
        return false;
    }
};

inline static std::string OpKindToName(clang::OverloadedOperatorKind kind);

template <typename T>
inline static T GetArgumentAt(const clang::AnnotateAttr* attr, size_t index)
{
    auto args = attr->args_begin() + index;
    if constexpr (std::is_same_v<T, clang::StringRef>)
    {
        auto arg = llvm::dyn_cast<clang::StringLiteral>((*args)->IgnoreParenCasts());
        return arg->getString();
    }
    else if constexpr (std::is_integral_v<T>)
    {
        auto arg = llvm::dyn_cast<clang::IntegerLiteral>((*args)->IgnoreParenCasts());
        return arg->getValue().getLimitedValue();
    }
    else
    {
        static_assert(std::is_same_v<T, std::nullptr_t>, "Unsupported type for GetArgumentAt");
    }
}

inline static clang::AnnotateAttr* ExistShaderAttrWithName(const clang::Decl* decl, const char* name)
{
    auto attrs = decl->specific_attrs<clang::AnnotateAttr>();
    for (auto attr : attrs)
    {
        if (attr->getAnnotation() != "skr-shader" && attr->getAnnotation() != "luisa-shader")
            continue;
        if (GetArgumentAt<clang::StringRef>(attr, 0) == name)
            return attr;
    }
    return nullptr;
}

inline static clang::AnnotateAttr* IsIgnore(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "ignore"); }
inline static clang::AnnotateAttr* IsBuiltin(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "builtin"); }
inline static clang::AnnotateAttr* IsDump(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "dump"); }
inline static clang::AnnotateAttr* IsKernel(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "kernel"); }
inline static clang::AnnotateAttr* IsSwizzle(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "swizzle"); }
inline static clang::AnnotateAttr* IsUnaOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "unaop"); }
inline static clang::AnnotateAttr* IsBinOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "binop"); }
inline static clang::AnnotateAttr* IsCallOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "callop"); }
inline static clang::AnnotateAttr* IsAccess(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "access"); }
inline static clang::AnnotateAttr* IsInterpolation(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "interpolation"); }
inline static clang::AnnotateAttr* IsStage(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "stage"); }
inline static clang::AnnotateAttr* IsStageInout(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "stage_inout"); }
inline static clang::AnnotateAttr* IsResourceBind(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "binding"); }
inline static clang::AnnotateAttr* IsPushConstant(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "push_constant"); }

const bool LanguageRule_UseAssignForImplicitCopyOrMove(const clang::Decl* x)
{
    if (auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(x))
    {
        const bool isImplicit = x->isImplicit();
        bool isCopyOrMove = AsMethod->isCopyAssignmentOperator() || AsMethod->isMoveAssignmentOperator();
        if (auto AsCtor = llvm::dyn_cast<clang::CXXConstructorDecl>(x))
        {
            isCopyOrMove = AsCtor->isCopyConstructor() || AsCtor->isMoveConstructor();
        }
        if (isImplicit && isCopyOrMove)
            return true;
    }
    return false;
}

const bool LanguageRule_UseMethodForOperatorOverload(const clang::Decl* decl, std::string* pReplaceName)
{
    if (auto funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
        if (funcDecl->isOverloadedOperator())
        {
            if (pReplaceName) *pReplaceName = OpKindToName(funcDecl->getOverloadedOperator());
            return true;
        }
    }
    if (auto asConversion = llvm::dyn_cast<clang::CXXConversionDecl>(decl))
    {
        auto tname = asConversion->getReturnType().getAsString();
        std::replace(tname.begin(), tname.end(), ' ', '_');
        std::replace(tname.begin(), tname.end(), '<', '_');
        std::replace(tname.begin(), tname.end(), '>', '_');
        std::replace(tname.begin(), tname.end(), '(', '_');
        std::replace(tname.begin(), tname.end(), ')', '_');
        std::replace(tname.begin(), tname.end(), ',', '_');
        if (pReplaceName) *pReplaceName = "cast_to_" + tname;
        return true;
    }
    return false;
}

bool LanguageRule_BanDoubleFieldsAndVariables(const clang::Decl* decl, const clang::QualType& qt)
{
    if (auto AsBuiltin = qt->getAs<clang::BuiltinType>())
    {
        if (AsBuiltin->getKind() == clang::BuiltinType::Double)
        {
            return false;
        }
    }
    return true;
}

bool LanguageRule_UseFunctionInsteadOfMethod(const clang::CXXMethodDecl* Method)
{
    return Method->isStatic() || IsBuiltin(Method->getParent());
}

const skr::CppSL::TypeDecl* FunctionStack::methodThisType() const
{
    if (auto method = llvm::dyn_cast<clang::CXXMethodDecl>(func))
    {
        if (method->isInstance())
        {
            return pASTConsumer->getType(method->getThisType()->getPointeeType());
        }
    }
    return nullptr;
}

inline void ASTConsumer::ReportFatalError(const std::string& message) const
{
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const
{
    auto message = std::format(_fmt, std::forward<Args>(args)...);
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const
{
    DumpWithLocation(expr);
    ReportFatalError(_fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(const clang::Decl* decl, std::format_string<Args...> _fmt, Args&&... args) const
{
    DumpWithLocation(decl);
    ReportFatalError(_fmt, std::forward<Args>(args)...);
}

void ASTConsumer::DumpWithLocation(const clang::Stmt *stmt) const
{
    stmt->getBeginLoc().dump(pASTContext->getSourceManager());
    stmt->dump();
}

void ASTConsumer::DumpWithLocation(const clang::Decl* decl) const
{
    decl->getBeginLoc().dump(pASTContext->getSourceManager());
    decl->dump();
}

inline static String ToText(clang::StringRef str)
{
    return String(str.begin(), str.end());
}

inline static CppSL::UnaryOp TranslateUnaryOp(clang::UnaryOperatorKind op)
{
    switch (op)
    {
        case clang::UO_Plus: return CppSL::UnaryOp::PLUS;
        case clang::UO_Minus: return CppSL::UnaryOp::MINUS;
        case clang::UO_LNot: return CppSL::UnaryOp::NOT;
        case clang::UO_Not: return CppSL::UnaryOp::BIT_NOT;

        case clang::UO_PreInc: return CppSL::UnaryOp::PRE_INC;
        case clang::UO_PreDec: return CppSL::UnaryOp::PRE_DEC;
        case clang::UO_PostInc: return CppSL::UnaryOp::POST_INC;
        case clang::UO_PostDec: return CppSL::UnaryOp::POST_DEC;
        default:
            llvm::report_fatal_error("Unsupported unary operator");
    }
}

inline static CppSL::BinaryOp TranslateBinaryOp(clang::BinaryOperatorKind op)
{
    switch (op)
    {
        case clang::BO_Add: return CppSL::BinaryOp::ADD;
        case clang::BO_Sub: return CppSL::BinaryOp::SUB;
        case clang::BO_Mul: return CppSL::BinaryOp::MUL;
        case clang::BO_Div: return CppSL::BinaryOp::DIV;
        case clang::BO_Rem: return CppSL::BinaryOp::MOD;
        case clang::BO_Shl: return CppSL::BinaryOp::SHL;
        case clang::BO_Shr: return CppSL::BinaryOp::SHR;
        case clang::BO_And: return CppSL::BinaryOp::BIT_AND;
        case clang::BO_Or: return CppSL::BinaryOp::BIT_OR;
        case clang::BO_Xor: return CppSL::BinaryOp::BIT_XOR;
        case clang::BO_LAnd: return CppSL::BinaryOp::AND;
        case clang::BO_LOr: return CppSL::BinaryOp::OR;

        case clang::BO_LT: return CppSL::BinaryOp::LESS; break;
        case clang::BO_GT: return CppSL::BinaryOp::GREATER; break;
        case clang::BO_LE: return CppSL::BinaryOp::LESS_EQUAL; break;
        case clang::BO_GE: return CppSL::BinaryOp::GREATER_EQUAL; break;
        case clang::BO_EQ: return CppSL::BinaryOp::EQUAL; break;
        case clang::BO_NE: return CppSL::BinaryOp::NOT_EQUAL; break;

        case clang::BO_Assign: return CppSL::BinaryOp::ASSIGN;
        case clang::BO_AddAssign: return CppSL::BinaryOp::ADD_ASSIGN;
        case clang::BO_SubAssign: return CppSL::BinaryOp::SUB_ASSIGN;
        case clang::BO_MulAssign: return CppSL::BinaryOp::MUL_ASSIGN;
        case clang::BO_DivAssign: return CppSL::BinaryOp::DIV_ASSIGN;
        case clang::BO_RemAssign: return CppSL::BinaryOp::MOD_ASSIGN;
        case clang::BO_OrAssign: return CppSL::BinaryOp::BIT_OR_ASSIGN;
        case clang::BO_XorAssign: return CppSL::BinaryOp::BIT_XOR_ASSIGN;
        case clang::BO_ShlAssign: return CppSL::BinaryOp::SHL_ASSIGN;

        default:
            llvm::report_fatal_error("Unsupported binary operator");
    }
}

CompileFrontendAction::CompileFrontendAction(skr::CppSL::AST& AST)
    : clang::ASTFrontendAction(), AST(AST)
{
}

bool CompileFrontendAction::BeginInvocation(clang::CompilerInstance& CI)
{
    clang::DependencyOutputOptions &DepOpts = CI.getInvocation().getDependencyOutputOpts();
        
    const auto& Input = CI.getFrontendOpts().Inputs[0];
    
    std::filesystem::path P = Input.getFile().str();
    auto N = P.filename().replace_extension("").string();
    DepOpts.OutputFile = N + ".d";
    DepOpts.Targets.push_back(N + ".o");
    DepOpts.UsePhonyTargets = false;
    DepOpts.IncludeSystemHeaders = true;
    
    return clang::ASTFrontendAction::BeginInvocation(CI);
}

std::unique_ptr<clang::ASTConsumer> CompileFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile)
{
    auto &LO = CI.getLangOpts();
    LO.CommentOpts.ParseAllComments = false;
    return std::make_unique<skr::CppSL::ASTConsumer>(AST);
}

ASTConsumer::ASTConsumer(skr::CppSL::AST& AST)
    : clang::ASTConsumer(), AST(AST)
{
    for (uint32_t i = 0; i < (uint32_t)BinaryOp::COUNT; i++) 
    {
        const auto op = (BinaryOp)i;
        _bin_ops.emplace(magic_enum::enum_name(op), op);
    }
}

ASTConsumer::~ASTConsumer()
{
    for (auto& [func, stack] : _stacks)
    {
        delete stack;
    }
}

FunctionStack* ASTConsumer::zzNewStack(const clang::FunctionDecl* func)
{
    auto stack = new FunctionStack(func, this);
    _stacks.emplace(func, stack);
    return stack;
}

void ASTConsumer::HandleTranslationUnit(clang::ASTContext& Context)
{
    pASTContext = &Context;
    
    DebugASTVisitor debug = {};
    debug.TraverseDecl(Context.getTranslationUnitDecl());

    // add primitive type mappings
    addType(Context.VoidTy, AST.VoidType);
    addType(Context.BoolTy, AST.BoolType);
    addType(Context.FloatTy, AST.FloatType);
    addType(Context.UnsignedIntTy, AST.UIntType);
    addType(Context.IntTy, AST.IntType);
    addType(Context.DoubleTy, AST.DoubleType);
    addType(Context.UnsignedLongLongTy, AST.U64Type);
    addType(Context.LongLongTy, AST.I64Type);

    // add record types
    TraverseDecl(Context.getTranslationUnitDecl());
}

bool ASTConsumer::VisitEnumDecl(const clang::EnumDecl* enumDecl)
{
    return TranslateEnumDecl(enumDecl);
}

bool ASTConsumer::VisitRecordDecl(const clang::RecordDecl* recordDecl)
{    
    TranslateRecordDecl(recordDecl);
    return true;
}

CppSL::TypeDecl* ASTConsumer::TranslateEnumDecl(const clang::EnumDecl* enumDecl)
{
    using namespace clang;

    if (IsDump(enumDecl))
        enumDecl->dump();

    if (auto Existed = getType(enumDecl->getTypeForDecl()->getCanonicalTypeInternal())) return Existed; // already processed

    auto UnderlyingType = getType(enumDecl->getIntegerType());
    addType(enumDecl->getTypeForDecl()->getCanonicalTypeInternal(), UnderlyingType);

    auto EnumName = enumDecl->getQualifiedNameAsString();
    std::replace(EnumName.begin(), EnumName.end(), ':', '_');
    for (auto E : enumDecl->enumerators())
    {
        const auto I = E->getInitVal().getLimitedValue();
        auto VarName = (EnumName + "__" + E->getName()).str();
        auto _constant = AST.DeclareGlobalConstant(UnderlyingType, ToText(VarName), AST.Constant(IntValue(I)));
        _enum_constants.emplace(E, _constant);
    }
    return UnderlyingType;
}

CppSL::TypeDecl* ASTConsumer::TranslateRecordDecl(const clang::RecordDecl* recordDecl)
{
    using namespace clang;

    if (IsDump(recordDecl))
        recordDecl->dump();

    for (auto subDecl : recordDecl->decls())
    {
        if (auto SubRecordDecl = llvm::dyn_cast<RecordDecl>(subDecl))
            TranslateRecordDecl(SubRecordDecl);
        else if (auto SubEnumDecl = llvm::dyn_cast<EnumDecl>(subDecl))
            TranslateEnumDecl(SubEnumDecl);
    }

    const auto* ThisType = recordDecl->getTypeForDecl();
    const auto ThisQualType = ThisType->getCanonicalTypeInternal();
    const auto* TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl);
    const auto* TSD_Partial = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(recordDecl);
    const auto* TemplateItSelf = recordDecl->getDescribedTemplate();
    if (auto Existed = getType(ThisType->getCanonicalTypeInternal())) return Existed; // already processed
    if (recordDecl->isUnion()) return nullptr; // unions are not supported
    if (IsIgnore(recordDecl)) return nullptr; // skip ignored types
    if (TSD && TSD_Partial) return nullptr; // skip no-def template specs
    
    clang::AnnotateAttr* BuiltinAttr = IsBuiltin(recordDecl);
    if (TSD)
    {
        BuiltinAttr = BuiltinAttr ? BuiltinAttr : IsBuiltin(TSD);
        BuiltinAttr = BuiltinAttr ? BuiltinAttr : IsBuiltin(TSD->getSpecializedTemplate()->getTemplatedDecl());
    }
    if (BuiltinAttr != nullptr)
    {
        auto What = GetArgumentAt<clang::StringRef>(BuiltinAttr, 1);
        if (TSD && What == "vec")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs
            
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType().getCanonicalType();
            const uint64_t N = Arguments.get(1).getAsIntegral().getLimitedValue();

            if (getType(ET) == nullptr)
                ReportFatalError(recordDecl, "Error element type!");
            if (N <= 1 || N > 4) 
                ReportFatalError(TSD, "Unsupported vec size: {}", std::to_string(N));

            if (getType(ET) == AST.FloatType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Float2Type, AST.Float3Type, AST.Float4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.IntType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Int2Type, AST.Int3Type, AST.Int4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.UIntType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.UInt2Type, AST.UInt3Type, AST.UInt4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.BoolType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Bool2Type, AST.Bool3Type, AST.Bool4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else if (getType(ET) == AST.HalfType)
            {
                const skr::CppSL::TypeDecl* Types[] = { AST.Half2Type, AST.Half3Type, AST.Half4Type };
                addType(ThisQualType, Types[N - 2]);
            }
            else
            {
                ReportFatalError(recordDecl, "Unsupported vec type: {} for vec size: {}", std::string(ET->getTypeClassName()), std::to_string(N));
            }
        }
        else if (TSD && What == "array")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs

            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto N = Arguments.get(1).getAsIntegral().getLimitedValue();
            const auto ArrayFlags = Arguments.get(2).getAsIntegral().getLimitedValue();

            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            auto ArrayType = AST.ArrayType(getType(ET), uint32_t(N), (CppSL::ArrayFlags)ArrayFlags);
            addType(ThisQualType, ArrayType);
        }
        else if (TSD && What == "matrix")
        {
            if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs

            const auto& Arguments = TSD->getTemplateArgs();
            const auto N = Arguments.get(0).getAsIntegral().getLimitedValue();
            const skr::CppSL::TypeDecl* Types[] = { AST.Float2x2Type, AST.Float3x3Type, AST.Float4x4Type };
            addType(ThisQualType, Types[N - 2]);
        }
        else if (What == "half")
        {
            addType(ThisQualType, AST.HalfType);
        }
        else if (TSD && What == "buffer")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto CacheFlags = Arguments.get(1).getAsIntegral().getLimitedValue();
            const auto BufferFlag = (CacheFlags == 2) ? CppSL::BufferFlags::Read : CppSL::BufferFlags::ReadWrite;
            
            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            if (ET->isVoidType())
                addType(ThisQualType, AST.ByteBuffer((CppSL::BufferFlags)BufferFlag));
            else
                addType(ThisQualType, AST.StructuredBuffer(getType(ET), (CppSL::BufferFlags)BufferFlag));
        }
        else if (TSD && What == "constant_buffer")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            
            if (getType(ET) == nullptr)
                TranslateType(ET->getCanonicalTypeInternal());

            if (ET->isVoidType())
                ReportFatalError(recordDecl, "Constant buffer cannot be void type!");

            addType(ThisQualType, AST.ConstantBuffer(getType(ET)));
        }
        else if ((TSD && What == "image") || (TSD && What == "volume"))
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto ET = Arguments.get(0).getAsType();
            const auto CacheFlags = Arguments.get(1).getAsIntegral().getLimitedValue();
            const auto TextureFlag = (CacheFlags == 2) ? CppSL::TextureFlags::Read : CppSL::TextureFlags::ReadWrite;
            if (What == "image")
                addType(ThisQualType, AST.Texture2D(getType(ET), (CppSL::TextureFlags)TextureFlag));
            else
                addType(ThisQualType, AST.Texture3D(getType(ET), (CppSL::TextureFlags)TextureFlag));
        }
        else if (What == "sampler")
        {
            addType(ThisQualType, AST.Sampler());
        }
        else if (What == "accel")
        {
            addType(ThisQualType, AST.Accel());
        }
        else if (TSD && What == "ray_query")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto Flags = Arguments.get(0).getAsIntegral().getLimitedValue();
            auto RayQueryFlags = (CppSL::RayQueryFlags)Flags;
            addType(ThisQualType, AST.RayQuery(RayQueryFlags));
        }
        else if (What == "bindless_array")
        {
            addType(ThisQualType, AST.DeclareBuiltinType(L"bindless_array", 0));
        }
    } 
    else 
    {
        if (!recordDecl->isCompleteDefinition()) return nullptr; // skip forward declares
        if (TSD && !TSD->isCompleteDefinition()) return nullptr; // skip no-def template specs
        if (!TSD && TemplateItSelf) return nullptr; // skip template definitions

        auto TypeName = TSD ? std::format("{}_{}", TSD->getQualifiedNameAsString(), next_template_spec_id++) :
                                recordDecl->getQualifiedNameAsString();
        if (getType(ThisQualType))
            ReportFatalError(recordDecl, "Duplicate type declaration: {}", TypeName);

        std::replace(TypeName.begin(), TypeName.end(), ':', '_');
        auto NewType = AST.DeclareStructure(ToText(TypeName), {});
        if (NewType == nullptr)
            ReportFatalError(recordDecl, "Failed to create type: {}", TypeName);

        auto AsStageInout = IsStageInout(recordDecl);
        if (AsStageInout != nullptr)
        {
            NewType->add_attr(AST.DeclareAttr<CppSL::StageInoutAttr>());
        }

        for (auto field : recordDecl->fields())
        {
            if (IsDump(field)) 
                field->dump();

            auto fieldType = field->getType();
            if (field->getType()->isReferenceType() || field->getType()->isPointerType())
            {
                ReportFatalError(field, "Field type cannot be reference or pointer!");
            }

            auto _fieldType = getType(fieldType);
            if (!_fieldType)
            {
                TranslateType(fieldType);
                _fieldType = getType(fieldType);
            }

            if (!_fieldType)
                ReportFatalError(recordDecl, "Unknown field type: {} for field: {}", std::string(fieldType->getTypeClassName()), field->getName().str());

            auto _f = AST.DeclareField(ToText(field->getName()), _fieldType);
            if (auto AsInterpolation = IsInterpolation(field))
            {
                auto InterpolationModeString = GetArgumentAt<clang::StringRef>(AsInterpolation, 1);
                auto InterpolationMode = AST.GetInterpolationModeFromString(InterpolationModeString.str().c_str()); 
                if (AsStageInout != nullptr)
                    _f->add_attr(AST.DeclareAttr<CppSL::InterpolationAttr>(InterpolationMode));
                else
                    ReportFatalError(field, "Interpolation attribute can only be used in stage inout types!");
            }
            NewType->add_field(_f);
        }

        addType(ThisQualType, NewType);
    }
    return getType(ThisQualType);
}

const CppSL::TypeDecl* ASTConsumer::TranslateLambda(const clang::LambdaExpr* x)
{
    if (_lambda_types.contains(x))
        return _lambda_types[x];

    auto lambdaMethod = x->getCallOperator();
    appendStack(x->getCallOperator());
    DeferGuard defer([&]() { popStack(); });

    std::vector<CppSL::ParamVarDecl*> _params;
    TranslateParams(_params, lambdaMethod);
    
    TranslateLambdaCapturesToParams(x);
    _params.insert(_params.end(), current_stack->_captured_params.begin(), current_stack->_captured_params.end());

    auto returnType = lambdaMethod->getReturnType();
    // 需要先翻译 body，因为会展开出潜在的全局变量，如果先翻译类型，可能会导致生成的方法函数体排序在全局变量之前，产生未定义变量错误
    auto lambdaBody = TranslateStmt<CppSL::CompoundStmt>(x->getBody());
    auto lambdaWrapper = AST.DeclareStructure(std::format(L"lambda_{}", next_lambda_id++), {});

    auto newLambda = AST.DeclareMethod(lambdaWrapper, L"operator_call", getType(returnType), _params, lambdaBody);
    lambdaWrapper->add_method(newLambda);
    lambdaWrapper->add_ctor(AST.DeclareConstructor(lambdaWrapper, L"lambda_ctor", {}, AST.Block({})));

    addFunc(lambdaMethod, newLambda);
    addType(x->getLambdaClass()->getTypeForDecl()->getCanonicalTypeInternal(), lambdaWrapper);
    _lambda_types[x] = lambdaWrapper;
    _lambda_wrappers[lambdaWrapper] = x;
    _lambda_methods[lambdaMethod] = x;
    return lambdaWrapper;
}

void ASTConsumer::TranslateLambdaCapturesToParams(const clang::LambdaExpr* x)
{
    auto translateCaptureToParam = [&](CppSL::TypeDecl* _type, const CppSL::String& name, bool byref) 
    {
        return TranslateParam(current_stack->_captured_params, byref ? EVariableQualifier::Inout : EVariableQualifier::None, _type, L"cap_" + name);
    };

    current_stack->_captured_params.reserve(x->capture_size() + current_stack->_captured_params.size()); // reserve space for captures
    for (auto capture : x->captures())
    {
        bool isThis = capture.capturesThis();
        if (!isThis)
        {
            // 1.1  = 生成传值，& 生成 inout
            auto newParam = translateCaptureToParam(
                getType(capture.getCapturedVar()->getType()), 
                ToText(capture.getCapturedVar()->getName()), 
                capture.getCaptureKind() == clang::LambdaCaptureKind::LCK_ByRef
            );
            FunctionStack::CapturedParamInfo info = {
                .owner = x,
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
            analyzer.TraverseStmt(x->getBody());
            for (auto&& [field, exprs] : analyzer.member_redirects)
            {
                auto newParam = translateCaptureToParam(
                    getType(field->getType()),
                    ToText(field->getName()),
                    true
                );
                FunctionStack::CapturedParamInfo info = {
                    .owner = x,
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

CppSL::GlobalVarDecl* ASTConsumer::TranslateGlobalVariable(const clang::VarDecl* Var)
{
    auto _type = getType(Var->getType());
    if (_type->is_resource())
    {
        auto ShaderResource = AST.DeclareGlobalResource(getType(Var->getType()), ToText(Var->getName()));
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
        }
        addVar(Var, ShaderResource);
        return ShaderResource;
    }
    else if (!_vars.contains(Var))
    {
        auto _init = TranslateStmt<CppSL::Expr>(Var->getInit());
        if (!getType(Var->getType()))
            TranslateType(Var->getType());
        auto _const = AST.DeclareGlobalConstant(getType(Var->getType()), ToText(Var->getName()), _init);
        addVar(Var, _const);
        return _const;
    }
    return nullptr;
}

CppSL::Stmt* ASTConsumer::TranslateCall(const clang::Decl* _funcDecl, const clang::Stmt* callExpr)
{
    auto funcDecl = llvm::dyn_cast<clang::FunctionDecl>(_funcDecl);
    auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(_funcDecl);
    auto AsConstruct = llvm::dyn_cast<clang::CXXConstructExpr>(callExpr);
    auto AsCall = llvm::dyn_cast<clang::CallExpr>(callExpr);
    auto AsMethodCall = llvm::dyn_cast<clang::CXXMemberCallExpr>(callExpr);
    auto AsCXXOperatorCall = llvm::dyn_cast<clang::CXXOperatorCallExpr>(callExpr);
    auto AsConstructorForBuiltin = AsConstruct && getType(AsConstruct->getType())->is_builtin();

    if (LanguageRule_UseAssignForImplicitCopyOrMove(funcDecl))
        return TranslateStmt(AsConstruct ? AsConstruct->getArg(0) : AsCall->getArg(0));
    
    // some args carray types that function shall use (like lambdas, etc.)
    // so we translate all args before translate & call the function 
    std::vector<CppSL::Expr*> _args;
    _args.reserve(AsCall ? AsCall->getNumArgs() : AsConstruct->getNumArgs());
    for (auto arg : AsCall ? AsCall->arguments() : AsConstruct->arguments())
        _args.emplace_back(TranslateStmt<CppSL::Expr>(arg));
    if (AsCXXOperatorCall && methodDecl) // op call to methods use first arg as the caller
        _args.erase(_args.begin());

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
                    _args.emplace_back(AST.Field(AST.This(current_stack->methodThisType()), 
                        current_stack->methodThisType()->get_field(ToText(info.asCaptureThisField->getName()))));
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
            _callee = AST.Method(_caller, (CppSL::MethodDecl*)getFunc(AsMethod));
        }
        else
            ReportFatalError(callExpr, "Unsupported method call expression: {}", callExpr->getStmtClassName());
        return AST.CallMethod(_callee, std::span<CppSL::Expr*>(_args));
    }
    else
    {
        if (AsMethod)
        {
            auto _callee = getFunc(AsMethod)->ref();
            _args.emplace(_args.begin(), (CppSL::Expr*)TranslateStmt<CppSL::MemberExpr>(AsMethodCall->getCallee())->owner());
            return AST.CallFunction(_callee, _args);
        }
        else
        {
            auto _callee = TranslateStmt<CppSL::DeclRefExpr>(AsCall->getCallee());
            return AST.CallFunction(_callee, _args); 
        }
    }
}

bool ASTConsumer::VisitFunctionDecl(const clang::FunctionDecl* x)
{
    if (auto StageInfo = IsStage(x))
    {
        root_stack = nullptr;
        current_stack = nullptr;

        auto StageName = GetArgumentAt<clang::StringRef>(StageInfo, 1);
        auto FunctionName = GetArgumentAt<clang::StringRef>(StageInfo, 2);

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
        else
        {
            ReportFatalError(x, "Unsupported stage function: {}", std::string(x->getNameAsString()));
        }
    }
    return true;
}

bool ASTConsumer::VisitFieldDecl(const clang::FieldDecl* x)
{
    if (IsDump(x))
        x->dump();

    if (!LanguageRule_BanDoubleFieldsAndVariables(x, x->getType()))
        ReportFatalError(x, "Double fields are not allowed");

    return true;
}

bool ASTConsumer::VisitVarDecl(const clang::VarDecl* x)
{
    if (IsDump(x))
        x->dump();
    return true;
}

CppSL::TypeDecl* ASTConsumer::TranslateType(clang::QualType type) 
{
    type = Decay(type);
    if (auto Existed = getType(type)) 
        return Existed; // already processed

    if (auto RecordDecl = type->getAsRecordDecl())
    {
        TranslateRecordDecl(RecordDecl);
    }
    else if (auto EnumType = type->getAs<clang::EnumType>())
    {
        TranslateEnumDecl(EnumType->getDecl());
    }
    else if (auto isConstantArrayType = type->isConstantArrayType())
    {
        return getType(type);
    }
    else
    {
        type->dump();
        ReportFatalError("Unsupported type: " + std::string(type->getTypeClassName()));
    }
    
    return getType(type);
}

CppSL::ParamVarDecl* ASTConsumer::TranslateParam(std::vector<CppSL::ParamVarDecl*>& params, skr::CppSL::EVariableQualifier qualifier, const skr::CppSL::TypeDecl* type, const skr::CppSL::Name& name)
{
    auto _param = AST.DeclareParam(qualifier, type, name);
    params.emplace_back(_param);
    if (auto is_lambda = _lambda_wrappers.contains(type))
    {
        TranslateLambdaCapturesToParams(_lambda_wrappers[type]);
    }
    return _param;
}

void ASTConsumer::TranslateParams(std::vector<CppSL::ParamVarDecl*>& params, const clang::FunctionDecl* func)
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
            (isRef && isConst) ? CppSL::EVariableQualifier::Const : 
            (isRef && !isConst) ? CppSL::EVariableQualifier::Inout : 
            CppSL::EVariableQualifier::None;

        if (auto _paramType = getType(ParamQualType))
        {
            auto paramName = param->getName().str();
            if (paramName.empty())
                paramName = std::format("param_{}", param->getFunctionScopeIndex());
            auto _param = TranslateParam(params, qualifier, _paramType, ToText(paramName));
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
}

CppSL::FunctionDecl* ASTConsumer::TranslateFunction(const clang::FunctionDecl *x, llvm::StringRef override_name) 
{
    if (IsDump(x))
        x->dump();
    
    if (auto Existed = getFunc(x))
        return Existed;
    if (LanguageRule_UseAssignForImplicitCopyOrMove(x))
        return nullptr;

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
    auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(x);
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

            auto body = AST.Block({});
            for (auto ctor_init : AsCtor->inits())
            {
                if (auto F = ctor_init->getMember())
                {
                    auto I = ctor_init->getMember()->getFieldIndex();
                    auto N = ToText(F->getDeclName().getAsString());
                    body->add_statement(
                        AST.Assign(
                            AST.Field(AST.This(_parentType), _parentType->get_field(N)),
                            (CppSL::Expr*)TranslateStmt(ctor_init->getInit())
                        )
                    );
                }
                else
                {
                    ReportFatalError(x, "Derived class is currently unsupported!");
                }
            }
            
            if (auto func = TranslateStmt<CppSL::CompoundStmt>(x->getBody()))
                body->add_statement(func);

            F = AST.DeclareConstructor(
                _parentType,
                ConstructorDecl::kSymbolName,
                params,
                body
            );
            _parentType->add_ctor((CppSL::ConstructorDecl*)F);
        }
        else
        {
            auto CxxMethodName = override_name.empty() ? AsMethod->getNameAsString() : override_name.str();
            F = AST.DeclareMethod(
                _parentType,
                ToText(CxxMethodName),
                getType(x->getReturnType()),
                params,
                TranslateStmt<CppSL::CompoundStmt>(x->getBody())
            );
            _parentType->add_method((CppSL::MethodDecl*)F);
        }
    }
    else
    {
        if (AsMethod)
        {
            auto _this = AST.DeclareParam(EVariableQualifier::Inout, getType(AsMethod->getThisType()->getPointeeType()), L"_this");
            params.emplace(params.begin(), _this);
            current_stack->_this_redirect = _this->ref();
        }

        auto CxxFunctionName = override_name.empty() ? x->getQualifiedNameAsString() : override_name.str();
        std::replace(CxxFunctionName.begin(), CxxFunctionName.end(), ':', '_');
        F = AST.DeclareFunction(ToText(CxxFunctionName),
            getType(x->getReturnType()),
            params,
            TranslateStmt<CppSL::CompoundStmt>(x->getBody())
        );
    }
    addFunc(x, F);
    return F;
}

template <typename T>
T* ASTConsumer::TranslateStmt(const clang::Stmt *x) 
{
    return (T*)TranslateStmt(x);
}

Stmt* ASTConsumer::TranslateStmt(const clang::Stmt *x) 
{
    using namespace clang;
    using namespace skr;

    if (x == nullptr) 
        return nullptr;

    if (auto cxxBranch = llvm::dyn_cast<clang::IfStmt>(x)) 
    {
        auto cxxCond = cxxBranch->getCond();
        auto ifConstVar = cxxCond->getIntegerConstantExpr(*pASTContext);
        if (ifConstVar) {
            if (ifConstVar->getExtValue() != 0) {
                if (cxxBranch->getThen())
                    return TranslateStmt(cxxBranch->getThen());
                else
                    return AST.Comment(L"c++: here is an optimized if constexpr false branch");
            } else {
                if (cxxBranch->getElse())
                    return TranslateStmt(cxxBranch->getElse());
                else
                    return AST.Comment(L"c++: here is an optimized if constexpr true branch");
            }
        } else {
            auto cxxThen = cxxBranch->getThen();
            auto cxxElse = cxxBranch->getElse();
            auto _cond = TranslateStmt<CppSL::Expr>(cxxCond);
            auto _then = TranslateStmt(cxxThen);
            auto _else = TranslateStmt(cxxElse);
            CppSL::CompoundStmt* _then_body = cxxThen ? llvm::dyn_cast<clang::CompoundStmt>(cxxThen) ? (CppSL::CompoundStmt*)_then : AST.Block({_then}) : nullptr;
            CppSL::CompoundStmt* _else_body = cxxElse ? llvm::dyn_cast<clang::CompoundStmt>(cxxElse) ? (CppSL::CompoundStmt*)_else : AST.Block({_else}) : nullptr;
            return AST.If(_cond, _then_body, _else_body);
        }
    } 
    else if (auto cxxSwitch = llvm::dyn_cast<clang::SwitchStmt>(x)) 
    {
        std::vector<CppSL::CaseStmt*> cases;
        std::vector<const clang::SwitchCase*> cxxCases;
        if (auto caseList = cxxSwitch->getSwitchCaseList()) 
        {
            while (caseList) {
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
    else if (auto cxxBreak = llvm::dyn_cast<clang::BreakStmt>(x)) 
    {
        return AST.Break();
    } 
    else if (auto cxxWhile = llvm::dyn_cast<clang::WhileStmt>(x)) 
    {
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
            if (!decl) continue;

            if (auto *varDecl = dyn_cast<clang::VarDecl>(decl)) 
            {
                const auto Ty = varDecl->getType();

                if (Ty->isReferenceType())
                    ReportFatalError(x, "VarDecl as reference type is not supported: [{}]", Ty.getAsString());

                if (auto AsLambda = Ty->getAsRecordDecl(); AsLambda && AsLambda->isLambda())
                    TranslateLambda(clang::dyn_cast<clang::LambdaExpr>(varDecl->getInit()));

                const bool isConst = varDecl->getType().isConstQualified();
                if (auto CppSLType = getType(Ty.getCanonicalType()))
                {
                    auto _init = TranslateStmt<CppSL::Expr>(varDecl->getInit());
                    auto _name = CppSL::String(varDecl->getName().begin(), varDecl->getName().end());
                    auto v = AST.Variable(isConst ? CppSL::EVariableQualifier::Const : CppSL::EVariableQualifier::None, CppSLType, _name, _init);
                    addVar(varDecl, (CppSL::VarDecl*)v->decl());
                    var_decls.emplace_back(v); 
                } 
                else
                {
                    ReportFatalError("VarDecl with unfound type: [{}]", Ty.getAsString());
                }
            } else if (auto aliasDecl = dyn_cast<clang::TypeAliasDecl>(decl)) {// ignore
                comments.emplace_back(AST.Comment(L"c++: this line is a typedef"));
            } else if (auto staticAssertDecl = dyn_cast<clang::StaticAssertDecl>(decl)) {// ignore
                comments.emplace_back(AST.Comment(L"c++: this line is a static_assert"));
            } else {
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
        if(auto retExpr = cxxReturn->getRetValue())
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
        return AST.Conditional(TranslateStmt<CppSL::Expr>(cxxConditional->getCond()),
                               TranslateStmt<CppSL::Expr>(cxxConditional->getTrueExpr()),
                               TranslateStmt<CppSL::Expr>(cxxConditional->getFalseExpr()));
    } 
    else if (auto cxxLambda = llvm::dyn_cast<LambdaExpr>(x)) 
    {
        current_stack->_local_lambdas.insert(cxxLambda);
        TranslateLambda(cxxLambda);
        return AST.Construct(getType(cxxLambda->getType()), {});
    }
    else if (auto cxxParenExpr = llvm::dyn_cast<clang::ParenExpr>(x))
    {
        return TranslateStmt<CppSL::Expr>(cxxParenExpr->getSubExpr());
    }
    else if (auto cxxDefaultArg = llvm::dyn_cast<clang::CXXDefaultArgExpr>(x))
    {
        return TranslateStmt(cxxDefaultArg->getExpr());
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
    else if (auto InitList = llvm::dyn_cast<clang::InitListExpr>(x))
    {
        std::vector<CppSL::Expr*> exprs;
        for (auto init : InitList->inits())
        {
            exprs.emplace_back(TranslateStmt<CppSL::Expr>(init));
        }
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
                // TODO: CACHE THIS
                if (auto Spec = AST.SpecializeTemplateFunction(Intrin, _arg_types, _arg_qualifiers))
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
                return AST.This(getType(_this->getType().getCanonicalType())); // deref 'this' (*this)
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
                uint64_t swizzle_seq[] = {0u, 0u, 0u, 0u}; /*4*/
                int64_t swizzle_size = 0;
                for (auto iter = swizzleText.begin(); iter != swizzleText.end(); iter++) {
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
                return AST.Field(owner, ownerType->get_field(memberName));
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

bool ASTConsumer::addVar(const clang::VarDecl* var, skr::CppSL::VarDecl* _var)
{
    if (!_vars.emplace(var, _var).second)
    {
        ReportFatalError(var, "Duplicate variable declaration: {}", std::string(var->getName()));
        return false;
    }
    return true;
}

skr::CppSL::VarDecl* ASTConsumer::getVar(const clang::VarDecl* var) const
{
    if (current_stack && current_stack->_value_redirects.contains(var))
        return current_stack->_value_redirects[var];

    auto it = _vars.find(var);
    if (it != _vars.end())
        return it->second;

    ReportFatalError(var, "getVar with unfound variable: [{}]", var->getNameAsString());
    return nullptr;
}

bool ASTConsumer::addType(clang::QualType type, skr::CppSL::TypeDecl* decl)
{
    type = Decay(type);
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
        {
            ReportFatalError("Duplicate builtin type declaration: {}", std::string(bt->getTypeClassName()));
            return false;
        }
        _builtin_types[kind] = decl;
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
        {
            ReportFatalError(tag, "Duplicate tag type declaration: {}", std::string(tag->getName()));
            return false;
        }
        _tag_types[tag] = decl;
    }
    else
    {
        ReportFatalError("Unknown type declaration: " + std::string(type->getTypeClassName()));
        return false;
    }
    return true;
}

bool ASTConsumer::addType(clang::QualType type, const skr::CppSL::TypeDecl* decl)
{
    return addType(type, const_cast<skr::CppSL::TypeDecl*>(decl));
}

skr::CppSL::TypeDecl* ASTConsumer::getType(clang::QualType type) const
{
    type = Decay(type);
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
            return _builtin_types.at(kind);
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
            return _tag_types.at(tag);
    }
    else if (auto _array = pASTContext->getAsConstantArrayType(type))
    {
        auto ConstantArrayType = pASTContext->getAsConstantArrayType(type);
        return (CppSL::TypeDecl*)AST.ArrayType(
            getType(ConstantArrayType->getElementType()), 
            ConstantArrayType->getSize().getLimitedValue(), 
            ArrayFlags::None
        );
    }
    return nullptr;
}

bool ASTConsumer::addFunc(const clang::FunctionDecl* func, skr::CppSL::FunctionDecl* decl)
{
    if (!_funcs.emplace(func, decl).second)
    {
        ReportFatalError("Duplicate function declaration: " + std::string(func->getName()));
        return false;
    }
    return true;
}

skr::CppSL::FunctionDecl* ASTConsumer::getFunc(const clang::FunctionDecl* func) const
{
    auto it = _funcs.find(func);
    if (it != _funcs.end())
        return it->second;
    return nullptr;
}

clang::QualType ASTConsumer::Decay(clang::QualType type) const
{
    auto _type = type.getNonReferenceType()
        .getUnqualifiedType()
        .getDesugaredType(*pASTContext)
        .getCanonicalType();
    return _type;
}

void ASTConsumer::CheckStageInputs(const clang::FunctionDecl* x, skr::CppSL::ShaderStage stage)
{
    auto CheckParamIsBuiltin = [](const clang::ParmVarDecl* p) -> bool { return IsBuiltin(p); };
    auto CheckParamTypeIsStageInout = [this](const clang::ParmVarDecl* p) -> bool 
    {
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

inline static std::string OpKindToName(clang::OverloadedOperatorKind op)
{
    switch (op) {
        case clang::OO_PipeEqual: return "operator_pipe_equal";
        case clang::OO_Pipe: return "operator_pipe";
        case clang::OO_Amp: return "operator_amp";
        case clang::OO_AmpEqual: return "operator_amp_assign";
        case clang::OO_Plus: return "operator_plus";
        case clang::OO_Minus: return "operator_minus";
        case clang::OO_Star: return "operator_multiply";
        case clang::OO_Slash: return "operator_divide";
        case clang::OO_StarEqual: return "operator_multiply_assign";
        case clang::OO_SlashEqual: return "operator_divide_assign";
        case clang::OO_PlusEqual: return "operator_plus_assign";
        case clang::OO_MinusEqual: return "operator_minus_assign";
        case clang::OO_EqualEqual: return "operator_equal";
        case clang::OO_ExclaimEqual: return "operator_not_equal";
        case clang::OO_Less: return "operator_less";
        case clang::OO_Greater: return "operator_greater";
        case clang::OO_LessEqual: return "operator_less_equal";
        case clang::OO_GreaterEqual: return "operator_greater_equal";
        case clang::OO_Subscript: return "operator_subscript";
        case clang::OO_Call: return "operator_call";
        default: 
            auto message = std::string("Unsupported operator kind: ") + std::to_string(op);
            llvm::report_fatal_error(message.c_str());
            return "operator_unknown";
    }
}

} // namespace skr::CppSL