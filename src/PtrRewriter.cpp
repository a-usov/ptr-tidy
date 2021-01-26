#include "PtrRewriter.h"
#include "Helper.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

void PtrRewriter::initialise(SourceManager &sourceManager, const LangOptions &langOptions) {
  m_rewriter.setSourceMgr(sourceManager, langOptions);
}

void PtrRewriter::changeFunctionReturn(const FunctionDecl *function) {
  if (function && function->getReturnType()->isPointerType()) {
    std::string newType = (boost::format(shared_ptr) %
                           function->getReturnType()->getPointeeType().getLocalUnqualifiedType().getAsString())
                              .str();
    m_rewriter.ReplaceText(function->getReturnTypeSourceRange(), newType);
  }
}

void PtrRewriter::changeDeclaration(const VarDecl *var) {
  if (var && var->getType()->isPointerType()) {
    std::string newType =
        (boost::format(shared_ptr) % var->getType()->getPointeeType().getLocalUnqualifiedType().getAsString()).str();

    m_rewriter.ReplaceText(SourceRange(var->getTypeSpecStartLoc(), var->getTypeSpecEndLoc()), newType);
  }
}

void PtrRewriter::changeInit(const VarDecl *var) {
  std::string type = var->getType()->getPointeeType().getUnqualifiedType().getAsString();
  std::string value;

  if (auto init = var->getInit(); init) {
    rewriteInit(var, init);
  } else {
    handleDeferredInit(var);
  }
}

void PtrRewriter::handleDeferredInit(const VarDecl *initial) {
  using namespace clang::ast_matchers;
  using namespace clang::tooling;

  class TestCallback : public MatchFinder::MatchCallback {
    PtrRewriter *me;
    const VarDecl *m_initial;

    void run(const MatchFinder::MatchResult &result) override {
      auto bop = result.Nodes.getNodeAs<BinaryOperator>("");
      if (auto declRef = dyn_cast<DeclRefExpr>(bop->getLHS());
          declRef && declRef->getDecl()->getID() == m_initial->getID()) {
        me->rewriteInit(m_initial, bop->getRHS());
      }
    }

  public:
    TestCallback(PtrRewriter *ptrRewriter, const VarDecl *var) : me(ptrRewriter), m_initial(var){};
  };

  TestCallback callback{this, initial};
  MatchFinder Finder;

  Finder.addMatcher(binaryOperator().bind(""), &callback);
  std::unique_ptr<FrontendActionFactory> Factory{newFrontendActionFactory(&Finder)};
  runToolOnCode(Factory->create(), m_rewriter.getSourceMgr().getBufferData(m_rewriter.getSourceMgr().getMainFileID()));
}

void PtrRewriter::rewriteInit(const VarDecl *decl, const Expr *init) {
  std::string type = decl->getType()->getPointeeType().getUnqualifiedType().getAsString();
  std::string value;

  init = init->IgnoreImpCasts();
  if (auto newExpr = dyn_cast<CXXNewExpr>(init)) {
    value = strFromSourceRange(newExpr->getInitializer()->getSourceRange(), m_rewriter.getSourceMgr(),
                               m_rewriter.getLangOpts())
                .str();
  } else if (auto newRef = dyn_cast<DeclRefExpr>(init)) {
    value = newRef->getDecl()->getNameAsString();
  } else {
    // TODO - handle dereference of non-pointer - UnaryOperator 0x5602f12b4d98 'int *' prefix '&' cannot overflow
    init->dump();
    exit("Could not match expression for rewriting of initializer");
  }

  std::string newInit = (boost::format(make_shared) % type % value).str();
  m_rewriter.ReplaceText(init->getSourceRange(), newInit);
}
