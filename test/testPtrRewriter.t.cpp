#include "Helper.h"
#include "PtrRewriter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "gtest/gtest.h"

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

  // output must start with new header, but to simplify test scenario, we remove it from the string.
  auto header = "#include <memory>\n";
  assert(output.starts_with(header) || output.empty());
  output.erase(0, strlen(header));

  return output;
}

class PtrRewriterTest : public ::testing::Test {
protected:
  TestTool tool;
  PtrRewriter<TestTool> rewriter{tool};
  std::unique_ptr<FrontendActionFactory> factory;
  clang::ast_matchers::MatchFinder finder;

  LambdaCallback callback{[&](const clang::ast_matchers::MatchFinder::MatchResult &result) {
    rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

    auto var = result.Nodes.getNodeAs<clang::VarDecl>("");

    if (var->isImplicit()) {
      return;
    }
    rewriter.rewrite(var);
  }};

  PtrRewriterTest() {
    finder.addMatcher(clang::ast_matchers::varDecl().bind(""), &callback);
    factory = newFrontendActionFactory(&finder);
  }
};

TEST_F(PtrRewriterTest, FunctionReturnTypeChanged) {
  tool.code = "int *foo(){int *a = new int(2); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeNotChangedForNonPointer) {
  tool.code = "int foo(){int a = 2; return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeRewriteHandlesConst) {
  tool.code = "const int *foo(){int *a = new int(2); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "const std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

TEST_F(PtrRewriterTest, FunctionReturnTypeRewriteHandlesStatic) {
  tool.code = "static int *foo(){int *a = new int(2); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "static std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); return a;}");
}

// TODO handle classes properly
TEST_F(PtrRewriterTest, DISABLED_ReferenceFunctionReturnTypeUnchanged) {
  tool.code = "class A { int m_int; int &getInt(){return m_int;}};";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, PointerVariableDeclarationTypeChanged) {
  tool.code = "void test(){int *a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "void test(){std::shared_ptr<int> a;}");
}

TEST_F(PtrRewriterTest, NonPointerVariableDeclarationTypeUnChanged) {
  tool.code = "void test(){int a;}";

  runToolOnCode(factory->create(), tool.code);

  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "");
}

TEST_F(PtrRewriterTest, CallInitialisationNewChanged) {
  tool.code = "void test(){int *a = new int(2);}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a = std::make_shared<int>(2);}");
}

TEST_F(PtrRewriterTest, ListInitialisationNewChanged) {
  tool.code = "void test(){int *a = new int{2};}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a = std::make_shared<int>(2);}");
}

TEST_F(PtrRewriterTest, InitialisationDeferredNewChanged) {
  tool.code = "void test(){int *a; a = new int(2);}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2);}");
}

TEST_F(PtrRewriterTest, InitialisationDeferredChanged) {
  tool.code = "void test(){int *a; a = new int(2); int *b; b = a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2); std::shared_ptr<int> b; b = "
            "std::shared_ptr<int>(a);}");
}

TEST_F(PtrRewriterTest, InitialisationThenDeletion) {
  tool.code = "void test(){int *a = new int(2); delete a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a = std::make_shared<int>(2); }");
}

TEST_F(PtrRewriterTest, ReturnRewrittingHandlesEmptyReturn) {
  tool.code = "void test(){ return; } int *foo(){int *a = new int(2); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){ return; } std::shared_ptr<int> foo(){std::shared_ptr<int> a = std::make_shared<int>(2); "
            "return a;}");
}

TEST_F(PtrRewriterTest, HandleArrays) {
  tool.code = "void foo(){int *a = new int[2];}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void foo(){std::shared_ptr<int> a = std::shared_ptr<int>(new int[2]);}");
}

TEST_F(PtrRewriterTest, HandleClassesNoInit) {
  tool.code = "class A {}; A *test(){A *a = new A; return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "class A {}; std::shared_ptr<A> test(){std::shared_ptr<A> a = std::make_shared<A>(); return a;}");
}

TEST_F(PtrRewriterTest, HandleClassesCallInit) {
  tool.code = "class A {}; A *test(){A *a = new A(); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "class A {}; std::shared_ptr<A> test(){std::shared_ptr<A> a = std::make_shared<A>(); return a;}");
}

TEST_F(PtrRewriterTest, HandleClassesListInit) {
  tool.code = "class A {}; A *test(){A *a = new A{}; return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "class A {}; std::shared_ptr<A> test(){std::shared_ptr<A> a = std::make_shared<A>(); return a;}");
}

TEST_F(PtrRewriterTest, HandleStructNoInit) {
  tool.code = "struct A {}; A *test(){A *a = new A; return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "struct A {}; std::shared_ptr<A> test(){std::shared_ptr<A> a = std::make_shared<A>(); return a;}");
}

TEST_F(PtrRewriterTest, HandleStructListInitValue) {
  tool.code = "struct A {int m_a; A(int a) : m_a(a) {}}; A *test(){A *a = new A{1}; return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "struct A {int m_a; A(int a) : m_a(a) {}}; std::shared_ptr<A> test(){std::shared_ptr<A> a = "
            "std::make_shared<A>(1); return a;}");
}

TEST_F(PtrRewriterTest, HandleStructCallInitValue) {
  tool.code = "struct A {int m_a; A(int a) : m_a(a) {}}; A *test(){A *a = new A(1); return a;}";

  runToolOnCode(factory->create(), tool.code);
  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "struct A {int m_a; A(int a) : m_a(a) {}}; std::shared_ptr<A> test(){std::shared_ptr<A> a = "
            "std::make_shared<A>(1); return a;}");
}
