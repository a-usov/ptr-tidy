#include "AstHandler.h"
#include "clang/Tooling/Tooling.h"

using namespace llvm;
using namespace clang::ast_matchers;
using namespace clang::tooling;

void AstHandler::runCallback(MatchFinder::MatchCallback &callback, const StringRef code) {
  MatchFinder Finder;

  Finder.addMatcher(namedDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), code);
}
