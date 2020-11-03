#include "IrHandler.h"
#include <sstream>

#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;
using namespace llvm;

IrHandler::IrHandler(const llvm::StringRef codePath) {
  CompilerInstance Clang;
  Clang.createDiagnostics();

  std::vector<const char *> args;

  args.emplace_back(codePath.data());

  std::ostringstream input;
  input << "-triple=" << llvm::sys::getDefaultTargetTriple();
  args.emplace_back(input.str().c_str());

  CompilerInvocation::CreateFromArgs(Clang.getInvocation(), makeArrayRef(args),
                                     Clang.getDiagnostics());

  EmitLLVMOnlyAction action(&m_context);
  Clang.ExecuteAction(action);

  m_module = action.takeModule();

  errs() << *m_module;
}

boost::optional<llvm::Module &> IrHandler::getModule() const {
  return m_module ? boost::optional<llvm::Module &>{*m_module} : boost::none;
}