#include "parser.hpp"

#include <llvm/Support/CommandLine.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace llvm;

cl::OptionCategory category("kopic options");

cl::opt<std::string> inputName(cl::Positional, cl::desc("<input file>"),
                               cl::Required, cl::cat(category));
cl::opt<std::string> outputName("o", cl::desc("Specify output filename"),
                                cl::value_desc("filename"), cl::cat(category));

cl::opt<bool> dumpAst("dump-ast", cl::desc("Print AST to stdout"),
                      cl::cat(category));
cl::opt<bool> dumpIr("dump-ir", cl::desc("Print LLVM IR to stdout"),
                     cl::cat(category));

int main(int argc, char *argv[]) {
    cl::HideUnrelatedOptions(category);
    cl::ParseCommandLineOptions(argc, argv);

    std::filesystem::path sourcePath(inputName.c_str());
    std::filesystem::path outputPath;

    if (outputName.empty()) {
        outputPath = sourcePath.stem().concat(".o");
    } else {
        outputPath = outputName.c_str();
    }

    if (!codegenInit(inputName)) {
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
        if (dumpAst) {
            ast->dbgprint();
        }
        ast->emit();

        if (dumpIr) {
            codegenPrintIR();
        }
        codegenOutput(outputPath);
    }

    return EXIT_SUCCESS;
}
