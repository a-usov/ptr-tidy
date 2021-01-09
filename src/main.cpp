#include "AstHandler.h"
#include "Callback.h"
#include "Helper.h"
#include <sstream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }

  auto sourceFile = getFile(argv[1]);
  if (!sourceFile) {
    std::ostringstream ss;
    ss << "Cannot open source file " << argv[1];
    exit(ss.str());
  }

  CountCallback callback(argv[1]);
  AstHandler::runCallback(callback, sourceFile.value()->getBuffer());

  callback.getRewriter().getEditBuffer(callback.getRewriter().getSourceMgr().getMainFileID()).write(llvm::errs());
}
