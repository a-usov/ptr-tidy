#pragma once

#include "IrHandler.h"
#include "PtrRewriter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class Callback : public clang::ast_matchers::MatchFinder::MatchCallback {
  PtrRewriter m_ptrRewriter;
  const std::unique_ptr<llvm::Module> m_module;
  // fine to store since we know it will life and wont be reallocated - stores file path to source code
  const llvm::StringRef m_path;

public:
  Callback(clang::tooling::ClangTool &tool, std::unique_ptr<llvm::Module> module, const llvm::StringRef path)
      : m_ptrRewriter(tool), m_module(std::move(module)), m_path(path){};

  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

  const clang::Rewriter &getRewriter() { return m_ptrRewriter.getRewriter(); }
};

std::string getFunctionMangledName(clang::ASTContext *context, const clang::FunctionDecl *namedDecl);
const llvm::Value *getIRValue(const clang::VarDecl *var, const llvm::Module *module, clang::ASTContext *context);
bool checkVarValid(const clang::VarDecl *var);
