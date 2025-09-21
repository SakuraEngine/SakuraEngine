#include "ASTConsumer.h"
#include "meta.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attrs.inc"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Path.h"
#include <vector>

namespace help {
void str_remove_all(std::string &str, const std::string &remove_str) {
  for (size_t i; (i = str.find(remove_str)) != std::string::npos;) {
    str.replace(i, remove_str.length(), "");
  }
}
void str_join(std::string &a, const std::string &b, const std::string_view sep = ",") {
  if (!a.empty()) {
    a += sep;
  }
  a += b;
}

// 前向声明
std::string filter_cstdint_alias(clang::QualType type, clang::ASTContext *ctx);
std::string filter_using_alias(clang::QualType type, clang::ASTContext *ctx);
std::string resolve_template_arguments(clang::QualType type, clang::ASTContext *ctx);

std::string get_type_name(clang::QualType type, clang::ASTContext *ctx) {
  // 优先查找标记了__final_name__的using别名
  std::string using_alias = filter_using_alias(type, ctx);
  if (!using_alias.empty()) {
    return using_alias;
  }

  // 检查是否有cstdint类型别名，优先使用cstdint别名
  std::string cstdint_alias = filter_cstdint_alias(type, ctx);
  if (!cstdint_alias.empty()) {
    return cstdint_alias;
  }

  // 没有找到标记的using别名或cstdint别名，返回canonical类型名称
  auto canonicalType = type.getCanonicalType();
  auto canonicalName = canonicalType.getAsString(ctx->getLangOpts());
  str_remove_all(canonicalName, "struct ");
  str_remove_all(canonicalName, "class ");
  return canonicalName;
}

std::string filter_cstdint_alias(clang::QualType type, clang::ASTContext *ctx) {
  // 检查类型链，寻找cstdint中的类型别名
  clang::QualType current_type = type;

  // 遍历typedef链
  while (auto typedef_type = current_type->getAs<clang::TypedefType>()) {
    auto typedef_decl = typedef_type->getDecl();
    std::string typedef_name = typedef_decl->getName().str();

    // 检查是否是cstdint中的基本类型别名
    if (typedef_name == "int8_t" || typedef_name == "int16_t" ||
        typedef_name == "int32_t" || typedef_name == "int64_t" ||
        typedef_name == "uint8_t" || typedef_name == "uint16_t" ||
        typedef_name == "uint32_t" || typedef_name == "uint64_t" ||
        typedef_name == "size_t") {

      // 获取完整的限定名称（可能包含std::命名空间）
      std::string qualified_name = typedef_decl->getQualifiedNameAsString();
      return qualified_name;
    }

    // 继续查找底层类型
    current_type = typedef_decl->getUnderlyingType();
  }

  // 没有找到cstdint别名，返回空字符串
  return "";
}

std::string filter_using_alias(clang::QualType type, clang::ASTContext *ctx) {
  // 检查类型链，寻找标记了__final_name__的using/typedef声明
  clang::QualType current_type = type;

  // 遍历typedef链
  while (auto typedef_type = current_type->getAs<clang::TypedefType>()) {
    auto typedef_decl = typedef_type->getDecl();

    // 检查是否有__final_name__标记
    for (auto annotate : typedef_decl->specific_attrs<clang::AnnotateAttr>()) {
      if (annotate->getAnnotation() == "__final_name__") {
        // 找到标记的using/typedef，返回完整限定名称
        return typedef_decl->getQualifiedNameAsString();
      }
    }

    // 继续查找底层类型
    current_type = typedef_decl->getUnderlyingType();
  }

  // 也检查TypeAliasDecl (using声明) - 遍历整个模板特化链条
  current_type = type;
  clang::TypeAliasTemplateDecl* found_decl = nullptr;
  while (auto template_spec_type = current_type->getAs<clang::TemplateSpecializationType>()) {
    // 对于模板特化类型，检查模板声明
    if (auto template_decl = template_spec_type->getTemplateName().getAsTemplateDecl()) {
      if (auto type_alias_template = llvm::dyn_cast<clang::TypeAliasTemplateDecl>(template_decl)) {
        auto type_alias_decl = type_alias_template->getTemplatedDecl();

        // 检查是否有__final_name__标记
        for (auto annotate : type_alias_decl->specific_attrs<clang::AnnotateAttr>()) {
          if (annotate->getAnnotation() == "__final_name__") {
            found_decl = type_alias_template;
          }
        }

        // 继续查找底层类型，遍历using链条
        current_type = type_alias_decl->getUnderlyingType();
      } else {
        // 如果不是TypeAliasTemplateDecl，退出循环
        break;
      }
    } else {
      // 如果无法获取模板声明，退出循环
      break;
    }
  }

  if (found_decl) {
    auto desuger_type = type;
    while (true) {
      auto template_spec_type = desuger_type->getAs<clang::TemplateSpecializationType>();
      auto template_decl = template_spec_type->getTemplateName().getAsTemplateDecl();
      auto type_alias_template = llvm::dyn_cast<clang::TypeAliasTemplateDecl>(template_decl);
      if (type_alias_template == found_decl) {
        break;
      } else {
        desuger_type = desuger_type.getSingleStepDesugaredType(*ctx);
      }
    }
    return resolve_template_arguments(desuger_type, ctx);
  }

  // 没有找到标记的using别名，返回空字符串
  return "";
}

std::string resolve_template_arguments(clang::QualType type, clang::ASTContext *ctx) {
  // 检查是否是模板特化类型
  if (auto template_spec_type = type->getAs<clang::TemplateSpecializationType>()) {
    std::string result;

    // 获取模板名称
    if (auto template_decl = template_spec_type->getTemplateName().getAsTemplateDecl()) {
      result = template_decl->getQualifiedNameAsString();
    } else {
      // 对于其他情况，使用类型的字符串表示来提取模板名称
      std::string full_type = type.getAsString(ctx->getLangOpts());
      size_t angle_pos = full_type.find('<');
      if (angle_pos != std::string::npos) {
        result = full_type.substr(0, angle_pos);
      } else {
        result = full_type;
      }
    }

    // 处理模板参数
    result += "<";
    auto template_args = template_spec_type->template_arguments();
    for (unsigned i = 0; i < template_args.size(); ++i) {
      if (i > 0) {
        result += ", ";
      }

      const auto &arg = template_args[i];
      if (arg.getKind() == clang::TemplateArgument::Type) {
        // 递归处理类型参数，使用完整的get_type_name逻辑
        clang::QualType arg_type = arg.getAsType();
        result += get_type_name(arg_type, ctx);
      } else {
        // 对于非类型参数，使用默认字符串表示
        std::string arg_str;
        llvm::raw_string_ostream arg_stream(arg_str);
        arg.print(ctx->getPrintingPolicy(), arg_stream, true);
        arg_stream.flush();
        result += arg_str;
      }
    }
    result += ">";

    return result;
  }

  // 不是模板类型，返回普通类型名称
  return get_type_name(type, ctx);
}

std::string get_raw_type_name(clang::QualType type, clang::ASTContext *ctx) {
  if (type->isPointerType() || type->isReferenceType())
    type = type->getPointeeType();
  type = type.getUnqualifiedType();
  auto baseName = type.getAsString(ctx->getLangOpts());
  str_remove_all(baseName, "struct ");
  str_remove_all(baseName, "class ");
  return baseName;
}
std::string get_access_string(clang::AccessSpecifier access) {
  switch (access) {
  case clang::AS_public:
    return "public";
  case clang::AS_protected:
    return "protected";
  case clang::AS_private:
    return "private";
  case clang::AS_none:
    return "none";
  }
  return "none";
}
std::string get_comment(clang::Decl *decl, clang::ASTContext *ctx, clang::SourceManager &sm) {
  using namespace clang;
  std::string comment;
  const RawComment *rc = ctx->getRawCommentForDeclNoCache(decl);
  if (rc) {
    SourceRange range = rc->getSourceRange();

    PresumedLoc startPos = sm.getPresumedLoc(range.getBegin());
    PresumedLoc endPos = sm.getPresumedLoc(range.getEnd());

    comment = rc->getBriefText(*ctx);
  }
  return comment;
}
std::string relative_path(const llvm::StringRef &root, const llvm::StringRef &path) {
  if (!path.starts_with(root))
    return {};
  return path.substr(root.size()).str();
}
std::vector<std::string> parse_attr(clang::NamedDecl *decl) {
  std::vector<std::string> attrs;
  for (auto annotate : decl->specific_attrs<clang::AnnotateAttr>()) {
    auto text = annotate->getAnnotation();
    if (text == "__reflect__") {
      continue;
    }
    attrs.push_back(text.str());
  }
  return attrs;
};
bool has_reflect_flag(clang::NamedDecl *decl) {
  for (auto annotate : decl->specific_attrs<clang::AnnotateAttr>()) {
    auto text = annotate->getAnnotation();
    if (text == "__reflect__")
      return true;
  }
  return false;
}
std::string get_decl_file_name(clang::Decl *decl, const clang::PresumedLoc &location) {
  using namespace clang;
  if (location.isValid()) {
    SmallString<2048> AbsolutePath(tooling::getAbsolutePath(location.getFilename()));
    llvm::sys::path::remove_dots(AbsolutePath, true);
    return llvm::sys::path::convert_to_slash(AbsolutePath.str());
  } else {
    return "";
  }
}
} // namespace help

class ParmVisitor : public clang::RecursiveASTVisitor<ParmVisitor> {
public:
  bool VisitParmVarDecl(clang::ParmVarDecl *param_decl) {
    // root decl is a parameter, we visit child depth
    if (param_decl == root_decl) {
      // initialize depth by the current depth
      depth = param_decl->getFunctionScopeDepth() + 1;
      return true;
    }

    // only visit parameters in the same scope
    if (param_decl->getFunctionScopeDepth() != depth)
      return true;

    meta::Field param_data;

    // comment & location
    param_data.comment = help::get_comment(param_decl, consumer->transition_unit_ctx(), consumer->transition_unit_ctx()->getSourceManager());
    param_data.line = consumer->transition_unit_ctx()->getSourceManager().getPresumedLineNumber(param_decl->getLocation());

    // parse parameter data
    param_data.name = param_decl->getNameAsString();
    if (param_data.name.empty()) {
      param_data.name = "unnamed" + std::to_string(param_decl->getFunctionScopeIndex());
      param_data.is_anonymous = true;
    }
    param_data.attrs = help::parse_attr(param_decl);

    // parse array data
    if (param_decl->getType()->isConstantArrayType()) {
      auto ftype = llvm::dyn_cast<clang::ConstantArrayType>(param_decl->getType());
      param_data.array_size = ftype->getSize().getZExtValue();
      param_data.type = help::get_type_name(ftype->getElementType(), consumer->transition_unit_ctx());
      param_data.raw_type = help::get_raw_type_name(ftype->getElementType(), consumer->transition_unit_ctx());
    } else {
      param_data.array_size = 0;
      param_data.type = help::get_type_name(param_decl->getType(), consumer->transition_unit_ctx());
      param_data.raw_type = help::get_raw_type_name(param_decl->getType(), consumer->transition_unit_ctx());
    }

    // recursive handle function pointer
    consumer->_fill_function_pointer(param_decl, param_data);

    // parameter unused data
    param_data.access = help::get_access_string(clang::AS_none);
    param_data.array_size = 0;
    param_data.default_value = "";

    // add parameter
    parameters.emplace_back(std::move(param_data));
    return true;
  }

  // input
  meta::ASTConsumer *consumer; // 用于获取 transition unit 和 递归调用
  clang::Decl *root_decl;      // 根部的 decl, 用于避免重复访问根节点（作为函数参数时）

  std::vector<meta::Field> parameters; // 填充的参数列表
  int depth = 0;
};

namespace meta {
ASTConsumer::ASTConsumer(FileDataMap &datamap, std::string root)
    : _datamap(datamap) {
  _root = llvm::sys::path::convert_to_slash(root);
}

// override
void ASTConsumer::HandleTranslationUnit(ASTContext &ctx) {
  // cache transition unit ctx
  _transition_unit_ctx = &ctx;

  // each translation unit decl
  auto transition_unit_decl = ctx.getTranslationUnitDecl();
  for (auto decl_it = transition_unit_decl->decls_begin(); decl_it != transition_unit_decl->decls_end(); ++decl_it) {
    clang::NamedDecl *child_decl = llvm::dyn_cast<clang::NamedDecl>(*decl_it);
    if (child_decl) {
      switch (child_decl->getKind()) {
      case (clang::Decl::Namespace):
        handle_namespace(child_decl);
        break;
      case (clang::Decl::CXXRecord):
        handle_record(child_decl);
        break;
      case (clang::Decl::Function):
        handle_function(child_decl);
        break;
      case (clang::Decl::Enum):
        handle_enum(child_decl);
        break;
      case (clang::Decl::ClassTemplate):
        handle_template(child_decl);
        break;
      default:
        break;
      }
    }
  }
}

// root level parse functions
void ASTConsumer::handle_namespace(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return;

  // each child decl
  clang::DeclContext *decl_ctx = decl->castToDeclContext(decl);
  for (auto decl_it = decl_ctx->decls_begin(); decl_it != decl_ctx->decls_end(); ++decl_it) {
    clang::NamedDecl *child_decl = llvm::dyn_cast<clang::NamedDecl>(*decl_it);
    if (child_decl) {
      switch (child_decl->getKind()) {
      case (clang::Decl::Namespace):
        handle_namespace(child_decl);
        break;
      case (clang::Decl::CXXRecord):
        handle_record(child_decl);
        break;
      case (clang::Decl::Function):
        handle_function(child_decl);
        break;
      case (clang::Decl::Enum):
        handle_enum(child_decl);
        break;
      case (clang::Decl::ClassTemplate):
        handle_template(child_decl);
        break;
      default:
        break;
      }
    }
  }
}
void ASTConsumer::handle_record(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return;

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return;
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return;
  }

  // filter reflect flag
  if (!_filter_reflect_flag(decl)) {
    return;
  }

  // get file data base
  auto &db = _get_file_db(rel_file_name);

  // filter record
  clang::CXXRecordDecl *record_decl = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
  if (!record_decl) {
    return;
  }

  // filter forward declaration
  if (record_decl->isThisDeclarationADefinition() == clang::VarDecl::DeclarationOnly) {
    // ignore forward declaration attributes
    // LOG("attribute on forward declaration is ignored.");
    return;
  }

  // filter nested record
  {
    clang::DeclContext *parent = record_decl->getDeclContext();
    while (parent) {
      if (auto parentRecord = llvm::dyn_cast<clang::CXXRecordDecl>(parent)) {
        return;
      }
      parent = parent->getParent();
    }
  }

  // filter anonymous record
  if (record_decl->isAnonymousStructOrUnion()) {
    // LOG("attribute on anonymous record is ignored.");
    return;
  }

  // filter union
  if (record_decl->isUnion()) {
    // LOG("union is not fully supported, use at your own risk.");
    return;
  }

  Record record_data = {};

  // parse comment & location
  record_data.comment = help::get_comment(record_decl, _transition_unit_ctx, source_manager);
  record_data.file_name = abs_file_name;
  record_data.line = line;

  // parse record data
  record_data.name = record_decl->getQualifiedNameAsString();
  record_data.attrs = help::parse_attr(record_decl);
  for (auto base : record_decl->bases()) {
    record_data.bases.push_back(help::get_type_name(base.getType(), _transition_unit_ctx));
    // TODO. base info
    base.isVirtual();
    base.getAccessSpecifier();
  }

  // dispatch child decl
  for (auto child_decl : decl->castToDeclContext(decl)->decls()) {
    auto named_child_decl = llvm::dyn_cast<clang::NamedDecl>(child_decl);
    if (named_child_decl) {
      switch (named_child_decl->getKind()) {
      case (clang::Decl::Field): {
        auto result = handle_field(named_child_decl);
        if (result) {
          record_data.fields.push_back(std::move(result.value()));
        }
        break;
      }
      case (clang::Decl::Var): {
        auto result = handle_static_field(named_child_decl);
        if (result) {
          record_data.fields.push_back(std::move(result.value()));
        }
        break;
      }
      case (clang::Decl::CXXMethod): {
        auto result = handle_method(named_child_decl);
        if (result) {
          record_data.methods.emplace_back(std::move(result.value()));
        }
        break;
      }
      case (clang::Decl::Function): {
        auto result = handle_static_method(named_child_decl);
        if (result) {
          record_data.methods.emplace_back(std::move(result.value()));
        }
        break;
      }
      case (clang::Decl::CXXConstructor): {
        auto result = handle_constructor(named_child_decl);
        if (result) {
          record_data.ctors.emplace_back(std::move(result.value()));
        }
        break;
      }
      case (clang::Decl::Record):
        // nested record is not supported now
        break;
      default:
        break;
      }
    }
  }

  // push record
  _get_file_db(rel_file_name).records.emplace_back(std::move(record_data));
}
void ASTConsumer::handle_enum(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return;

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return;
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return;
  }

  // filter reflect flag
  if (!_filter_reflect_flag(decl)) {
    return;
  }

  // filter enum
  clang::EnumDecl *enum_decl = llvm::dyn_cast<clang::EnumDecl>(decl);
  if (!enum_decl) {
    return;
  }

  Enum enum_data;

  // parse comment & location
  enum_data.comment = help::get_comment(enum_decl, _transition_unit_ctx, source_manager);
  enum_data.file_name = abs_file_name;
  enum_data.line = line;

  // parse enum data
  enum_data.name = enum_decl->getQualifiedNameAsString();
  enum_data.is_scoped = enum_decl->isScoped();
  enum_data.underlying_type = enum_decl->isFixed()
                                  ? enum_decl->getIntegerType().getAsString(_transition_unit_ctx->getLangOpts())
                                  : "unfixed";
  enum_data.attrs = help::parse_attr(enum_decl);

  // parse enum item
  for (auto enumerator : enum_decl->enumerators()) {
    EnumValue enumerator_data;
    // parse comment & location
    enumerator_data.comment = help::get_comment(enumerator, _transition_unit_ctx, source_manager);
    enumerator_data.line = source_manager.getPresumedLineNumber(enumerator->getLocation());

    // parse enum item data
    enumerator_data.name = enumerator->getQualifiedNameAsString();
    enumerator_data.value = enumerator->getInitVal().getRawData()[0];
    enumerator_data.attrs = help::parse_attr(enumerator);

    // push enum item
    enum_data.values.push_back(std::move(enumerator_data));
  }

  // push enum
  _get_file_db(rel_file_name).enums.push_back(std::move(enum_data));
}
void ASTConsumer::handle_function(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return;

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return;
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return;
  }

  // filter reflect flag
  if (!_filter_reflect_flag(decl)) {
    return;
  }

  // filter function
  clang::FunctionDecl *func_decl = llvm::dyn_cast<clang::FunctionDecl>(decl);
  if (!func_decl) {
    return;
  }

  Function func_data;

  // comment & location
  func_data.comment = help::get_comment(func_decl, _transition_unit_ctx, source_manager);
  func_data.file_name = abs_file_name;
  func_data.line = line;

  // parse function data
  _fill_function_data(func_decl, func_data);

  // unused function data
  func_data.access = help::get_access_string(func_decl->getAccess());
  func_data.is_const = false;

  // push function
  _get_file_db(rel_file_name).functions.push_back(std::move(func_data));
}
void ASTConsumer::handle_template(clang::NamedDecl *decl) {
  // unsupported now
  return;

  // get template decl
  CXXRecordDecl *template_decl = nullptr;
  if (auto templateDecl = llvm::dyn_cast<clang::ClassTemplateDecl>(decl)) {
    if (auto inner = templateDecl->getTemplatedDecl())
      template_decl = inner;
  }
}

// leaf level parse functions
std::optional<Function> ASTConsumer::handle_method(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return {};

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return {};
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return {};
  }

  // filter method
  clang::CXXMethodDecl *method_decl = llvm::dyn_cast<clang::CXXMethodDecl>(decl);
  if (!method_decl) {
    return {};
  }

  Function out_method = {};

  // comment & location
  out_method.comment = help::get_comment(method_decl, _transition_unit_ctx, source_manager);
  out_method.file_name = abs_file_name;
  out_method.line = line;

  // parse method data
  _fill_function_data(method_decl, out_method);

  // access & const
  out_method.access = help::get_access_string(method_decl->getAccess());
  out_method.is_const = method_decl->isConst();

  return std::move(out_method);
}
std::optional<Function> ASTConsumer::handle_static_method(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return {};

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return {};
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return {};
  }

  // filter static method
  clang::FunctionDecl *func_decl = llvm::dyn_cast<clang::FunctionDecl>(decl);
  if (!func_decl) {
    return {};
  }

  Function out_method = {};

  // comment & location
  out_method.comment = help::get_comment(func_decl, _transition_unit_ctx, source_manager);
  out_method.file_name = abs_file_name;
  out_method.line = line;

  // parse function data
  _fill_function_data(func_decl, out_method);

  // access & const
  out_method.access = help::get_access_string(func_decl->getAccess());
  out_method.is_const = false;

  return std::move(out_method);
}
std::optional<Field> ASTConsumer::handle_field(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return {};

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return {};
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return {};
  }

  // filter field
  clang::FieldDecl *field_decl = llvm::dyn_cast<clang::FieldDecl>(decl);
  if (!field_decl) {
    return {};
  }

  Field out_field = {};

  // comment & location
  out_field.comment = help::get_comment(field_decl, _transition_unit_ctx, source_manager);
  out_field.line = line;

  // parse field data
  out_field.name = field_decl->getNameAsString();
  out_field.attrs = help::parse_attr(field_decl);
  out_field.access = help::get_access_string(field_decl->getAccess());
  out_field.is_static = false;

  // parse array data
  if (field_decl->getType()->isConstantArrayType()) {
    auto ftype =
        llvm::dyn_cast<clang::ConstantArrayType>(field_decl->getType());
    out_field.array_size = ftype->getSize().getZExtValue();
    out_field.type = help::get_type_name(ftype->getElementType(), _transition_unit_ctx);
    out_field.raw_type = help::get_raw_type_name(ftype->getElementType(), _transition_unit_ctx);
  } else {
    out_field.array_size = 0;
    out_field.type = help::get_type_name(field_decl->getType(), _transition_unit_ctx);
    out_field.raw_type = help::get_raw_type_name(field_decl->getType(), _transition_unit_ctx);
  }

  // default value
  if (field_decl->hasInClassInitializer()) {
    llvm::raw_string_ostream s(out_field.default_value);
    auto defArg = field_decl->getInClassInitializer();
    defArg->printPretty(s, nullptr, _transition_unit_ctx->getPrintingPolicy());
  }

  // handle if field is function pointer
  _fill_function_pointer(field_decl, out_field);

  return out_field;
}
std::optional<Field> ASTConsumer::handle_static_field(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return {};

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return {};
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return {};
  }

  // filter static field
  clang::VarDecl *var_decl = llvm::dyn_cast<clang::VarDecl>(decl);
  if (!var_decl || !var_decl->isStaticDataMember()) {
    return {};
  }

  Field out_field = {};

  // comment & location
  out_field.comment = help::get_comment(var_decl, _transition_unit_ctx, source_manager);
  out_field.line = line;

  // field data
  out_field.name = var_decl->getNameAsString();
  out_field.attrs = help::parse_attr(var_decl);
  out_field.access = help::get_access_string(var_decl->getAccess());
  out_field.is_static = true;

  // parse array data
  if (var_decl->getType()->isConstantArrayType()) {
    auto ftype =
        llvm::dyn_cast<clang::ConstantArrayType>(var_decl->getType());
    out_field.array_size = ftype->getSize().getZExtValue();
    out_field.type = help::get_type_name(ftype->getElementType(), _transition_unit_ctx);
    out_field.raw_type = help::get_raw_type_name(ftype->getElementType(), _transition_unit_ctx);
  } else {
    out_field.array_size = 0;
    out_field.type = help::get_type_name(var_decl->getType(), _transition_unit_ctx);
    out_field.raw_type = help::get_raw_type_name(var_decl->getType(), _transition_unit_ctx);
  }

  // handle if field is function pointer
  _fill_function_pointer(var_decl, out_field);

  return std::move(out_field);
}
std::optional<Constructor> ASTConsumer::handle_constructor(clang::NamedDecl *decl) {
  // filter invalid decl
  if (decl->isInvalidDecl())
    return {};

  // filter location
  clang::SourceManager &source_manager = _transition_unit_ctx->getSourceManager();
  clang::PresumedLoc location = source_manager.getPresumedLoc(decl->getLocation());
  std::string abs_file_name;
  std::string rel_file_name;
  unsigned line;
  if (!_filter_decl_location(
          decl,
          location,
          abs_file_name,
          rel_file_name,
          line)) {
    return {};
  }

  // filter parsed identity
  if (!_filter_parsed_identity(decl, abs_file_name, line)) {
    return {};
  }

  // filter
  clang::CXXConstructorDecl *ctor_decl = llvm::dyn_cast<clang::CXXConstructorDecl>(decl);
  if (!ctor_decl) {
    return {};
  }

  Constructor out_ctor = {};

  // comment & location
  out_ctor.comment = help::get_comment(ctor_decl, _transition_unit_ctx, source_manager);
  out_ctor.file_name = abs_file_name;
  out_ctor.line = line;

  // llvm::outs() << "ctor: " << ctor_decl->getQualifiedNameAsString() << "\n";
  // parse method data
  _fill_ctor_data(ctor_decl, out_ctor);
  // llvm::outs() << "end ctor: " << ctor_decl->getQualifiedNameAsString() << "\n";

  // access & const
  out_ctor.access = help::get_access_string(ctor_decl->getAccess());

  return std::move(out_ctor);
}

// helper functions
bool ASTConsumer::_filter_decl_location(clang::NamedDecl *decl,
                                        const clang::PresumedLoc &location,
                                        std::string &out_abs_file_name,
                                        std::string &out_rel_file_name,
                                        unsigned &out_line) {
  // filter location
  if (location.isInvalid()) {
    // llvm::outs() << "[No Location]";
    // decl->printName(llvm::outs());
    // llvm::outs() << "\n";
    return false;
  }

  // location info
  out_abs_file_name = help::get_decl_file_name(decl, location);
  out_rel_file_name = help::relative_path(_root, out_abs_file_name);
  out_line = location.getLine();

  // filter files not under root
  if (out_rel_file_name.empty()) {
    // llvm::outs() << "[No Filename]";
    // decl->printName(llvm::outs());
    // llvm::outs() << "\n";
    return false;
  }
  return true;
}
bool ASTConsumer::_filter_parsed_identity(clang::NamedDecl *decl, const std::string &file_name, unsigned line) {
  Identity ident;
  ident.fileName = file_name;
  ident.line = line;
  bool is_parsed = _parsed.find(ident) != _parsed.end();
  if (is_parsed && decl->getKind() != clang::Decl::Namespace) {
    return false;
  }
  _parsed.insert(ident);
  return true;
}
bool ASTConsumer::_filter_reflect_flag(clang::NamedDecl *decl) {
  bool has_reflect_entry = help::has_reflect_flag(decl);
  switch (decl->getKind()) {
  case (clang::Decl::CXXRecord):
  case (clang::Decl::Enum):
  case (clang::Decl::Function):
    if (!has_reflect_entry) {
      return false;
    }
    break;
  case (clang::Decl::ClassTemplate):
  case (clang::Decl::ClassTemplateSpecialization):
  case (clang::Decl::FunctionTemplate): // current unsupported
    return false;
  default: // 其它情况不影响性能，可以不做过滤
    break;
  }
  return true;
}
Database &ASTConsumer::_get_file_db(const std::string &rel_file_name) {
  return _datamap[rel_file_name];
}
void ASTConsumer::_fill_function_data(clang::FunctionDecl *func_decl, Function &out_func_data) {
  // parse function data
  out_func_data.name = func_decl->getQualifiedNameAsString();
  out_func_data.is_static = func_decl->isStatic();
  auto func_proto_type = func_decl->getType()->getAs<clang::FunctionProtoType>();
  out_func_data.is_nothrow = func_proto_type ? func_proto_type->isNothrow() : false;
  out_func_data.attrs = help::parse_attr(func_decl);
  if (!func_decl->isNoReturn()) {
    out_func_data.ret_type = help::get_type_name(func_decl->getReturnType(), _transition_unit_ctx);
    out_func_data.raw_ret_type = help::get_raw_type_name(func_decl->getReturnType(), _transition_unit_ctx);
  }

  // parse parameters
  for (auto param : func_decl->parameters()) {
    Field param_data;

    // comment & location
    param_data.comment = help::get_comment(param, _transition_unit_ctx, _transition_unit_ctx->getSourceManager());
    param_data.line = _transition_unit_ctx->getSourceManager().getPresumedLineNumber(param->getLocation());

    // parse parameter data
    param_data.name = param->getNameAsString();
    if (param_data.name.empty()) {
      param_data.name = "unnamed" + std::to_string(out_func_data.parameters.size());
      param_data.is_anonymous = true;
    }
    param_data.attrs = help::parse_attr(param);

    // parse array data
    if (param->getType()->isConstantArrayType()) {
      auto ftype = llvm::dyn_cast<clang::ConstantArrayType>(param->getType());
      param_data.array_size = ftype->getSize().getZExtValue();
      param_data.type = help::get_type_name(ftype->getElementType(), _transition_unit_ctx);
      param_data.raw_type = help::get_raw_type_name(ftype->getElementType(), _transition_unit_ctx);
    } else {
      param_data.array_size = 0;
      param_data.type = help::get_type_name(param->getType(), _transition_unit_ctx);
      param_data.raw_type = help::get_raw_type_name(param->getType(), _transition_unit_ctx);
    }

    // parse default value
    if (param->hasDefaultArg()) {
      llvm::raw_string_ostream s(param_data.default_value);
      if (param->hasUninstantiatedDefaultArg()) {
        auto defArg = param->getUninstantiatedDefaultArg();
        defArg->printPretty(s, nullptr, _transition_unit_ctx->getPrintingPolicy());
      } else {
        auto defArg = param->getDefaultArg();
        defArg->printPretty(s, nullptr, _transition_unit_ctx->getPrintingPolicy());
      }
    }

    // parameter unused data
    param_data.access = help::get_access_string(clang::AS_none);

    // handle if function pointer
    _fill_function_pointer(param, param_data);

    // push parameter
    out_func_data.parameters.push_back(std::move(param_data));
  }
}
void ASTConsumer::_fill_function_pointer(clang::DeclaratorDecl *decl, Field &out_field) {
  // init
  clang::Decl *signature_decl = decl;
  clang::QualType signature_type = decl->getType();

  // decode typedef
  {
    auto typedef_type = signature_type->getAs<clang::TypedefType>();
    if (typedef_type) {
      signature_decl = typedef_type->getDecl();
      signature_type = typedef_type->getDecl()->getUnderlyingType();
    }
  }

  // decode std::function
  // TODO. remove it
  bool is_functor = false;
  {
    auto template_specialization_type = signature_type->getAs<clang::TemplateSpecializationType>();
    if (template_specialization_type) {
      auto arguments = template_specialization_type->template_arguments();
      if (arguments.size() == 0) {
        return;
      }
      if (arguments[0].getKind() != clang::TemplateArgument::Type) {
        return;
      }

      // update signature
      signature_type = arguments[0].getAsType();
      is_functor = true;
    }
  }

  // decode typedef again for functor
  // TODO. remove it
  {
    auto typedef_type = signature_type->getAs<clang::TypedefType>();
    if (typedef_type) {
      signature_decl = typedef_type->getDecl();
      signature_type = typedef_type->getDecl()->getUnderlyingType();
    }
  }

  // decay signature
  if (signature_type->isFunctionPointerType()) {
    signature_type = signature_type->castAs<clang::PointerType>()->getPointeeType();
  } else if (signature_type->isFunctionReferenceType()) {
    signature_type = signature_type->castAs<clang::ReferenceType>()->getPointeeType();
  } else if (signature_type->isConstantArrayType()) {
    signature_type = signature_type->getAsArrayTypeUnsafe()->getElementType();
  }

  // final type
  auto final_func_type = signature_type->getAs<clang::FunctionType>();
  if (!final_func_type) {
    return;
  }

  // visit parameters
  ParmVisitor param_visitor;
  param_visitor.consumer = this;
  param_visitor.root_decl = signature_decl;
  param_visitor.TraverseDecl(signature_decl);

  // fill field function info
  out_field.is_functor = is_functor;
  out_field.is_callback = true;

  // signature comment & location
  out_field.signature.comment = help::get_comment(signature_decl, transition_unit_ctx(), transition_unit_ctx()->getSourceManager());
  out_field.signature.file_name = help::get_decl_file_name(signature_decl, transition_unit_ctx()->getSourceManager().getPresumedLoc(signature_decl->getLocation()));
  out_field.signature.line = transition_unit_ctx()->getSourceManager().getPresumedLineNumber(signature_decl->getLocation());

  // fill signature data
  out_field.signature.name = out_field.name;
  out_field.signature.attrs = out_field.attrs;
  auto func_proto_type = final_func_type->getAs<clang::FunctionProtoType>();
  out_field.signature.is_nothrow = func_proto_type ? func_proto_type->isNothrow() : false;
  out_field.signature.ret_type = help::get_type_name(final_func_type->getReturnType(), transition_unit_ctx());
  out_field.signature.raw_ret_type = help::get_raw_type_name(final_func_type->getReturnType(), transition_unit_ctx());

  // fill parameters
  out_field.signature.parameters = std::move(param_visitor.parameters);

  // signature unused data
  out_field.signature.access = help::get_access_string(clang::AS_none);
  out_field.signature.is_static = true;
  out_field.signature.is_const = false;
}
void ASTConsumer::_fill_ctor_data(clang::CXXConstructorDecl *ctor_decl, Constructor &out_ctor_data) {
  // parse function data
  out_ctor_data.name = ctor_decl->getQualifiedNameAsString();
  auto func_proto_type = ctor_decl->getType()->getAs<clang::FunctionProtoType>();
  out_ctor_data.attrs = help::parse_attr(ctor_decl);

  // parse parameters
  for (auto param : ctor_decl->parameters()) {
    Field param_data;

    // comment & location
    param_data.comment = help::get_comment(param, _transition_unit_ctx, _transition_unit_ctx->getSourceManager());
    param_data.line = _transition_unit_ctx->getSourceManager().getPresumedLineNumber(param->getLocation());

    // parse parameter data
    param_data.name = param->getNameAsString();
    if (param_data.name.empty()) {
      param_data.name = "unnamed" + std::to_string(out_ctor_data.parameters.size());
      param_data.is_anonymous = true;
    }
    param_data.attrs = help::parse_attr(param);

    // parse array data
    if (param->getType()->isConstantArrayType()) {
      auto ftype = llvm::dyn_cast<clang::ConstantArrayType>(param->getType());
      param_data.array_size = ftype->getSize().getZExtValue();
      param_data.type = help::get_type_name(ftype->getElementType(), _transition_unit_ctx);
      param_data.raw_type = help::get_raw_type_name(ftype->getElementType(), _transition_unit_ctx);
    } else {
      param_data.array_size = 0;
      param_data.type = help::get_type_name(param->getType(), _transition_unit_ctx);
      param_data.raw_type = help::get_raw_type_name(param->getType(), _transition_unit_ctx);
    }

    // parse default value
    if (param->hasDefaultArg()) {
      llvm::raw_string_ostream s(param_data.default_value);
      if (param->hasUninstantiatedDefaultArg()) {
        auto defArg = param->getUninstantiatedDefaultArg();
        defArg->printPretty(s, nullptr, _transition_unit_ctx->getPrintingPolicy());
      } else {
        auto defArg = param->getDefaultArg();
        defArg->printPretty(s, nullptr, _transition_unit_ctx->getPrintingPolicy());
      }
    }

    // parameter unused data
    param_data.access = help::get_access_string(clang::AS_none);

    // handle if function pointer
    _fill_function_pointer(param, param_data);

    // push parameter
    out_ctor_data.parameters.push_back(std::move(param_data));
  }
}

} // namespace meta