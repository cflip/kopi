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

void BinaryOpExprASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<expr_op> " << op.contents << '\n';
    left->dbgprint(indent + 1);
    right->dbgprint(indent + 1);
}

void ReturnStmtASTNode::dbgprint(int indent) const {
    ASTNode::dbgprint(indent);
    std::cout << "<stmt_ret>" << '\n';
    expr->dbgprint(indent + 1);
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
    std::cout << "<func> " << identifier.contents << '\n';
    body->dbgprint(indent + 1);
}
