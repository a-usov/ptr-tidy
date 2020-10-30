#include "IrHandler.h"

#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;
using namespace llvm;

IrHandler::IrHandler(const std::string& codePath) {
  CompilerInstance Clang;
  Clang.createDiagnostics();

  std::vector<const char *> args{"-triple=x86_64-unknown-linux-gnu"};
  args.push_back(codePath.c_str());
  CompilerInvocation::CreateFromArgs(Clang.getInvocation(), makeArrayRef(args),
                                     Clang.getDiagnostics());

  EmitLLVMOnlyAction action(&m_context);
  Clang.ExecuteAction(action);

  m_module = action.takeModule();
}

boost::optional<llvm::Module &> IrHandler::getModule() const {
  return m_module ? boost::optional<llvm::Module &>{*m_module} : boost::none;
}
