#pragma once

#include "IrHandler.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"

class CountCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
private:
  IrHandler m_irHandler;
  clang::Rewriter m_rewriter;

public:
  explicit CountCallback(const llvm::StringRef codePath) : m_irHandler(codePath){};

  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

  clang::Rewriter &getRewriter() { return m_rewriter; }
};
