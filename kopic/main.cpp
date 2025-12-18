#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "token.hpp"

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> mainModule;
static std::unique_ptr<llvm::IRBuilder<>> builder;

enum class Visibility { Private, Protected, Public };

class ASTNode {
  public:
    virtual ~ASTNode() = default;

    virtual llvm::Value *emit() const = 0;

    virtual void dbgprint(int indent = 0) const {
        for (int i = 0; i < indent; i++)
            std::cout << '\t';
    }
};

class ExprASTNode : public ASTNode {
  public:
    explicit ExprASTNode(Token number) : number(number) {}

    llvm::Value *emit() const override {
        long value = std::stoi(number.contents);
        return llvm::ConstantInt::get(*context, llvm::APInt(32, value, true));
    }

    void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<expr> -> " << number.contents << std::endl;
    }

  private:
    Token number;
};

class StmtASTNode : public ASTNode {};

class ReturnStmtASTNode : public StmtASTNode {
  public:
    explicit ReturnStmtASTNode(std::unique_ptr<ExprASTNode> &expr)
        : expr(std::move(expr)) {}

    llvm::Value *emit() const override {
        return builder->CreateRet(expr->emit());
    }

    void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<return>" << '\n';
        expr->dbgprint(indent + 1);
    }

  private:
    std::unique_ptr<ExprASTNode> expr;
};

class CompoundStmtASTNode : public StmtASTNode {
  public:
    explicit CompoundStmtASTNode(std::vector<std::unique_ptr<StmtASTNode>> list)
        : stmts(std::move(list)) {}

    llvm::Value *emit() const override {
        for (auto &&stmt : stmts) {
            stmt->emit();
        }
        return nullptr;
    }

    void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<block>" << '\n';
        for (auto &&stmt : stmts) {
            stmt->dbgprint(indent + 1);
        }
    }

  private:
    std::vector<std::unique_ptr<StmtASTNode>> stmts;
};

class FuncASTNode : public ASTNode {
  public:
    FuncASTNode(Token ident, std::unique_ptr<CompoundStmtASTNode> &body)
        : vis(Visibility::Public), identifier(ident), body(std::move(body)) {}

    llvm::Value *emit() const override {
        llvm::FunctionType *funcType =
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
        llvm::Function *func =
            llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                   identifier.contents, *mainModule);

        llvm::BasicBlock *block =
            llvm::BasicBlock::Create(*context, "entry", func);
        builder->SetInsertPoint(block);
        body->emit();

        llvm::verifyFunction(*func);

        return func;
    }

    void dbgprint(int indent) const override {
        ASTNode::dbgprint(indent);
        std::cout << "<func> " << identifier.contents << '\n';
        body->dbgprint(indent + 1);
    }

  private:
    Visibility vis;
    Token identifier;
    std::unique_ptr<CompoundStmtASTNode> body;
};

std::unique_ptr<ExprASTNode> parseExpr(TokenReader &tokenizer) {
    Token returnValue;
    if (!tokenizer.expectNext(TokenType::Number, &returnValue))
        return nullptr;
    return std::make_unique<ExprASTNode>(returnValue);
}

// Parse any kind of statement
std::unique_ptr<StmtASTNode> parseStmt(TokenReader &tokenizer) {
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
std::unique_ptr<CompoundStmtASTNode> parseCompoundStmt(TokenReader &tokenizer) {
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

std::unique_ptr<FuncASTNode> parseFunction(TokenReader &tokenizer) {
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "no input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path sourcePath(argv[1]);

    context = std::make_unique<llvm::LLVMContext>();
    mainModule =
        std::make_unique<llvm::Module>(sourcePath.stem().string(), *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    std::ifstream infile(sourcePath);
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
        ast->emit();
        mainModule->print(llvm::outs(), nullptr);
    }

    return EXIT_SUCCESS;
}
