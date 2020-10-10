#include <iostream>
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

int main() {
	std::cout << "Hello world" << std::endl;
}

class DumpCallback : public clang::ast_matchers::MatchFinder::MatchCallback {};
