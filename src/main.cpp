#include "llvm/Analysis/CaptureTracking.h"

#include "AstHandler.h"
#include "Callback.h"
#include "IrHandler.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    return -1;
  }

  auto sourceFileOrError = llvm::MemoryBuffer::getFile(argv[1]);
  if (!sourceFileOrError) {
    return sourceFileOrError.getError().value();
  }

  auto irFileOrError = llvm::MemoryBuffer::getFile(argv[2]);
  if (!irFileOrError) {
    return irFileOrError.getError().value();
  }

  CountCallback callback;
  AstHandler::runCallback(callback, sourceFileOrError.get()->getBuffer());
  llvm::errs() << "Number of pointer dereferences is " << callback.getCount() << "\n";

  IrHandler irHandler{irFileOrError.get()->getMemBufferRef()};
  if (auto optionalModule = irHandler.getModule(); optionalModule) {
    auto &module = optionalModule.get();
    llvm::Function *F = module.getFunction("_Z3asdv");
    auto *eb = &F->getEntryBlock();

    llvm::errs() << eb->front() << " is captured "
                 << PointerMayBeCaptured(&eb->front(), true, true);
  }
}
