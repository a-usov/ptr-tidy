#pragma once

#include "IrHandler.h"
#include "PtrRewriter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class CountCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
private:
  IrHandler m_irHandler;
  PtrRewriter m_ptrRewriter;

public:
  explicit CountCallback(const llvm::StringRef codePath) : m_irHandler(codePath){};

  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

  clang::Rewriter &getRewriter() { return m_ptrRewriter.getRewriter(); }
};
