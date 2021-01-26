#include "Callback.h"
#include "CaptureTracking.h"
#include "Helper.h"

using namespace llvm;
using namespace clang;

void CountCallback::run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
  m_ptrRewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

  auto var = result.Nodes.getNodeAs<VarDecl>("");
  if (!var || var->hasGlobalStorage() || var->isImplicit() || !var->getType()->isPointerType()) {
    return;
  }

  auto moduleOpt = m_irHandler.getModule();
  if (!moduleOpt) {
    exit("Could not get IR module");
  }
  auto &module = moduleOpt.get();
  auto value = getLocalValue(var, module, result.Context);

  auto isNonEscape = isNonEscapingLocalObject(value, nullptr);

  errs() << var->getSourceRange().printToString(*result.SourceManager) << " Variable " << var->getName();
  if (!isNonEscape) {
    errs() << " escapes\n";
    return;
  }
  errs() << " does not escape\n";

  // TODO - we dont always change return type
  auto function = dyn_cast<FunctionDecl>(var->getParentFunctionOrMethod());
  m_ptrRewriter.changeFunctionReturn(function);

  m_ptrRewriter.changeDeclaration(var);
  m_ptrRewriter.changeInit(var);
}
