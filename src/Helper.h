#pragma once

inline void exit(const llvm::Twine &message) {
  llvm::errs() << message << "\n";
  std::exit(-1);
}
