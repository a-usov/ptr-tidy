#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/Mangle.h"
#include "clang/Lex/Lexer.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Support/MemoryBuffer.h"

inline void exit(const llvm::StringRef &message) {
  llvm::errs() << message << "\n";
  std::exit(-1);
}

inline std::string getMangledName(clang::ASTContext *context, const clang::NamedDecl *namedDecl) {
  auto mangler = context->createMangleContext(&context->getTargetInfo());

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

inline llvm::Value *getLocalValue(const clang::VarDecl *var, const llvm::Module &module, clang::ASTContext *context) {
  auto parentFunctionAST = llvm::dyn_cast<clang::FunctionDecl>(var->getParentFunctionOrMethod());
  auto parentFunctionIR = module.getFunction(getMangledName(context, parentFunctionAST));
  auto valueSymbol = parentFunctionIR->getValueSymbolTable();
  return valueSymbol->lookup(var->getName());
}

inline std::optional<std::unique_ptr<llvm::MemoryBuffer>> getFile(const llvm::StringRef name) {
  auto sourceFileOrError = llvm::MemoryBuffer::getFile(name);
  return sourceFileOrError ? std::make_optional(std::move(sourceFileOrError.get())) : std::nullopt;
}

inline llvm::StringRef strFromSourceRange(clang::SourceRange sourceRange, const clang::SourceManager &sm,
                                          const clang::LangOptions &lo) {
  return clang::Lexer::getSourceText(clang::Lexer::getAsCharRange(sourceRange, sm, lo), sm, lo);
}
