#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/Twine.h"

inline void exit(const llvm::Twine &message) {
  llvm::errs() << message << "\n";
  std::exit(-1);
}

class LambdaCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
  std::function<void(const clang::ast_matchers::MatchFinder::MatchResult &result)> m_lambda;

  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override { m_lambda(result); }

public:
  explicit LambdaCallback(auto lambda) : m_lambda(std::move(lambda)){};
};

class TestTool {
public:
  const char *code;

  void run(clang::tooling::FrontendActionFactory *factory) const {
    clang::tooling::runToolOnCode(factory->create(), code);
  }
};
