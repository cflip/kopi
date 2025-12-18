#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>

enum class TokenType {
    EoF,

    // Symbols
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    Semicolon,

    // Keywords
    Public,
    Int,
    Return,

    // Parts
    Identifier,
    Number,
};

struct Token {
    TokenType type;
    std::string contents;
};

Token getToken(std::ifstream &in)
{
    int tokenChar;
    std::string word;

    tokenChar = in.get();

    if (tokenChar == EOF)
        return { TokenType::EoF, "_done_" };

    // Ignore whitespace characters
    while (isspace(tokenChar))
        tokenChar = in.get();

    // Read keywords and identifiers
    if (isalpha(tokenChar)) {
        while (isalnum(tokenChar)) {
            word += tokenChar;
            tokenChar = in.get();
        }

        in.unget();
        if (word == "public") {
            return { TokenType::Public, word };
        }
        if (word == "int") {
            return { TokenType::Int, word };
        }
        if (word == "return") {
            return { TokenType::Return, word };
        }

        return { TokenType::Identifier, word };
    }

    if (isdigit(tokenChar)) {
        while (isdigit(tokenChar)) {
            word += tokenChar;
            tokenChar = in.get();
        }
        in.unget();
        return { TokenType::Number, word };
    }

    switch (tokenChar) {
    case '(': return { TokenType::OpenBracket, "(" };
    case ')': return { TokenType::CloseBracket, ")" };
    case '{': return { TokenType::OpenBrace, "{" };
    case '}': return { TokenType::CloseBrace, "}" };
    case ';': return { TokenType::Semicolon, ";" };
    default:
        assert(false && "Unrecognized token");
        return { TokenType::EoF, "POOP!!" };
    }
}

Token getToken(std::ifstream &in, TokenType expectedType)
{
    Token result = getToken(in);
    if (result.type != expectedType) {
        std::cerr << "WRONG TOKEN TYPE!";
    }
    return result;
}

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

std::unique_ptr<ASTNode> parse(std::ifstream &in)
{
    // Parse function
    getToken(in, TokenType::Public);
    getToken(in, TokenType::Int);
    Token ident = getToken(in, TokenType::Identifier);

    getToken(in, TokenType::OpenBracket);
    getToken(in, TokenType::CloseBracket);
    getToken(in, TokenType::OpenBrace);

        // Parse statement
        getToken(in, TokenType::Return);
            
            // Parse expression
            Token returnValue = getToken(in, TokenType::Number);
            auto expr = std::make_unique<ExprASTNode>(returnValue);

        getToken(in, TokenType::Semicolon);
        auto stmt = std::make_unique<StmtASTNode>(expr);

    getToken(in, TokenType::CloseBrace);
    
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

    auto ast = parse(infile);
    ast->dbgprint();

    infile.close();

    return EXIT_SUCCESS;
}
