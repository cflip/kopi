#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <iostream>

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> mainModule;
static std::unique_ptr<llvm::IRBuilder<>> builder;

llvm::Value *NumericExprASTNode::emit() const {
    int64_t value = std::stoi(number.contents);
    return llvm::ConstantInt::get(*context, llvm::APInt(32, value, true));
}

llvm::Value *BinaryOpExprASTNode::emit() const {
    switch (op.type) {
    case TokenType::Plus:
        return builder->CreateAdd(left->emit(), right->emit());
    case TokenType::Minus:
        return builder->CreateSub(left->emit(), right->emit());
    case TokenType::Multiply:
        return builder->CreateMul(left->emit(), right->emit());
    case TokenType::Divide:
        return builder->CreateSDiv(left->emit(), right->emit());
    default:
        std::cerr << "Invalid binary operator in expression";
        return nullptr;
    }
}

llvm::Value *ReturnStmtASTNode::emit() const {
    return builder->CreateRet(expr->emit());
}

llvm::Value *CompoundStmtASTNode::emit() const {
    for (auto &&stmt : stmts) {
        stmt->emit();
    }
    return nullptr;
}

llvm::Value *FuncASTNode::emit() const {
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
    llvm::Function *func =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                               identifier.contents, *mainModule);

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(block);
    body->emit();

    llvm::verifyFunction(*func);

    return func;
}

bool codegenInit(const std::string &moduleName) {
    context = std::make_unique<llvm::LLVMContext>();
    mainModule = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    return true;
}

void codegenPrintIR() { mainModule->print(llvm::outs(), nullptr); }
