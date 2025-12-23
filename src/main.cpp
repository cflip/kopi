#include "parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "no input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path sourcePath(argv[1]);

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
        codegenPrintIR();
    }

    return EXIT_SUCCESS;
}
