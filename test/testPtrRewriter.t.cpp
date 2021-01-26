#include "PtrRewriter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "gtest/gtest.h"

using namespace clang;
using namespace llvm;
using namespace clang::ast_matchers;
using namespace clang::tooling;

std::string compareRewriterOutput(Rewriter &rewriter) {
  std::string output;
  raw_string_ostream os{output};
  rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(os);
  os.flush();
  return output;
}

TEST(PtrRewriterTest, FunctionReturnTypeChanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto function = result.Nodes.getNodeAs<FunctionDecl>("");
      rewriter.changeFunctionReturn(function);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "std::shared_ptr<int> foo(){return nullptr;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(functionDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "int *foo(){return nullptr;}");
}

TEST(PtrRewriterTest, FunctionReturnTypeRewriteHandlesConst) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto function = result.Nodes.getNodeAs<FunctionDecl>("");
      rewriter.changeFunctionReturn(function);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "const std::shared_ptr<int> foo(){return nullptr;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(functionDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "const int *foo(){return nullptr;}");
}

TEST(PtrRewriterTest, FunctionReturnTypeRewriteHandlesStatic) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto function = result.Nodes.getNodeAs<FunctionDecl>("");
      rewriter.changeFunctionReturn(function);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "static std::shared_ptr<int> foo(){return nullptr;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(functionDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "static int *foo(){return nullptr;}");
}

TEST(PtrRewriterTest, NonPointerFunctionReturnTypeUnchanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto function = result.Nodes.getNodeAs<FunctionDecl>("");
      rewriter.changeFunctionReturn(function);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "int foo(){return 0;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(functionDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "int foo(){return 0;}");
}

TEST(PtrRewriterTest, ReferenceFunctionReturnTypeUnchanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto function = result.Nodes.getNodeAs<FunctionDecl>("");
      rewriter.changeFunctionReturn(function);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "class A { int m_int; int &getInt(){return m_int;}};");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(functionDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "class A { int m_int; int &getInt(){return m_int;}};");
}

TEST(PtrRewriterTest, PointerVariableDeclarationTypeChanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto var = result.Nodes.getNodeAs<VarDecl>("");
      rewriter.changeDeclaration(var);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "void test(){std::shared_ptr<int> a;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "void test(){int *a;}");
}

TEST(PtrRewriterTest, NonPointerVariableDeclarationTypeUnChanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto var = result.Nodes.getNodeAs<VarDecl>("");
      rewriter.changeDeclaration(var);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()), "void test(){int a;}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "void test(){int a;}");
}

TEST(PtrRewriterTest, InitialisationNewChanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {

      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto var = result.Nodes.getNodeAs<VarDecl>("");

      if (var->isImplicit()) {
        return;
      }
      rewriter.changeDeclaration(var);
      rewriter.changeInit(var);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
                "void test(){std::shared_ptr<int> a = std::make_shared<int>(2);}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "void test(){int *a = new int(2);}");
}

TEST(PtrRewriterTest, InitialisationDeferredNewChanged) {

  class TestCallback : public MatchFinder::MatchCallback {

    void run(const MatchFinder::MatchResult &result) override {
      PtrRewriter rewriter;
      rewriter.initialise(*result.SourceManager, result.Context->getLangOpts());

      auto var = result.Nodes.getNodeAs<VarDecl>("");

      if (var->isImplicit()) {
        return;
      }
      rewriter.changeDeclaration(var);
      rewriter.changeInit(var);

      ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
                "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2);}");
    }
  };

  TestCallback callback;
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "void test(){int *a; a = new int(2);}");
}

TEST(PtrRewriterTest, InitialisationDeferredChanged) {
  PtrRewriter rewriter;

  class TestCallback : public MatchFinder::MatchCallback {
    PtrRewriter *rewriter;

    void run(const MatchFinder::MatchResult &result) override {
      rewriter->initialise(*result.SourceManager, result.Context->getLangOpts());

      auto var = result.Nodes.getNodeAs<VarDecl>("");

      if (var->isImplicit()) {
        return;
      }
      rewriter->changeDeclaration(var);
      rewriter->changeInit(var);
    }

  public:
    explicit TestCallback(PtrRewriter *ptrRewriter) : rewriter(ptrRewriter){};
  };

  TestCallback callback{&rewriter};
  MatchFinder Finder;

  Finder.addMatcher(varDecl().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory(newFrontendActionFactory(&Finder));
  runToolOnCode(Factory->create(), "void test(){int *a; a = new int(2); int *b; b = a;}");

  ASSERT_EQ(compareRewriterOutput(rewriter.getRewriter()),
            "void test(){std::shared_ptr<int> a; a = std::make_shared<int>(2); std::shared_ptr<int> b; b = "
            "std::make_shared<int>(a);}");
}
