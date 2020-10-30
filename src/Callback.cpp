#include "Callback.h"

#include "clang/AST/Mangle.h"
#include "clang/Basic/TargetCXXABI.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/IR/ValueSymbolTable.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

void CountCallback::run(const MatchFinder::MatchResult &result) {
  if (auto *var = result.Nodes.getNodeAs<VarDecl>(""); var) {
    auto *parentFunction = dyn_cast<FunctionDecl>(var->getParentFunctionOrMethod());

    if (auto moduleOpt = m_irHandler.getModule(); moduleOpt) {
      auto &module = moduleOpt.get();
      auto *mangler = result.Context->createMangleContext(&result.Context->getTargetInfo());

      Function *func;

      if (mangler->shouldMangleDeclName(parentFunction)) {
        std::string parentFunctionMangled;
        {
          raw_string_ostream mangledNameStream(parentFunctionMangled);
          mangler->mangleName(parentFunction, mangledNameStream);
          mangledNameStream.flush();
        }
        func = module.getFunction(parentFunctionMangled);
      } else {
        func = module.getFunction(parentFunction->getName());
      }

      auto *valueSymbol = func->getValueSymbolTable();
      auto *value = valueSymbol->lookup(var->getName());

      auto isCaptured = PointerMayBeCaptured(value, true, true);
      errs() << "Variable " << var->getName() << " in function " << parentFunction->getName();
      if (isCaptured) {
        errs() << " is captured\n";
      } else {
        errs() << " is not captured\n";
      }
    }
  }
}