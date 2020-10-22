#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include <llvm/IR/InstVisitor.h>

using namespace llvm;

int main() {
  LLVMContext context;
  SMDiagnostic error;
  std::unique_ptr<Module> m = parseIRFile("../ptr.ll", error, context);

  if (m) {
    Function *F = m->getFunction("_Z3asdv");
    auto *eb = &F->getEntryBlock();

    auto *a = dyn_cast<AllocaInst>(&eb->front());
    a->print(llvm::errs());
    assert(PointerMayBeCaptured(a, true, true) == true);

    F->print(llvm::errs(), nullptr);
  }

  return 0;
}