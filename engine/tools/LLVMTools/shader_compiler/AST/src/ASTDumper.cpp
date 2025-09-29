#include "CppSL/ASTDumper.hpp"
#include <typeinfo>

namespace skr::CppSL {

void ASTDumper::visit(const skr::CppSL::Stmt* stmt, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    if (auto binary = dynamic_cast<const BinaryExpr*>(stmt))
    {
        auto op = binary->op();
        String op_name = L"";
        switch (op)
        {
        case BinaryOp::ADD:
            op_name = L" + ";
            break;
        case BinaryOp::SUB:
            op_name = L" - ";
            break;
        case BinaryOp::MUL:
            op_name = L" * ";
            break;
        case BinaryOp::DIV:
            op_name = L" / ";
            break;
        case BinaryOp::MOD:
            op_name = L" % ";
            break;

        case BinaryOp::BIT_AND:
            op_name = L" & ";
            break;
        case BinaryOp::BIT_OR:
            op_name = L" | ";
            break;
        case BinaryOp::BIT_XOR:
            op_name = L" ^ ";
            break;
        case BinaryOp::SHL:
            op_name = L" << ";
            break;
        case BinaryOp::SHR:
            op_name = L" >> ";
            break;
        case BinaryOp::AND:
            op_name = L" && ";
            break;
        case BinaryOp::OR:
            op_name = L" || ";
            break;

        case BinaryOp::LESS:
            op_name = L" < ";
            break;
        case BinaryOp::GREATER:
            op_name = L" > ";
            break;
        case BinaryOp::LESS_EQUAL:
            op_name = L" <= ";
            break;
        case BinaryOp::GREATER_EQUAL:
            op_name = L" >= ";
            break;
        case BinaryOp::EQUAL:
            op_name = L" == ";
            break;
        case BinaryOp::NOT_EQUAL:
            op_name = L" != ";
            break;

        case BinaryOp::ASSIGN:
            op_name = L" = ";
            break;
        case BinaryOp::ADD_ASSIGN:
            op_name = L" += ";
            break;
        case BinaryOp::SUB_ASSIGN:
            op_name = L" -= ";
            break;
        case BinaryOp::MUL_ASSIGN:
            op_name = L" *= ";
            break;
        case BinaryOp::DIV_ASSIGN:
            op_name = L" /= ";
            break;
        case BinaryOp::MOD_ASSIGN:
            op_name = L" %= ";
            break;
        case BinaryOp::BIT_OR_ASSIGN:
            op_name = L" |= ";
            break;
        case BinaryOp::BIT_XOR_ASSIGN:
            op_name = L" ^= ";
            break;
        case BinaryOp::SHL_ASSIGN:
            op_name = L" <<= ";
            break;
        default:
            assert(false && "Unsupported binary operation");
        }

        sb.append_expr(L"BinaryExpr ");
        sb.append(op_name);
        sb.endline();
    }
    else if (auto bitwiseCast = dynamic_cast<const BitwiseCastExpr*>(stmt))
    {
        sb.append_expr(L"BitwiseCastExpr ");
        sb.endline();
    }
    else if (auto breakStmt = dynamic_cast<const BreakStmt*>(stmt))
    {
        sb.append_expr(L"BreakStmt ");
        sb.endline();
    }
    else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
    {
        sb.append_expr(L"CompoundStmt ");
        sb.endline();
    }
    else if (auto callExpr = dynamic_cast<const CallExpr*>(stmt))
    {
        sb.append_expr(L"CallExpr ");
        sb.endline();
    }
    else if (auto caseStmt = dynamic_cast<const CaseStmt*>(stmt))
    {
        sb.append_expr(L"CaseStmt ");
        sb.endline();
    }
    else if (auto methodCall = dynamic_cast<const MethodCallExpr*>(stmt))
    {
        sb.append_expr(L"MethodCallExpr ");
        sb.endline();
    }
    else if (auto constant = dynamic_cast<const ConstantExpr*>(stmt))
    {
        sb.append_expr(L"ConstantExpr ");
        if (auto i = std::get_if<IntValue>(&constant->value))
        {
            if (i->is_signed())
                sb.append(L"IntValue: " + std::to_wstring(i->value<int64_t>().get()));
            else
                sb.append(L"UIntValue: " + std::to_wstring(i->value<uint64_t>().get()));
        }
        else if (auto f = std::get_if<FloatValue>(&constant->value))
        {
            sb.append(L"FloatValue: " + std::to_wstring(f->ieee.value()));
        }
        else
        {
            sb.append(L"UnknownConstant: ");
        }
        sb.endline();
    }
    else if (auto constructExpr = dynamic_cast<const ConstructExpr*>(stmt))
    {
        sb.append_expr(L"ConstructExpr ");
        sb.append_type(constructExpr->type()->name());
        sb.endline();
    }
    else if (auto continueExpr = dynamic_cast<const ContinueStmt*>(stmt))
    {
        sb.append_expr(L"ContinueStmt ");
        sb.endline();
    }
    else if (auto defaultStmt = dynamic_cast<const DefaultStmt*>(stmt))
    {
        sb.append_expr(L"DefaultStmt ");
        sb.endline();
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        sb.append_expr(L"MemberExpr ");
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            sb.append(_as_field->name());
        }
        else if (auto _as_method = dynamic_cast<const MethodDecl*>(field))
        {
            sb.append(_as_method->name());
        }
        else
        {
            sb.append(L"UnknownMember");
        }
        sb.endline();
    }
    else if (auto forStmt = dynamic_cast<const ForStmt*>(stmt))
    {
        sb.append_expr(L"ForStmt ");
        sb.endline();
    }
    else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt))
    {
        sb.append_expr(L"IfStmt ");
        sb.endline();
    }
    else if (auto initList = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append_expr(L"InitListExpr ");
        sb.endline();
    }
    else if (auto implicitCast = dynamic_cast<const ImplicitCastExpr*>(stmt))
    {
        sb.append_expr(L"ImplicitCastExpr ");
        sb.append_type(implicitCast->type()->name());
        sb.endline();
    }
    else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
    {
        sb.append_expr(L"DeclRefExpr ");
        
        if (auto var = dynamic_cast<const VarDecl*>(declRef->decl()))
            sb.append(L"var " + var->name());
        else if (auto func = dynamic_cast<const FunctionDecl*>(declRef->decl()))
            sb.append(L"function " + func->name());
        else if (auto type = dynamic_cast<const TypeDecl*>(declRef->decl()))
            sb.append(L"type " + type->name());
        else
            sb.append(L"UnknownDecl");

        sb.endline();
    }
    else if (auto returnStmt = dynamic_cast<const ReturnStmt*>(stmt))
    {
        sb.append_expr(L"ReturnStmt ");
        sb.endline();
    }
    else if (auto staticCast = dynamic_cast<const StaticCastExpr*>(stmt))
    {
        sb.append_expr(L"StaticCastExpr ");
        sb.append_type(staticCast->type()->name());
        sb.endline();
    }
    else if (auto switchStmt = dynamic_cast<const SwitchStmt*>(stmt))
    {
        sb.append_expr(L"SwitchStmt ");
        sb.endline();
    }
    else if (auto swizzle = dynamic_cast<const SwizzleExpr*>(stmt))
    {
        sb.append_expr(L"SwizzleExpr ");
        for (uint32_t i = 0; i < swizzle->seq().size(); i++)
            sb.append(std::to_wstring(swizzle->seq()[i]));
        sb.endline();
    }
    else if (auto unary = dynamic_cast<const UnaryExpr*>(stmt))
    {
        auto op = unary->op();
        String op_name = L"";
        switch (op)
        {
        case UnaryOp::PLUS:
            op_name = L" + ";
            break;
        case UnaryOp::MINUS:
            op_name = L" - ";
            break;
        case UnaryOp::NOT:
            op_name = L" ! ";
            break;
        case UnaryOp::BIT_NOT:
            op_name = L" ~ ";
            break;
        case UnaryOp::PRE_INC:
            op_name = L" ++ ";
            break;
        case UnaryOp::PRE_DEC:
            op_name = L" -- ";
            break;
        case UnaryOp::POST_INC:
            op_name = L" ++ (postfix) ";
            break;
        case UnaryOp::POST_DEC:
            op_name = L" -- (postfix) ";
            break;
        default:
            assert(false && "Unsupported unary operation");
        }

        sb.append_expr(L"UnaryExpr ");
        sb.append(op_name);
        sb.endline();
    }
    else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
    {
        sb.append_expr(L"DeclStmt ");
        sb.endline();
        
        sb.indent([&](){
            if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
            {
                visit(decl, sb);
            }
            else if (auto decl = dynamic_cast<const FunctionDecl*>(declStmt->decl()))
            {
                visit(decl, sb);
            }
            else
            {
                sb.append_expr(L"UnknownDecl ");
                sb.endline();
            }
        });
    }
    else if (auto whileStmt = dynamic_cast<const WhileStmt*>(stmt))
    {
        sb.append_expr(L"WhileStmt ");
        sb.endline();
    }
    else
    {
        sb.append_expr(L"UnknownExpr ");
        sb.endline();
    }
    
    sb.indent([&](){
        for (auto expr : stmt->children())
        {
            visit(expr, sb);
            sb.endline();
        }
    });
}

void ASTDumper::visit(const skr::CppSL::TypeDecl* typeDecl, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    if (typeDecl->is_builtin())
    {
        sb.append_decl(L"BuiltinType ");
        sb.append_type(typeDecl->name());
        sb.endline();
    }
    else
    {
        sb.append_decl(L"TypeDecl ");
        sb.append_type(typeDecl->name());
        sb.endline();
        
        sb.indent([&](){
            for (auto field : typeDecl->fields())
            {
                visit(field, sb);
            }
            for (auto method : typeDecl->methods())
            {
                visit(method, sb);
            }
        });
    }
}

void ASTDumper::visit(const skr::CppSL::FieldDecl* fieldDecl, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    sb.append_decl(L"FieldDecl ");
    sb.append(fieldDecl->name());
    sb.append_type(L" '" + fieldDecl->type().name() + L"'");
    sb.endline();
}

void ASTDumper::visit(const skr::CppSL::ParamVarDecl* paramDecl, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    sb.append_decl(L"ParamVarDecl ");
    sb.append_type(paramDecl->type().name() + L" ");
    sb.append(paramDecl->name());
    sb.endline();
}

void ASTDumper::visit(const skr::CppSL::FunctionDecl* funcDecl, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    auto as_method = dynamic_cast<const MethodDecl*>(funcDecl);

    sb.append_decl(as_method ? L"MethodDecl " : L"FunctionDecl ");
    sb.append(funcDecl->name());
    sb.endline();
    
    sb.indent([&](){
        for (auto param : funcDecl->parameters())
        {
            visit(param, sb);
        }
        
        if (auto body = funcDecl->body())
        {
            visit(body, sb);
        }
    });
}

void ASTDumper::visit(const skr::CppSL::VarDecl* varDecl, SourceBuilderNew& sb)
{
    using namespace skr::CppSL;
    const bool isGlobalConstant = dynamic_cast<const skr::CppSL::GlobalVarDecl*>(varDecl) != nullptr;
    sb.append_decl(isGlobalConstant ? L"GlobalVarDecl " : L"VarDecl ");
    sb.append(varDecl->name());
    sb.endline();

    if (auto init = varDecl->initializer())
    {
        visit(init, sb);
    }
}

void ASTDumper::Visit(const skr::CppSL::Decl* decl, SourceBuilderNew& sb)
{
    if (auto typeDecl = dynamic_cast<const TypeDecl*>(decl))
    {
        visit(typeDecl, sb);
    }
    else if (auto fieldDecl = dynamic_cast<const FieldDecl*>(decl))
    {
        visit(fieldDecl, sb);
    }
    else if (auto paramDecl = dynamic_cast<const ParamVarDecl*>(decl))
    {
        visit(paramDecl, sb);
    }
    else if (auto funcDecl = dynamic_cast<const FunctionDecl*>(decl))
    {
        visit(funcDecl, sb);
    }
    else if (auto varDecl = dynamic_cast<const VarDecl*>(decl))
    {
        visit(varDecl, sb);
    }
    else
    {
        assert(0 && "Unsupported decl type");
    }
}

void ASTDumper::Visit(const skr::CppSL::Stmt* stmt, SourceBuilderNew& sb)
{
    visit(stmt, sb);
}

String Decl::dump() const
{
    SourceBuilderNew sb;
    ASTDumper().Visit(this, sb);
    return sb.build(SourceBuilderNew::line_builder_tree);
}

String Stmt::dump() const
{
    SourceBuilderNew sb;
    ASTDumper().Visit(this, sb);
    return sb.build(SourceBuilderNew::line_builder_tree);
}

String AST::dump() const
{
    String content = L"";
    for (auto type : types())
    {
        content += type->dump();
    }
    for (auto global : global_vars())
    {
        content += global->dump();
    }
    for (auto func : funcs())
    {
        content += func->dump();
    }
    return content;
}

} // namespace skr::CppSL