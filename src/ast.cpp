#include "ast.hpp"

#include <iostream>

void ASTNode::dbgprint(int indent) const {
    for (int i = 0; i < indent; i++)
        std::cout << '\t';
}

void NumericExprASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<expr_num> " << number.contents << std::endl;
}

void IdentifierExprASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<expr_ident> " << identifier.contents << std::endl;
}

void UnaryOpExprASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<expr_unop> " << op.contents << '\n';
    operand->dbgprint(indent + 1);
}

void BinaryOpExprASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<expr_binop> " << op.contents << '\n';
    left->dbgprint(indent + 1);
    right->dbgprint(indent + 1);
}

void ReturnStmtASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<stmt_ret>" << '\n';
    expr->dbgprint(indent + 1);
}

void VariableDeclStmtASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<stmt_vardecl> " << identifier.contents << '\n';
    if (initExpr) {
        initExpr->dbgprint(indent + 1);
    }
}

void CompoundStmtASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<stmt_block>" << '\n';
    for (auto &&stmt : stmts) {
        stmt->dbgprint(indent + 1);
    }
}

void FuncASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<func> " << identifier.contents;
    for (const auto &param : params) {
        std::cout << ' ' << param.contents;
    }
    std::cout << '\n';
    body->dbgprint(indent + 1);
}

void SourceFileASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<file>\n";
    for (const auto &func : functions) {
        func->dbgprint(indent + 1);
    }
}
