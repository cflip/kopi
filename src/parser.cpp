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
    std::stack<ExprASTNode *> exprStack;
    std::stack<Token> unaryOpStack;
    std::stack<Token> binaryOpStack;
    bool expectingOperand = true;
    int bracketDepth = 0;

    auto placeUnaryOp = [&exprStack, &unaryOpStack]() {
        Token unaryOpToken = unaryOpStack.top();
        unaryOpStack.pop();
        std::unique_ptr<ExprASTNode> operand(exprStack.top());
        exprStack.pop();
        exprStack.emplace(new UnaryOpExprASTNode(unaryOpToken, std::move(operand)));
    };

    auto placeBinaryOp = [&exprStack, &binaryOpStack]() {
        assert(!binaryOpStack.empty());

        Token operatorToken = binaryOpStack.top();
        binaryOpStack.pop();

        std::unique_ptr<ExprASTNode> operandExpr2(exprStack.top());
        exprStack.pop();
        std::unique_ptr<ExprASTNode> operandExpr1(exprStack.top());
        exprStack.pop();
        exprStack.emplace(new BinaryOpExprASTNode(operatorToken, std::move(operandExpr1), std::move(operandExpr2)));
    };

    while (tokenizer.peek() != TokenType::Semicolon && tokenizer.peek() != TokenType::Comma
           && !(bracketDepth == 0 && tokenizer.peek() == TokenType::CloseBracket)) {
        Token token = tokenizer.next();
        switch (token.type) {
        case TokenType::Number:
            exprStack.emplace(new NumericExprASTNode(token));
            if (!unaryOpStack.empty() && unaryOpStack.top().type != TokenType::OpenBracket) {
                placeUnaryOp();
            }
            expectingOperand = false;
            break;
        case TokenType::Identifier:
            if (tokenizer.peek() == TokenType::OpenBracket) {
                std::vector<std::unique_ptr<ExprASTNode>> args;
                tokenizer.next();
                do {
                    if (tokenizer.peek() == TokenType::CloseBracket) {
                        break;
                    }
                    auto expr = parseExpr(tokenizer);
                    args.emplace_back(std::move(expr));

                    if (tokenizer.peek() == TokenType::CloseBracket) {
                        break;
                    }
                    if (!tokenizer.expectNext(TokenType::Comma))
                        return nullptr;
                } while (1);
                if (!tokenizer.expectNext(TokenType::CloseBracket))
                    return nullptr;
                exprStack.emplace(new FuncCallExprASTNode(token, std::move(args)));
            } else {
                exprStack.emplace(new IdentifierExprASTNode(token));
            }
            if (!unaryOpStack.empty() && unaryOpStack.top().type != TokenType::OpenBracket) {
                placeUnaryOp();
            }
            expectingOperand = false;
            break;
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::Multiply:
        case TokenType::Divide:
            if (expectingOperand) {
                unaryOpStack.push(token);
            } else {
                Token otherToken;
                while ((!binaryOpStack.empty() && (otherToken = binaryOpStack.top()).type != TokenType::OpenBracket)
                       && precedence(otherToken.type) >= precedence(token.type)) {
                    placeBinaryOp();
                }
                binaryOpStack.emplace(token);
                expectingOperand = true;
            }
            break;
        case TokenType::OpenBracket:
            if (!unaryOpStack.empty()) {
                unaryOpStack.emplace(token);
            }
            binaryOpStack.emplace(token);
            bracketDepth++;
            break;
        case TokenType::CloseBracket:
            while (binaryOpStack.top().type != TokenType::OpenBracket) {
                placeBinaryOp();
            }
            assert(binaryOpStack.top().type == TokenType::OpenBracket);
            binaryOpStack.pop();

            if (!unaryOpStack.empty() && unaryOpStack.top().type == TokenType::OpenBracket) {
                assert(!exprStack.empty());
                unaryOpStack.pop();
                placeUnaryOp();
            }

            bracketDepth--;
            break;
        default:
            std::cerr << "Unexcpected token" << std::endl;
            return nullptr;
        }
    }

    while (!binaryOpStack.empty()) {
        assert(binaryOpStack.top().type != TokenType::OpenBracket);
        placeBinaryOp();
    }

    std::unique_ptr<ExprASTNode> result(exprStack.top());
    return result;
}

// Parse any kind of statement
static std::unique_ptr<StmtASTNode> parseStmt(TokenReader &tokenizer) {
    Token token = tokenizer.next();
    if (token.type == TokenType::Return) {
        auto expr = parseExpr(tokenizer);
        if (expr == nullptr)
            return nullptr;
        if (!tokenizer.expectNext(TokenType::Semicolon))
            return nullptr;
        return std::make_unique<ReturnStmtASTNode>(expr);
    } else if (token.type == TokenType::Int) {
        Token ident;
        if (!tokenizer.expectNext(TokenType::Identifier, &ident)) {
            return nullptr;
        }

        std::unique_ptr<ExprASTNode> initExpr;
        if (tokenizer.peek() == TokenType::Assign) {
            tokenizer.next();
            initExpr = parseExpr(tokenizer);
            if (initExpr == nullptr)
                return nullptr;
        }

        if (!tokenizer.expectNext(TokenType::Semicolon))
            return nullptr;
        return std::make_unique<VariableDeclStmtASTNode>(ident, std::move(initExpr));
    } else if (token.type == TokenType::Identifier) {
        std::unique_ptr<ExprASTNode> expr;

        if (!tokenizer.expectNext(TokenType::Assign))
            return nullptr;

        expr = parseExpr(tokenizer);
        if (expr == nullptr)
            return nullptr;

        if (!tokenizer.expectNext(TokenType::Semicolon))
            return nullptr;

        return std::make_unique<AssignmentStmtASTNode>(token, std::move(expr));
    } else {
        std::cerr << "Unrecognized statement type" << std::endl;
        return nullptr;
    }
}

// Specifically parse a compound statement. Function bodies cannot be any other
// kind of statement.
static std::unique_ptr<CompoundStmtASTNode> parseCompoundStmt(TokenReader &tokenizer) {
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

    std::vector<Token> params;
    while (tokenizer.peek() != TokenType::CloseBracket) {
        if (!tokenizer.expectNext(TokenType::Int))
            return nullptr;

        Token paramIdent;
        if (!tokenizer.expectNext(TokenType::Identifier, &paramIdent))
            return nullptr;
        params.push_back(paramIdent);

        if (tokenizer.peek() != TokenType::CloseBracket) {
            if (!tokenizer.expectNext(TokenType::Comma))
                return nullptr;
        }
    }

    if (!tokenizer.expectNext(TokenType::CloseBracket))
        return nullptr;

    auto stmt = parseCompoundStmt(tokenizer);
    if (stmt == nullptr)
        return nullptr;

    auto func = std::make_unique<FuncASTNode>(ident, params, stmt);
    return func;
}

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer) {
    std::vector<std::unique_ptr<FuncASTNode>> functions;
    while (tokenizer.peek() != TokenType::EoF) {
        functions.emplace_back(parseFunction(tokenizer));
    }
    return std::make_unique<SourceFileASTNode>(std::move(functions));
}
