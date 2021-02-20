#pragma once

#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/Twine.h"

inline void exit(const llvm::Twine &message) {
  llvm::errs() << message << "\n";
  std::exit(-1);
}

class TestTool {
public:
  const char *code;

  void run(clang::tooling::FrontendActionFactory *factory) const {
    clang::tooling::runToolOnCode(factory->create(), code);
  }
};
