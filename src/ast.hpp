#pragma once

#include <llvm/IR/Value.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "token.hpp"

class ASTNode {
  public:
    virtual ~ASTNode() = default;
    virtual llvm::Value *emit() const = 0;
    virtual void dbgprint(int indent = 0) const;
};

class ExprASTNode : public ASTNode {};

class NumericExprASTNode : public ExprASTNode {
  public:
    explicit NumericExprASTNode(Token number) : number(number) {}

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token number;
};

class BinaryOpExprASTNode : public ExprASTNode {
  public:
    BinaryOpExprASTNode(Token op, std::unique_ptr<ExprASTNode> l,
                        std::unique_ptr<ExprASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token op;
    std::unique_ptr<ExprASTNode> left;
    std::unique_ptr<ExprASTNode> right;
};

class StmtASTNode : public ASTNode {};

class ReturnStmtASTNode : public StmtASTNode {
  public:
    explicit ReturnStmtASTNode(std::unique_ptr<ExprASTNode> &expr)
        : expr(std::move(expr)) {}

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    std::unique_ptr<ExprASTNode> expr;
};

class CompoundStmtASTNode : public StmtASTNode {
  public:
    explicit CompoundStmtASTNode(std::vector<std::unique_ptr<StmtASTNode>> list)
        : stmts(std::move(list)) {}

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    std::vector<std::unique_ptr<StmtASTNode>> stmts;
};

enum class Visibility { Private, Protected, Public };

class FuncASTNode : public ASTNode {
  public:
    FuncASTNode(Token ident, std::unique_ptr<CompoundStmtASTNode> &body)
        : vis(Visibility::Public), identifier(ident), body(std::move(body)) {}

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Visibility vis;
    Token identifier;
    std::unique_ptr<CompoundStmtASTNode> body;
};

bool codegenInit(const std::string &moduleName);
void codegenPrintIR(const std::string &filename);
