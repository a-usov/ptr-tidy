#pragma once

#include "clang/Tooling/Tooling.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

std::unique_ptr<llvm::Module> getModule(clang::tooling::ClangTool &tool, llvm::LLVMContext &context);
