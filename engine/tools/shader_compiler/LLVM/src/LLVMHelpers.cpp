#include <clang/AST/RecursiveASTVisitor.h>
#include "LLVMHelpers.hpp"

namespace skr::CppSL
{

String ToText(clang::StringRef str)
{
    return String(str.begin(), str.end());
}

std::string OpKindToName(clang::OverloadedOperatorKind op)
{
    switch (op)
    {
    case clang::OO_PipeEqual:
        return "operator_pipe_equal";
    case clang::OO_Pipe:
        return "operator_pipe";
    case clang::OO_Amp:
        return "operator_amp";
    case clang::OO_AmpEqual:
        return "operator_amp_assign";
    case clang::OO_Plus:
        return "operator_plus";
    case clang::OO_Minus:
        return "operator_minus";
    case clang::OO_Star:
        return "operator_multiply";
    case clang::OO_Slash:
        return "operator_divide";
    case clang::OO_StarEqual:
        return "operator_multiply_assign";
    case clang::OO_SlashEqual:
        return "operator_divide_assign";
    case clang::OO_PlusEqual:
        return "operator_plus_assign";
    case clang::OO_MinusEqual:
        return "operator_minus_assign";
    case clang::OO_Equal:
        return "operator_equal";
    case clang::OO_EqualEqual:
        return "operator_equalequal";
    case clang::OO_ExclaimEqual:
        return "operator_not_equal";
    case clang::OO_Less:
        return "operator_less";
    case clang::OO_Greater:
        return "operator_greater";
    case clang::OO_LessEqual:
        return "operator_less_equal";
    case clang::OO_GreaterEqual:
        return "operator_greater_equal";
    case clang::OO_Subscript:
        return "operator_subscript";
    case clang::OO_Call:
        return "operator_call";
    default:
        auto message = std::string("Unsupported operator kind: ") + std::to_string(op);
        llvm::report_fatal_error(message.c_str());
        return "operator_unknown";
    }
}

CppSL::UnaryOp TranslateUnaryOp(clang::UnaryOperatorKind op)
{
    switch (op)
    {
    case clang::UO_Plus:
        return CppSL::UnaryOp::PLUS;
    case clang::UO_Minus:
        return CppSL::UnaryOp::MINUS;
    case clang::UO_LNot:
        return CppSL::UnaryOp::NOT;
    case clang::UO_Not:
        return CppSL::UnaryOp::BIT_NOT;

    case clang::UO_PreInc:
        return CppSL::UnaryOp::PRE_INC;
    case clang::UO_PreDec:
        return CppSL::UnaryOp::PRE_DEC;
    case clang::UO_PostInc:
        return CppSL::UnaryOp::POST_INC;
    case clang::UO_PostDec:
        return CppSL::UnaryOp::POST_DEC;
    default:
        llvm::report_fatal_error("Unsupported unary operator");
    }
}

CppSL::BinaryOp TranslateBinaryOp(clang::BinaryOperatorKind op)
{
    switch (op)
    {
    case clang::BO_Add:
        return CppSL::BinaryOp::ADD;
    case clang::BO_Sub:
        return CppSL::BinaryOp::SUB;
    case clang::BO_Mul:
        return CppSL::BinaryOp::MUL;
    case clang::BO_Div:
        return CppSL::BinaryOp::DIV;
    case clang::BO_Rem:
        return CppSL::BinaryOp::MOD;
    case clang::BO_Shl:
        return CppSL::BinaryOp::SHL;
    case clang::BO_Shr:
        return CppSL::BinaryOp::SHR;
    case clang::BO_And:
        return CppSL::BinaryOp::BIT_AND;
    case clang::BO_Or:
        return CppSL::BinaryOp::BIT_OR;
    case clang::BO_Xor:
        return CppSL::BinaryOp::BIT_XOR;
    case clang::BO_LAnd:
        return CppSL::BinaryOp::AND;
    case clang::BO_LOr:
        return CppSL::BinaryOp::OR;

    case clang::BO_LT:
        return CppSL::BinaryOp::LESS;
    case clang::BO_GT:
        return CppSL::BinaryOp::GREATER;
    case clang::BO_LE:
        return CppSL::BinaryOp::LESS_EQUAL;
    case clang::BO_GE:
        return CppSL::BinaryOp::GREATER_EQUAL;
    case clang::BO_EQ:
        return CppSL::BinaryOp::EQUAL;
    case clang::BO_NE:
        return CppSL::BinaryOp::NOT_EQUAL;

    case clang::BO_Assign:
        return CppSL::BinaryOp::ASSIGN;
    case clang::BO_MulAssign:
        return CppSL::BinaryOp::MUL_ASSIGN;
    case clang::BO_DivAssign:
        return CppSL::BinaryOp::DIV_ASSIGN;
    case clang::BO_AddAssign:
        return CppSL::BinaryOp::ADD_ASSIGN;
    case clang::BO_SubAssign:
        return CppSL::BinaryOp::SUB_ASSIGN;
    case clang::BO_ShlAssign:
        return CppSL::BinaryOp::SHL_ASSIGN;
    case clang::BO_ShrAssign:
        return CppSL::BinaryOp::SHR_ASSIGN;
    case clang::BO_AndAssign:
        return CppSL::BinaryOp::AND_ASSIGN;

    case clang::BO_XorAssign:
        return CppSL::BinaryOp::BIT_XOR_ASSIGN;
    case clang::BO_OrAssign:
        return CppSL::BinaryOp::BIT_OR_ASSIGN;
    case clang::BO_RemAssign:
        return CppSL::BinaryOp::MOD_ASSIGN;

    case clang::BO_Comma:
        return CppSL::BinaryOp::COMMA;

    default:
        llvm::report_fatal_error(std::format("Unsupported binary operator {}", (uint32_t)op).c_str());
    }
}

clang::AnnotateAttr* ExistShaderAttrWithName(const clang::Decl* decl, const char* name)
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

const clang::AnnotateAttr* ExistShaderAttrWithName(const clang::AttributedStmt* stmt, const char* name)
{
    auto attrs = stmt->getAttrs();
    for (auto _attr : attrs)
    {
        if (auto attr = clang::dyn_cast<clang::AnnotateAttr>(_attr))
        {
            if (attr->getAnnotation() != "skr-shader" && attr->getAnnotation() != "luisa-shader")
                continue;
            if (GetArgumentAt<clang::StringRef>(attr, 0) == name)
                return attr;
        }
    }
    return nullptr;
}

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
        std::replace(tname.begin(), tname.end(), ':', '_');
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

struct DerefThisScanner : public clang::RecursiveASTVisitor<DerefThisScanner>
{
    bool hasDerefThis = false;
    const clang::CXXRecordDecl* ownerClass = nullptr;
    std::vector<const clang::CXXMethodDecl*>* visited = nullptr;

    bool ContainsDerefThis(const clang::Stmt* stmt)
    {
        if (auto unaryOp = llvm::dyn_cast<clang::UnaryOperator>(stmt))
        {
            if (unaryOp->getOpcode() == clang::UO_Deref)
                if (llvm::isa<clang::CXXThisExpr>(unaryOp->getSubExpr()))
                    return true;
        }
        for (auto child : stmt->children())
        {
            if (child == nullptr)
                continue;
            if (ContainsDerefThis(child))
                return true;
        }
        return false;
    }

    bool VisitStmt(clang::Stmt* expr)
    {
        if (ContainsDerefThis(expr))
        {
            hasDerefThis = true;
            return false; // Stop traversal
        }
        return true;
    }

    bool VisitCallExpr(clang::CallExpr* call)
    {
        if (auto called = clang::dyn_cast<clang::CXXMethodDecl>(call->getCalleeDecl()))
        {
            // 仅同一类传播，且需要有定义体
            if (ownerClass && called->getParent() != ownerClass)
                return true;

            if (auto calledBody = called->getBody())
            {
                if (visited)
                {
                    if (std::find(visited->begin(), visited->end(), called) != visited->end())
                        return true;
                    visited->push_back(called);
                }

                DerefThisScanner scanner;
                scanner.ownerClass = ownerClass;
                scanner.visited = visited;
                scanner.TraverseStmt(calledBody);
                if (scanner.hasDerefThis)
                {
                    hasDerefThis = true;
                    return false;
                }
            }
        }
        return true;
    }
};

bool LanguageRule_UseFunctionInsteadOfMethod(const clang::CXXMethodDecl* Method)
{
    // Original conditions
    if (Method->isStatic() || IsBuiltin(Method->getParent()) || Method->getParent()->isLambda())
    {
        return true;
    }

    if (auto body = Method->getBody())
    {
        // TODO: HLSL does not support *this, we should process these shit in HLSL Generator but not here
        std::vector<const clang::CXXMethodDecl*> visited;
        DerefThisScanner visitor;
        visitor.ownerClass = Method->getParent();
        visitor.visited = &visited;
        visitor.TraverseStmt(body);
        if (visitor.hasDerefThis)
            return true;
    }

    return false;
}

bool IsShaderReserveWorld(const std::string& string)
{
    if (string == "register")
        return true;
    else if (string == "in")
        return true;
    else if (string == "out")
        return true;
    else if (string == "inout")
        return true;
    else if (string == "triangle")
        return true;
    else if (string == "kernel")
        return true;
    else if (string == "sampler")
        return true;
    return false;
}

} // namespace skr::CppSL