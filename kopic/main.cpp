#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>

#include "token.hpp"

// <FUNC> : public int <IDENTIFIER> ( ) { <STMT> }
// <STMT> : return <EXPR> ;
// <EXPR> : <INTEGER>

enum class Visibility {
    Private,
    Protected,
    Public
};

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void dbgprint(int indent = 0) {
        for (int i = 0; i < indent; i++)
            std::cout << '\t';
    }
};

class ExprASTNode : public ASTNode {
public:
    ExprASTNode(Token number) : number(number) {}

    virtual void dbgprint(int indent) override {
        ASTNode::dbgprint(indent);
        std::cout << "<expr> -> " << number.contents << std::endl;
    }
private:
    Token number;
};

class StmtASTNode : public ASTNode {
public:
    StmtASTNode(std::unique_ptr<ExprASTNode> &expr) : expr(std::move(expr)) {}
    
    virtual void dbgprint(int indent) override {
        ASTNode::dbgprint(indent);
        std::cout << "<stmt>" << '\n';
        expr->dbgprint(indent + 1);
    }
private:
    std::unique_ptr<ExprASTNode> expr;
};

class FuncASTNode : public ASTNode {
public:
    FuncASTNode(Token ident, std::unique_ptr<StmtASTNode> &body)
        : vis(Visibility::Public), identifier(ident), body(std::move(body)) {}
     
    virtual void dbgprint(int indent) override {
        ASTNode::dbgprint(indent);
        std::cout << "<func> " << identifier.contents << '\n';
        body->dbgprint(indent + 1);
    }
private:
    Visibility vis;
    Token identifier;
    std::unique_ptr<StmtASTNode> body;
};

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer)
{
    // Parse function
    tokenizer.expectNext(TokenType::Public);
    tokenizer.expectNext(TokenType::Int);
    Token ident = tokenizer.expectNext(TokenType::Identifier);

    tokenizer.expectNext(TokenType::OpenBracket);
    tokenizer.expectNext(TokenType::CloseBracket);

    tokenizer.expectNext(TokenType::OpenBrace);

        // Parse statement
        tokenizer.expectNext(TokenType::Return);
            
            // Parse expression
            Token returnValue = tokenizer.expectNext(TokenType::Number);
            auto expr = std::make_unique<ExprASTNode>(returnValue);

        tokenizer.expectNext(TokenType::Semicolon);
        auto stmt = std::make_unique<StmtASTNode>(expr);

    tokenizer.expectNext(TokenType::CloseBrace);
    
    auto func = std::make_unique<FuncASTNode>(ident, stmt);
    return func;
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
    try {
        auto ast = parse(tokenizer);
        ast->dbgprint();
    } catch (std::exception &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        infile.close();
        return EXIT_FAILURE;
    }

    infile.close();

    return EXIT_SUCCESS;
}
