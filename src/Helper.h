#pragma once

#include "clang/AST/ASTContext.h"
#include "llvm/Support/raw_os_ostream.h"

inline void exit(const llvm::StringRef &message) {
  llvm::errs() << message << "\n";
  std::exit(-1);
}

inline std::string getMangledName(clang::ASTContext *context, const clang::NamedDecl *namedDecl) {
  auto *mangler = context->createMangleContext(&context->getTargetInfo());

  if (mangler->shouldMangleDeclName(namedDecl)) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    mangler->mangleName(namedDecl, mangledNameStream);
    mangledNameStream.flush();

    return mangledName;
  } else {
    return namedDecl->getNameAsString();
  }
}

inline llvm::Value *getLocalValue(const clang::VarDecl *var, const llvm::Module &module,
                                     clang::ASTContext *context) {
  auto *parentFunctionAST = dyn_cast<clang::FunctionDecl>(var->getParentFunctionOrMethod());
  auto *parentFunctionIR = module.getFunction(getMangledName(context, parentFunctionAST));
  auto *valueSymbol = parentFunctionIR->getValueSymbolTable();
  return valueSymbol->lookup(var->getName());
}
