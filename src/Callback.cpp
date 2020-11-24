#include "Callback.h"
#include "CaptureTracking.h"
#include "Helper.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

void CountCallback::run(const MatchFinder::MatchResult &result) {
  m_rewriter.setSourceMgr(*result.SourceManager, result.Context->getLangOpts());

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

  if (var->getType()->isPointerType()) {
    std::string newType = "std::shared_ptr<";
    newType.append(var->getType()->getPointeeType().getAsString());
    newType.append("> ");
    m_rewriter.ReplaceText(SourceRange(var->getTypeSpecStartLoc(), var->getTypeSpecEndLoc()),
                           newType);
  }
}