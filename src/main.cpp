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

  CountCallback callback{irFileOrError.get()->getMemBufferRef()};
  AstHandler::runCallback(callback, sourceFileOrError.get()->getBuffer());
}
