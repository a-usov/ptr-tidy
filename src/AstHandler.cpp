#include "AstHandler.h"

using namespace clang::ast_matchers;
using namespace clang::tooling;

void AstHandler::runCallback(MatchFinder::MatchCallback &callback, llvm::StringRef code) {
  MatchFinder Finder;

  Finder.addMatcher(declRefExpr().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), code);
}
