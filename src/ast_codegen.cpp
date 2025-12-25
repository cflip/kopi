#include "ast.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <iostream>

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> mainModule;
static std::unique_ptr<llvm::IRBuilder<>> builder;

static llvm::TargetMachine *targetMachine;

static std::unordered_map<std::string, llvm::FunctionType *> funcTypes;
static std::unordered_map<std::string, llvm::Value *> funcParams;
static std::unordered_map<std::string, llvm::AllocaInst *> localVariables;

llvm::Value *NumericExprASTNode::emit() const {
    int64_t value = std::stoi(number.contents);
    return llvm::ConstantInt::get(*context, llvm::APInt(32, value, true));
}

llvm::Value *IdentifierExprASTNode::emit() const {
    if (funcParams.find(identifier.contents) != funcParams.end()) {
        return funcParams[identifier.contents];
    } else if (localVariables.find(identifier.contents) != localVariables.end()) {
        return builder->CreateLoad(llvm::Type::getInt32Ty(*context), localVariables[identifier.contents]);
    } else {
        std::cerr << "Unknown identifier " << identifier.contents << std::endl;
        return nullptr;
    }
}

llvm::Value *FuncCallExprASTNode::emit() const {
    llvm::Function *callee = mainModule->getFunction(function.contents);
    std::vector<llvm::Value *> argValues;
    for (const auto &arg : arguments) {
        argValues.push_back(arg->emit());
    }
    return builder->CreateCall(callee, argValues);
}

llvm::Value *UnaryOpExprASTNode::emit() const {
    return builder->CreateNeg(operand->emit());
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

llvm::Value *VariableDeclStmtASTNode::emit() const {
    auto alloc = builder->CreateAlloca(llvm::Type::getInt32Ty(*context), nullptr, identifier.contents);
    localVariables[identifier.contents] = alloc;
    if (initExpr) {
        builder->CreateStore(initExpr->emit(), alloc);
    } else {
        builder->CreateStore(llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), alloc);
    }
    return alloc;
}

llvm::Value *CompoundStmtASTNode::emit() const {
    for (auto &&stmt : stmts) {
        stmt->emit();
    }
    return nullptr;
}

llvm::Value *FuncASTNode::emit() const {
    std::vector<llvm::Type *> arguments(params.size(), llvm::Type::getInt32Ty(*context));

    llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), arguments, false);
    llvm::Function *func =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, identifier.contents, *mainModule);

    funcParams.clear();
    localVariables.clear();

    unsigned index = 0;
    for (auto &arg : func->args()) {
        funcParams[params[index].contents] = &arg;
        index++;
    }

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(block);
    body->emit();

    llvm::verifyFunction(*func);

    return func;
}

llvm::Value *SourceFileASTNode::emit() const {
    for (const auto &func : functions) {
        func->emit();
    }
    return nullptr;
}

bool codegenInit(const std::string &moduleName) {
    context = std::make_unique<llvm::LLVMContext>();
    mainModule = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    std::string targetTriple = llvm::sys::getDefaultTargetTriple();

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string errorMsg;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, errorMsg);
    if (target == nullptr) {
        std::cerr << "Unable to look up target triple " << targetTriple << ": " << errorMsg << std::endl;
        return false;
    }

    std::string targetCpu = "generic";
    std::string targetFeatures = "";
    llvm::TargetOptions targetOptions;
    targetMachine =
        target->createTargetMachine(targetTriple, targetCpu, targetFeatures, targetOptions, llvm::Reloc::Static);

    mainModule->setDataLayout(targetMachine->createDataLayout());
    mainModule->setTargetTriple(targetTriple);

    return true;
}

void codegenPrintIR() {
    mainModule->print(llvm::outs(), nullptr);
}

void codegenOutput(const std::string &filename) {
    std::error_code errCode;
    llvm::raw_fd_ostream outfile(filename, errCode, llvm::sys::fs::OF_None);

    llvm::legacy::PassManager passManager;

    if (targetMachine->addPassesToEmitFile(passManager, outfile, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "Could not add object file emit to pass manager" << std::endl;
        return;
    }

    passManager.run(*mainModule);
    outfile.flush();
}
