#pragma once

#include "boost/optional.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include <string>

class IrHandler {
  llvm::LLVMContext m_context;
  std::unique_ptr<llvm::Module> m_module;

public:
  explicit IrHandler(const std::string& codePath);

  [[nodiscard]] boost::optional<llvm::Module &> getModule() const;
};
