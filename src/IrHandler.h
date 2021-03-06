#pragma once

#include "boost/optional.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class IrHandler {
  llvm::LLVMContext m_context;
  std::unique_ptr<llvm::Module> m_module;

public:
  explicit IrHandler(llvm::StringRef codePath);

  [[nodiscard]] boost::optional<llvm::Module &> getModule() const;
};
