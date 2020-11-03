#include "Callback.h"

#include "clang/AST/Mangle.h"
#include "CaptureTracking.h"
#include "llvm/IR/ValueSymbolTable.h"

#include "Helper.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

void CountCallback::run(const MatchFinder::MatchResult &result) {
  auto *var = result.Nodes.getNodeAs<VarDecl>("");
  if (!var) {
    exit("Encountered not a VarDecl");
  }

  if (var->hasGlobalStorage()) {
    errs() << var->getSourceRange().printToString(*result.SourceManager)
           << " Not processing global variable " << var->getName() << "\n";
    return;
  }

  auto moduleOpt = m_irHandler.getModule();
  if (!moduleOpt) {
    exit("Could not get IR module");
  }
  auto &module = moduleOpt.get();

  Value *value = getLocalValue(var, module, result.Context);

  auto isCaptured = PointerMayBeCaptured(value, true, true);
  errs() << var->getSourceRange().printToString(*result.SourceManager) << " Variable "
         << var->getName();
  if (isCaptured) {
    errs() << " is captured\n";
  } else {
    errs() << " is not captured\n";
  }
}