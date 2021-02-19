#include "Callback.h"
#include "CaptureTracking.h"
#include "clang/AST/Mangle.h"
#include "llvm/IR/ValueSymbolTable.h"

void Callback::run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
  m_ptrRewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

  const auto var = result.Nodes.getNodeAs<clang::VarDecl>("");

  if (!checkVarValid(var)) {
    return;
  }

  if (const auto name = result.SourceManager->getFilename(var->getLocation()); name != m_path) {
    return;
  }

  const auto irValue = getIRValue(var, m_module.get(), result.Context);

  llvm::errs() << var->getSourceRange().printToString(*result.SourceManager) << " Variable " << var->getName();
  if (!isNonEscapingLocalObject(irValue, nullptr)) {
    llvm::errs() << " escapes\n";
    return;
  }
  llvm::errs() << " does not escape\n";

  m_ptrRewriter.rewrite(var);
}

// We want to check its a local pointer that is in the original source, amongst other things
bool checkVarValid(const clang::VarDecl *var) {
  return (var && !var->hasGlobalStorage() && !var->isImplicit() && var->getType()->isPointerType() &&
          !var->getName().empty());
}

std::string getFunctionMangledName(clang::ASTContext *context, const clang::FunctionDecl *namedDecl) {
  const auto mangler = context->createMangleContext(&context->getTargetInfo());

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

const llvm::Value *getIRValue(const clang::VarDecl *var, const llvm::Module *module, clang::ASTContext *context) {
  const auto parentFunctionAST = llvm::dyn_cast<clang::FunctionDecl>(var->getParentFunctionOrMethod());
  const auto parentFunctionIR = module->getFunction(getFunctionMangledName(context, parentFunctionAST));
  const auto valueSymbol = parentFunctionIR->getValueSymbolTable();
  return valueSymbol->lookup(var->getName());
}
