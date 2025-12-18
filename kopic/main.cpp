#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "token.hpp"

enum class Visibility {
    Private,
    Protected,
    Public
};

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void dbgprint(int indent = 0) const {
        for (int i = 0; i < indent; i++)
            std::cout << '\t';
    }
};

class ExprASTNode : public ASTNode {
public:
    ExprASTNode(Token number) : number(number) {}

    virtual void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<expr> -> " << number.contents << std::endl;
    }
private:
    Token number;
};

class StmtASTNode : public ASTNode {
private:
};

class ReturnStmtASTNode : public StmtASTNode {
public:
    ReturnStmtASTNode(std::unique_ptr<ExprASTNode> &expr) : expr(std::move(expr)) {}
    
    virtual void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<return>" << '\n';
        expr->dbgprint(indent + 1);
    }
private:
    std::unique_ptr<ExprASTNode> expr;
};

class CompoundStmtASTNode : public StmtASTNode {
public:
    CompoundStmtASTNode(std::vector<std::unique_ptr<StmtASTNode>> list) : stmts(std::move(list)) {}

    virtual void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<block>" << '\n';
        for (const auto &stmt : stmts)
            stmt->dbgprint(indent + 1);
    }
private:
    std::vector<std::unique_ptr<StmtASTNode>> stmts;
};

class FuncASTNode : public ASTNode {
public:
    FuncASTNode(Token ident, std::unique_ptr<CompoundStmtASTNode> &body)
        : vis(Visibility::Public), identifier(ident), body(std::move(body)) {}
     
    virtual void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<func> " << identifier.contents << '\n';
        body->dbgprint(indent + 1);
    }
private:
    Visibility vis;
    Token identifier;
    std::unique_ptr<CompoundStmtASTNode> body;
};

std::unique_ptr<ExprASTNode> parseExpr(TokenReader &tokenizer)
{
    Token returnValue;
    if (!tokenizer.expectNext(TokenType::Number, &returnValue))
        return nullptr;
    return std::make_unique<ExprASTNode>(returnValue);
}

// Parse any kind of statement
std::unique_ptr<StmtASTNode> parseStmt(TokenReader &tokenizer)
{
    if (!tokenizer.expectNext(TokenType::Return))
        return nullptr;
    auto expr = parseExpr(tokenizer);
    if (expr == nullptr)
        return nullptr;
    if (!tokenizer.expectNext(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<ReturnStmtASTNode>(expr);
}

// Specifically parse a compound statement. Function bodies cannot be any other kind of statement.
std::unique_ptr<CompoundStmtASTNode> parseCompoundStmt(TokenReader &tokenizer)
{
    if (!tokenizer.expectNext(TokenType::OpenBrace))
        return nullptr;
    
    std::vector<std::unique_ptr<StmtASTNode>> stmts;
    while (tokenizer.peek() != TokenType::CloseBrace) {
        auto stmt = parseStmt(tokenizer);
        if (stmt == nullptr)
            return nullptr;
        stmts.emplace_back(std::move(stmt));
    }
    
    if (!tokenizer.expectNext(TokenType::CloseBrace))
        return nullptr;

    return std::make_unique<CompoundStmtASTNode>(std::move(stmts));
}

std::unique_ptr<FuncASTNode> parseFunction(TokenReader &tokenizer)
{
    // Parse function
    if (!tokenizer.expectNext(TokenType::Public))
        return nullptr;
    if (!tokenizer.expectNext(TokenType::Int))
        return nullptr;

    Token ident;
    if (!tokenizer.expectNext(TokenType::Identifier, &ident))
        return nullptr;

    if (!tokenizer.expectNext(TokenType::OpenBracket))
        return nullptr;
    if (!tokenizer.expectNext(TokenType::CloseBracket))
        return nullptr;

    auto stmt = parseCompoundStmt(tokenizer);
    if (stmt == nullptr)
        return nullptr;

    auto func = std::make_unique<FuncASTNode>(ident, stmt);
    return func;
}

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer)
{
    return parseFunction(tokenizer);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "no input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Unable to open " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    TokenReader tokenizer(infile);
    auto ast = parse(tokenizer);
    if (ast == nullptr) {
        infile.close();
        return EXIT_FAILURE;
    } else {    
        ast->dbgprint();
    }

    infile.close();

    return EXIT_SUCCESS;
}
