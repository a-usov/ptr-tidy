#include "Callback.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

void CountCallback::run(const MatchFinder::MatchResult &Result) {
  Result.Nodes.getNodeAs<DeclRefExpr>("")->dump();

  if (Result.Nodes.getNodeAs<DeclRefExpr>("")->getDecl()->getType()->isPointerType()) {
    m_count++;
  }
}

unsigned CountCallback::getCount() const { return m_count; }
