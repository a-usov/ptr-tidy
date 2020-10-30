#include "IrHandler.h"

IrHandler::IrHandler(const llvm::MemoryBufferRef &moduleContent) {
  llvm::SMDiagnostic error;

  m_module = parseIR(moduleContent, error, m_context);
}

boost::optional<llvm::Module &> IrHandler::getModule() const {
  return m_module ? boost::optional<llvm::Module &>{*m_module} : boost::none;
}
