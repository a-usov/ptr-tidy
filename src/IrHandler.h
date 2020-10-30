#pragma once

#include "boost/optional.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

class IrHandler {
  llvm::LLVMContext m_context;
  std::unique_ptr<llvm::Module> m_module;

public:
  explicit IrHandler(const llvm::MemoryBufferRef &moduleContent);

  [[nodiscard]] boost::optional<llvm::Module &> getModule() const;
};
