#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

class AstHandler {
public:
  AstHandler() = default;

  static void runCallback(clang::ast_matchers::MatchFinder::MatchCallback &callback, llvm::StringRef code);
};
