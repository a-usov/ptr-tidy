#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "IrHandler.h"

class CountCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
  IrHandler m_irHandler;

public:
  explicit CountCallback(const llvm::StringRef codePath) : m_irHandler(codePath){};

  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
};
