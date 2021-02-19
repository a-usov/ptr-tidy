#include "IrHandler.h"
#include "Helper.h"
#include "clang/CodeGen/CodeGenAction.h"

// Need these custom classes to be able to extract module after using ClangTool with action factory
class EmitLLVMAction : public clang::EmitLLVMOnlyAction {
  std::vector<std::unique_ptr<llvm::Module>> &m_modules;

public:
  explicit EmitLLVMAction(std::vector<std::unique_ptr<llvm::Module>> &modules, llvm::LLVMContext &context)
      : clang::EmitLLVMOnlyAction(&context), m_modules(modules) {}

  void EndSourceFileAction() override {
    CodeGenAction::EndSourceFileAction();

    m_modules.emplace_back(takeModule());
  }
};

class EmitLLVMActionFactory : public clang::tooling::FrontendActionFactory {
  std::vector<std::unique_ptr<llvm::Module>> &m_modules;
  llvm::LLVMContext &m_context;

public:
  EmitLLVMActionFactory(std::vector<std::unique_ptr<llvm::Module>> &modules, llvm::LLVMContext &context)
      : m_modules(modules), m_context(context) {}

  std::unique_ptr<clang::FrontendAction> create() override {
    return std::make_unique<EmitLLVMAction>(m_modules, m_context);
  }
};

std::unique_ptr<llvm::Module> getModule(clang::tooling::ClangTool &tool, llvm::LLVMContext &context) {
  std::vector<std::unique_ptr<llvm::Module>> modules;

  auto factory = std::make_unique<EmitLLVMActionFactory>(modules, context);
  tool.run(factory.get());

  // TODO - check if we need to use more than first
  std::unique_ptr<llvm::Module> module{std::move(modules.front())};

  if (!module) {
    exit("Could not get LLVM IR module source file");
  }

  llvm::errs() << *module;
  return module;
}
