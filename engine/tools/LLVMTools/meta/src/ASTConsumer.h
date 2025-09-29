#pragma once

#include "meta.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecordLayout.h"
#include <unordered_set>

class ParmVisitor;

namespace meta {
using namespace clang;

class ASTConsumer : public clang::ASTConsumer {
public:
  ASTConsumer(FileDataMap &datamap, std::string root);

  // getter
  ASTContext *transition_unit_ctx() { return _transition_unit_ctx; }

  // override
  void HandleTranslationUnit(ASTContext &ctx) override;

  // root level parse functions
  void handle_namespace(clang::NamedDecl *decl);
  void handle_record(clang::NamedDecl *decl);
  void handle_enum(clang::NamedDecl *decl);
  void handle_function(clang::NamedDecl *decl);
  void handle_template(clang::NamedDecl *decl);

  // leaf level parse functions
  std::optional<Function> handle_method(clang::NamedDecl *decl);
  std::optional<Function> handle_static_method(clang::NamedDecl *decl);
  std::optional<Field> handle_field(clang::NamedDecl *decl);
  std::optional<Field> handle_static_field(clang::NamedDecl *decl);
  std::optional<Constructor> handle_constructor(clang::NamedDecl *decl);

protected:
  friend class ::ParmVisitor;

  // helper functions
  bool _filter_decl_location(clang::NamedDecl *decl,
                             const clang::PresumedLoc &location,
                             std::string &out_abs_file_name,
                             std::string &out_rel_file_name,
                             unsigned &out_line);
  bool _filter_parsed_identity(clang::NamedDecl *decl, const std::string &file_name, unsigned line);
  bool _filter_reflect_flag(clang::NamedDecl *decl);
  Database &_get_file_db(const std::string &rel_file_name);
  void _fill_function_data(clang::FunctionDecl *func_decl, Function &out_func_data);
  void _fill_function_pointer(clang::DeclaratorDecl *decl, Field &out_field);
  void _fill_ctor_data(clang::CXXConstructorDecl *ctor_decl, Constructor &out_ctor_data);

protected:
  // config
  FileDataMap &_datamap;
  std::string _root = {};

  // 跳过前置声明的重复解析
  std::unordered_set<meta::Identity, meta::IdentityHash> _parsed = {};

  // 编译单元的节点
  ASTContext *_transition_unit_ctx = nullptr;
};
} // namespace meta