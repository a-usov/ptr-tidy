#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"

class AstHandler {
public:
  AstHandler() = default;

  static void runCallback(clang::ast_matchers::MatchFinder::MatchCallback &callback,
                          llvm::StringRef code);
};
