#include "AstHandler.h"
#include "Callback.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }

  auto sourceFileOrError = llvm::MemoryBuffer::getFile(argv[1]);
  if (!sourceFileOrError) {
    return sourceFileOrError.getError().value();
  }

  CountCallback callback(argv[1]);
  AstHandler::runCallback(callback, sourceFileOrError.get()->getBuffer());

  callback.getRewriter().getEditBuffer(callback.getRewriter().getSourceMgr().getMainFileID()).write(llvm::errs());
}
