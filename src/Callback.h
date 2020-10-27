#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

class CountCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
  unsigned m_count = 0;

public:
  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

  [[nodiscard]] unsigned getCount() const;
};
