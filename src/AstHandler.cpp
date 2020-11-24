#include "AstHandler.h"
#include "clang/Tooling/Tooling.h"

using namespace clang::ast_matchers;
using namespace clang::tooling;

void AstHandler::runCallback(MatchFinder::MatchCallback &callback, const llvm::StringRef code) {
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), code);
}