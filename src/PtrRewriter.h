#pragma once

#include "boost/format.hpp"
#include "clang/AST/Decl.h"
#include "clang/Rewrite/Core/Rewriter.h"

class PtrRewriter {
private:
  clang::Rewriter m_rewriter;

  const std::string shared_ptr = "std::shared_ptr<%1%> ";
  const std::string make_shared = "std::make_shared<%1%>(%2%)";
  const std::string unique_ptr = "std::unique_ptr<%1%> ";
  const std::string make_unique = "std::make_unique<%1%>(%2%)";

  void handleDeferredInit(const clang::VarDecl *initial);

  void rewriteInit(const clang::VarDecl *decl, const clang::Expr *init);

public:
  clang::Rewriter &getRewriter() { return m_rewriter; }

  void initialise(clang::SourceManager &sourceManager, const clang::LangOptions &langOptions);

  void changeFunctionReturn(const clang::FunctionDecl *function);

  void changeDeclaration(const clang::VarDecl *var);

  void changeInit(const clang::VarDecl *var);
};
