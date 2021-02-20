#include "Helper.h"
#include "PtrRewriter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "gtest/gtest.h"

using namespace clang::ast_matchers;
using namespace clang::tooling;

// returns empty string if input code was unchanged
std::string compareRewriterOutput(clang::Rewriter &rewriter) {
  std::string output;
  llvm::raw_string_ostream os{output};
  auto buffer = rewriter.getRewriteBufferFor(rewriter.getSourceMgr().getMainFileID());
  if (buffer) {
    buffer->write(os);
  }

  os.flush();
  return output;
}

class LambdaCallback : public MatchFinder::MatchCallback {
  std::function<void(const MatchFinder::MatchResult &result)> m_lambda;

  void run(const MatchFinder::MatchResult &result) override { m_lambda(result); }

public:
  explicit LambdaCallback(std::function<void(const MatchFinder::MatchResult &result)> lambda)
      : m_lambda(std::move(lambda)){};
};

class PtrRewriterTest : public ::testing::Test {
protected:
  TestTool tool;
  PtrRewriter<TestTool> rewriter{tool};
  std::unique_ptr<FrontendActionFactory> factory;
  MatchFinder finder;

  LambdaCallback callback{[&](const MatchFinder::MatchResult &result) {
    rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

    auto var = result.Nodes.getNodeAs<clang::VarDecl>("");

    if (var->isImplicit()) {
      return;
    }
    rewriter.rewrite(var);
  }};

  PtrRewriterTest() {
    finder.addMatcher(varDecl().bind(""), &callback);
    factory = newFrontendActionFactory(&finder);
  }
};

TEST_F(PtrRewriterTest, FunctionReturnTypeChanged) {
  const char *code = "int *foo(){int *a = new int(2); return a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeNotChangedForNonPointer) {
  const char *code = "int foo(){int a = 2; return a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeRewriteHandlesConst) {
  const char *code = "const int *foo(){int *a = new int(2); return a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "const std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeRewriteHandlesStatic) {
  const char *code = "static int *foo(){int *a = new int(2); return a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "static std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

// TODO handle classes properly
TEST_F(PtrRewriterTest, DISABLED_ReferenceFunctionReturnTypeUnchanged) {
  const char *code = "class A { int m_int; int &getInt(){return m_int;}};";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, PointerVariableDeclarationTypeChanged) {
  const char *code = "void test(){int *a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "void test(){std::shared_ptr<int> a;}");
}

TEST_F(PtrRewriterTest, NonPointerVariableDeclarationTypeUnChanged) {
  const char *code = "void test(){int a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);

  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, InitialisationNewChanged) {
  const char *code = "void test(){int *a = new int(2);}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a = std::make_shared<int>(2);}");
}

TEST_F(PtrRewriterTest, InitialisationDeferredNewChanged) {
  const char *code = "void test(){int *a; a = new int(2);}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2);}");
}

TEST_F(PtrRewriterTest, InitialisationDeferredChanged) {
  const char *code = "void test(){int *a; a = new int(2); int *b; b = a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2); std::shared_ptr<int> b; b = "
            "std::make_shared<int>(a);}");
}

TEST_F(PtrRewriterTest, InitialisationThenDeletion) {
  const char *code = "void test(){int *a = new int(2); delete a;}";
  tool.code = code;

  runToolOnCode(factory->create(), code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a = std::make_shared<int>(2); }");
}
