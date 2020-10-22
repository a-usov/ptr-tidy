#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

class DumpCallback : public MatchFinder::MatchCallback {
public:
  int m_count = 0;

  void run(const MatchFinder::MatchResult &Result) override {
    llvm::errs() << "---\n";
    Result.Nodes.getNodeAs<DeclRefExpr>("")->dump();

    if (Result.Nodes.getNodeAs<DeclRefExpr>("")
            ->getDecl()
            ->getType()
            ->isPointerType()) {
      m_count++;
    }
  }
};

int main() {
  auto ss = std::ostringstream{};
  std::ifstream file("../ptr.cpp");
  ss << file.rdbuf();
  auto str = ss.str();

  std::cout << str << std::endl;

  DumpCallback Callback;
  MatchFinder Finder;
  Finder.addMatcher(declRefExpr().bind(""), &Callback);
  std::unique_ptr<FrontendActionFactory> Factory(
      newFrontendActionFactory(&Finder));
  clang::tooling::runToolOnCode(Factory->create(), str);
  std::cout << "Number of pointer dereferences is " << Callback.m_count
            << std::endl;
}
