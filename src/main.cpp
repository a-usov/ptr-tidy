#include "Callback.h"
#include "Helper.h"
#include "clang/Tooling/CompilationDatabase.h"

using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm::sys;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    exit("Usage: ./ptr-tidy input-file.cpp");
  }

  auto path = llvm::SmallString<128>(argv[1]);
  fs::make_absolute(path);
  fs::exists(path);
  if (!fs::exists(path)) {
    exit("Input file does not exist: " + path);
  }

  std::string error;
  const auto compilationDatabase = CompilationDatabase::autoDetectFromSource(path, error);
  ClangTool tool{*compilationDatabase, {path.c_str()}};
  // We need value names to match AST to IR
  tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("-fno-discard-value-names"));

  // TODO this only allows for support for single file processing, need to pass modules for all files if expanded
  llvm::LLVMContext context;
  auto module = getModule(tool, context);

  Callback callback{tool, std::move(module), path};
  MatchFinder finder;
  finder.addMatcher(varDecl().bind(""), &callback);
  tool.run(newFrontendActionFactory(&finder).get());

  const auto &rewriter = callback.getRewriter();
  for (auto it = rewriter.buffer_begin(); it != rewriter.buffer_end(); it++) {
    it->second.write(llvm::outs());
  }
}
