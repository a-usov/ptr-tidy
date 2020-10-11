#include <iostream>
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

class DumpCallback : public MatchFinder::MatchCallback {
	virtual void run(const MatchFinder::MatchResult &Result) {
		llvm::errs() << "---\n";
		Result.Nodes.getNodeAs<CXXRecordDecl>("x")->dump();
	}
};

int main() {
	std::cout << "Hello world" << std::endl;

	DumpCallback Callback;
	MatchFinder Finder;
	Finder.addMatcher(recordDecl().bind("x"), &Callback);
	std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
	clang::tooling::runToolOnCode(Factory->create(), "class X {};");
}
