#pragma once

#include "clang/AST/Decl.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Tooling.h"

template <typename Tool> class PtrRewriter {
  Tool &m_tool;
  clang::Rewriter m_rewriter;
  bool m_initialised = false;

  const char *shared_ptr = "std::shared_ptr<{0}> ";
  const char *make_shared = "std::make_shared<{0}>({1})";
  const char *unique_ptr = "std::unique_ptr<{0}> ";
  const char *make_unique = "std::make_unique<{0}>({1})";

  void rewriteFunctionReturn(const clang::VarDecl *var);

  void rewriteDeclaration(const clang::VarDecl *var);

  void rewriteInit(const clang::VarDecl *var);
  void rewriteInit(const clang::VarDecl *var, const clang::Expr *init);
  void rewriteDeferredInit(const clang::VarDecl *var);

  void removeDelete(const clang::VarDecl *var);

public:
  explicit PtrRewriter(Tool &tool) : m_tool(tool) {}

  void initialise(clang::SourceManager &sourceManager, const clang::LangOptions &langOptions);

  void rewrite(const clang::VarDecl *var);

  [[nodiscard]] clang::Rewriter &getRewriter() { return m_rewriter; }
};
