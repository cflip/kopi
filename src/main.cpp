#include "parser.hpp"

#include <llvm/Support/CommandLine.h>

#include <filesystem>
#include <fstream>
#include <iostream>

llvm::cl::opt<std::string> inputName(llvm::cl::Positional,
                                     llvm::cl::desc("<input file>"),
                                     llvm::cl::Required);
llvm::cl::opt<std::string> outputName("o",
                                      llvm::cl::desc("Specify output filename"),
                                      llvm::cl::value_desc("filename"));

int main(int argc, char *argv[]) {
    llvm::cl::ParseCommandLineOptions(argc, argv);

    std::filesystem::path sourcePath(inputName.c_str());
    std::filesystem::path outputPath;

    if (outputName.empty()) {
        outputPath = sourcePath.stem().concat(".o");
    } else {
        outputPath = outputName.c_str();
    }

    if (!codegenInit(sourcePath.stem().string())) {
        return EXIT_FAILURE;
    }

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
        codegenPrintIR(outputPath);
    }

    return EXIT_SUCCESS;
}
