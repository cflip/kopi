#include "parser.hpp"

#include <iostream>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

static int precedence(TokenType type) {
    switch (type) {
    case TokenType::Plus:
    case TokenType::Minus:
        return 1;
    case TokenType::Multiply:
    case TokenType::Divide:
        return 2;
    default:
        return 0;
    }
}

static std::unique_ptr<ExprASTNode> parseExpr(TokenReader &tokenizer) {
    // https://en.wikipedia.org/wiki/Shunting_yard_algorithm
    std::stack<Token> operatorStack;
    std::stack<ExprASTNode *> exprStack;

    while (tokenizer.peek() != TokenType::Semicolon) {
        Token token = tokenizer.next();
        switch (token.type) {
        case TokenType::Number:
            exprStack.emplace(new NumericExprASTNode(token));
            break;
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::Multiply:
        case TokenType::Divide: {
            Token otherToken;
            while ((!operatorStack.empty() &&
                    (otherToken = operatorStack.top()).type !=
                        TokenType::OpenBracket) &&
                   precedence(otherToken.type) >= precedence(token.type)) {
                operatorStack.pop();

                std::unique_ptr<ExprASTNode> operandExpr2(exprStack.top());
                exprStack.pop();
                std::unique_ptr<ExprASTNode> operandExpr1(exprStack.top());
                exprStack.pop();
                exprStack.emplace(
                    new BinaryOpExprASTNode(otherToken, std::move(operandExpr1),
                                            std::move(operandExpr2)));
            }
            operatorStack.emplace(token);
            break;
        }
        case TokenType::OpenBracket:
            operatorStack.emplace(token);
            break;
        case TokenType::CloseBracket:
            while (operatorStack.top().type != TokenType::OpenBracket) {
                assert(!operatorStack.empty());

                Token operatorToken = operatorStack.top();
                operatorStack.pop();

                std::unique_ptr<ExprASTNode> operandExpr2(exprStack.top());
                exprStack.pop();
                std::unique_ptr<ExprASTNode> operandExpr1(exprStack.top());
                exprStack.pop();
                exprStack.emplace(new BinaryOpExprASTNode(
                    operatorToken, std::move(operandExpr1),
                    std::move(operandExpr2)));
            }
            assert(operatorStack.top().type == TokenType::OpenBracket);
            operatorStack.pop();
            break;
        default:
            std::cerr << "Unexcpected token" << std::endl;
            return nullptr;
        }
    }

    while (!operatorStack.empty()) {
        assert(operatorStack.top().type != TokenType::OpenBracket);

        Token operatorToken = operatorStack.top();
        operatorStack.pop();

        std::unique_ptr<ExprASTNode> operandExpr2(exprStack.top());
        exprStack.pop();
        std::unique_ptr<ExprASTNode> operandExpr1(exprStack.top());
        exprStack.pop();
        exprStack.emplace(new BinaryOpExprASTNode(
            operatorToken, std::move(operandExpr1), std::move(operandExpr2)));
    }

    std::unique_ptr<ExprASTNode> result(exprStack.top());
    return result;
}

// Parse any kind of statement
static std::unique_ptr<StmtASTNode> parseStmt(TokenReader &tokenizer) {
    if (!tokenizer.expectNext(TokenType::Return))
        return nullptr;
    auto expr = parseExpr(tokenizer);
    if (expr == nullptr)
        return nullptr;
    if (!tokenizer.expectNext(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<ReturnStmtASTNode>(expr);
}

// Specifically parse a compound statement. Function bodies cannot be any other
// kind of statement.
static std::unique_ptr<CompoundStmtASTNode>
parseCompoundStmt(TokenReader &tokenizer) {
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

static std::unique_ptr<FuncASTNode> parseFunction(TokenReader &tokenizer) {
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

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer) {
    return parseFunction(tokenizer);
}
