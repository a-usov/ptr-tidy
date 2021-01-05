#include "Callback.h"
#include "CaptureTracking.h"
#include "Helper.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

void CountCallback::run(const MatchFinder::MatchResult &result) {
  m_rewriter.setSourceMgr(*result.SourceManager, result.Context->getLangOpts());

  auto *function = result.Nodes.getNodeAs<FunctionDecl>("");
  if (function && function->getReturnType()->isPointerType()) {
    std::string newType = "std::shared_ptr<";
    newType.append(function->getReturnType()->getPointeeType().getAsString());
    newType.append("> ");
    m_rewriter.ReplaceText(function->getReturnTypeSourceRange(), newType);
    return;
  }

  auto *var = result.Nodes.getNodeAs<VarDecl>("");
  if (!var) {
    return;
  }

  if (var->hasGlobalStorage()) {
    errs() << var->getSourceRange().printToString(*result.SourceManager) << " Not processing global variable "
           << var->getName() << "\n";
    return;
  }

  auto moduleOpt = m_irHandler.getModule();
  if (!moduleOpt) {
    exit("Could not get IR module");
  }
  auto &module = moduleOpt.get();

  Value *value = getLocalValue(var, module, result.Context);

  auto isCaptured = PointerMayBeCaptured(value, true, true);
  errs() << var->getSourceRange().printToString(*result.SourceManager) << " Variable " << var->getName();
  if (isCaptured) {
    errs() << " is captured\n";
  } else {
    errs() << " is not captured\n";
  }

  if (var->getType()->isPointerType()) {
    std::string newType = "std::shared_ptr<" + var->getType()->getPointeeType().getAsString() + ">";
    m_rewriter.ReplaceText(SourceRange(var->getTypeSpecStartLoc(), var->getTypeSpecEndLoc()), newType);

    if (var->hasInit()) {
      auto init = var->getInit();

      auto decl_ref = dyn_cast<DeclRefExpr>(*init->child_begin());
      auto name = decl_ref->getDecl()->getNameAsString();
      auto type = decl_ref->getDecl()->getType().getAsString();

      auto newType = "std::make_shared<" + type + ">(" + name + ")";
      m_rewriter.ReplaceText(init->getSourceRange(), newType);
    }
  }
}
