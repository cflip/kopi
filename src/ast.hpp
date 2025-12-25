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
    explicit NumericExprASTNode(Token number) : number(number) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token number;
};

class IdentifierExprASTNode : public ExprASTNode {
  public:
    explicit IdentifierExprASTNode(Token ident) : identifier(ident) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token identifier;
};

class FuncCallExprASTNode : public ExprASTNode {
  public:
    FuncCallExprASTNode(Token function, std::vector<std::unique_ptr<ExprASTNode>> arguments)
        : function(function), arguments(std::move(arguments)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token function;
    std::vector<std::unique_ptr<ExprASTNode>> arguments;
};

class UnaryOpExprASTNode : public ExprASTNode {
  public:
    UnaryOpExprASTNode(Token op, std::unique_ptr<ExprASTNode> expr) : op(op), operand(std::move(expr)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token op;
    std::unique_ptr<ExprASTNode> operand;
};

class BinaryOpExprASTNode : public ExprASTNode {
  public:
    BinaryOpExprASTNode(Token op, std::unique_ptr<ExprASTNode> l, std::unique_ptr<ExprASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {
    }

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
    explicit ReturnStmtASTNode(std::unique_ptr<ExprASTNode> &expr) : expr(std::move(expr)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    std::unique_ptr<ExprASTNode> expr;
};

class VariableDeclStmtASTNode : public StmtASTNode {
  public:
    VariableDeclStmtASTNode(Token ident, std::unique_ptr<ExprASTNode> init)
        : identifier(ident), initExpr(std::move(init)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Token identifier;
    std::unique_ptr<ExprASTNode> initExpr;
};

class CompoundStmtASTNode : public StmtASTNode {
  public:
    explicit CompoundStmtASTNode(std::vector<std::unique_ptr<StmtASTNode>> list) : stmts(std::move(list)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    std::vector<std::unique_ptr<StmtASTNode>> stmts;
};

enum class Visibility { Private, Protected, Public };

class FuncASTNode : public ASTNode {
  public:
    FuncASTNode(Token ident, std::vector<Token> params, std::unique_ptr<CompoundStmtASTNode> &body)
        : vis(Visibility::Public), identifier(ident), params(params), body(std::move(body)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    Visibility vis;
    Token identifier;
    std::vector<Token> params;
    std::unique_ptr<CompoundStmtASTNode> body;
};

class SourceFileASTNode : public ASTNode {
  public:
    explicit SourceFileASTNode(std::vector<std::unique_ptr<FuncASTNode>> funcs) : functions(std::move(funcs)) {
    }

    llvm::Value *emit() const override;
    void dbgprint(int indent) const override;

  private:
    std::vector<std::unique_ptr<FuncASTNode>> functions;
};

bool codegenInit(const std::string &moduleName);
void codegenPrintIR();
void codegenOutput(const std::string &filename);
