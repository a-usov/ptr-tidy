#include "PtrRewriter.h"
#include "Helper.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/FormatVariadic.h"

using namespace clang;
using namespace clang::ast_matchers;

class LambdaCallback : public MatchFinder::MatchCallback {
  std::function<void(const MatchFinder::MatchResult &result)> m_lambda;

  void run(const MatchFinder::MatchResult &result) override { m_lambda(result); }

public:
  explicit LambdaCallback(auto lambda) : m_lambda(std::move(lambda)){};
};

llvm::StringRef strFromSourceRange(const SourceRange &sourceRange, const SourceManager &sm, const LangOptions &lo) {
  return Lexer::getSourceText(Lexer::getAsCharRange(sourceRange, sm, lo), sm, lo);
}

template <typename Tool>
void PtrRewriter<Tool>::initialise(SourceManager &sourceManager, const LangOptions &langOptions) {
  if (!m_initialised) {
    m_rewriter.setSourceMgr(sourceManager, langOptions);
    m_initialised = true;
  }
}

template <typename Tool> void PtrRewriter<Tool>::rewrite(const VarDecl *var) {
  if (var && var->getType()->isPointerType()) {
    rewriteDeclaration(var);
    rewriteInit(var);
    removeDelete(var);
    rewriteFunctionReturn(var);
  }
}

template <typename Tool> void PtrRewriter<Tool>::rewriteDeclaration(const VarDecl *var) {
  m_rewriter.ReplaceText(
      SourceRange(var->getTypeSpecStartLoc(), var->getTypeSpecEndLoc()),
      llvm::formatv(shared_ptr, var->getType()->getPointeeType().getUnqualifiedType().getAsString()).str());
}

template <typename Tool> void PtrRewriter<Tool>::rewriteInit(const VarDecl *var) {
  if (const auto init = var->getInit(); init) {
    rewriteInit(var, init);
  } else {
    rewriteDeferredInit(var);
  }
}

template <typename Tool> void PtrRewriter<Tool>::rewriteInit(const VarDecl *var, const Expr *init) {
  // TODO check this - are we ignoring enough
  init = init->IgnoreImpCasts();

  auto type = var->getType()->getPointeeType().getUnqualifiedType().getAsString();

  llvm::StringRef value;
  if (auto newExpr = dyn_cast<CXXNewExpr>(init); newExpr) {
    value = strFromSourceRange(newExpr->getInitializer()->getSourceRange(), m_rewriter.getSourceMgr(),
                               m_rewriter.getLangOpts());
  } else {
    value = strFromSourceRange(init->getSourceRange(), m_rewriter.getSourceMgr(), m_rewriter.getLangOpts());
  }

  m_rewriter.ReplaceText(init->getSourceRange(), llvm::formatv(make_shared, type, value).str());
}

template <typename Tool> void PtrRewriter<Tool>::rewriteDeferredInit(const VarDecl *var) {
  auto f = [&](const MatchFinder::MatchResult &result) {
    auto bop = result.Nodes.getNodeAs<BinaryOperator>("");
    auto declRef = dyn_cast<DeclRefExpr>(bop->getLHS());
    if (declRef && declRef->getDecl()->getID() == var->getID()) {
      rewriteInit(var, bop->getRHS());
    }
  };

  LambdaCallback callback(f);
  MatchFinder finder;
  finder.addMatcher(binaryOperator().bind(""), &callback);
  m_tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
}

template <typename Tool> void PtrRewriter<Tool>::removeDelete(const clang::VarDecl *var) {
  auto f = [&](const MatchFinder::MatchResult &result) {
    auto del = result.Nodes.getNodeAs<CXXDeleteExpr>("");
    auto declRef = dyn_cast<DeclRefExpr>(del->getArgument()->IgnoreImpCasts());
    if (declRef && declRef->getDecl()->getID() == var->getID()) {
      m_rewriter.RemoveText(SourceRange(del->getBeginLoc(), del->getEndLoc().getLocWithOffset(1)));
    }
  };

  LambdaCallback callback(f);
  MatchFinder finder;
  finder.addMatcher(cxxDeleteExpr().bind(""), &callback);
  m_tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
}

template <typename Tool> void PtrRewriter<Tool>::rewriteFunctionReturn(const VarDecl *var) {
  auto function = dyn_cast<FunctionDecl>(var->getParentFunctionOrMethod());
  if (function && function->getReturnType()->isPointerType()) {

    auto f = [&](const MatchFinder::MatchResult &result) {
      auto ret = result.Nodes.getNodeAs<ReturnStmt>("");
      auto declRef = dyn_cast<DeclRefExpr>(ret->getRetValue()->IgnoreImpCasts());
      if (declRef && declRef->getDecl()->getID() == var->getID()) {
        m_rewriter.ReplaceText(
            function->getReturnTypeSourceRange(),
            llvm::formatv(shared_ptr, function->getReturnType()->getPointeeType().getUnqualifiedType().getAsString())
                .str());
      }
    };

    LambdaCallback callback(f);
    MatchFinder finder;
    finder.addMatcher(returnStmt().bind(""), &callback);
    m_tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
  }
}

template class PtrRewriter<clang::tooling::ClangTool>;
template class PtrRewriter<TestTool>;
