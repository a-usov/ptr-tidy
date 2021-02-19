#include "PtrRewriter.h"
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

void PtrRewriter::rewrite(const VarDecl *var) {
  rewriteDeclaration(var);
  rewriteInit(var);
  removeDelete(var);
}

void PtrRewriter::initialise(SourceManager &sourceManager, const LangOptions &langOptions) {
  if (!m_initialised) {
    m_rewriter.setSourceMgr(sourceManager, langOptions);
    m_initialised = true;
  }
}

void PtrRewriter::rewriteFunctionReturn(const FunctionDecl *function) {
  if (function && function->getReturnType()->isPointerType()) {
    m_rewriter.ReplaceText(
        function->getReturnTypeSourceRange(),
        llvm::formatv(shared_ptr, function->getReturnType()->getPointeeType().getUnqualifiedType().getAsString())
            .str());
  }
}

void PtrRewriter::rewriteDeclaration(const VarDecl *var) {
  if (var && var->getType()->isPointerType()) {
    m_rewriter.ReplaceText(
        SourceRange(var->getTypeSpecStartLoc(), var->getTypeSpecEndLoc()),
        llvm::formatv(shared_ptr, var->getType()->getPointeeType().getUnqualifiedType().getAsString()).str());
  }
}

void PtrRewriter::rewriteInit(const VarDecl *var) {
  if (const auto init = var->getInit(); init) {
    rewriteInit(var, init);
  } else {
    rewriteDeferredInit(var);
  }
}

void PtrRewriter::rewriteInit(const VarDecl *var, const Expr *init) {
  // TODO check this - are we ignoring enough
  init = init->IgnoreImpCasts();

  auto type = var->getType()->getPointeeType().getUnqualifiedType().getAsString();
  auto value = strFromSourceRange(init->getSourceRange(), m_rewriter.getSourceMgr(), m_rewriter.getLangOpts());

  m_rewriter.ReplaceText(init->getSourceRange(), llvm::formatv(make_shared, type, value).str());
}

void PtrRewriter::rewriteDeferredInit(const VarDecl *var) {
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

void PtrRewriter::removeDelete(const clang::VarDecl *var) {
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
