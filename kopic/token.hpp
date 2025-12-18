#pragma once

#include <string>
#include <fstream>

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

    _NumTypes
};

struct Token {
    TokenType type;
    std::string contents;
};

class TokenReader {
public:
    TokenReader(std::ifstream &inf) : infile(inf) {}

    Token next();
    // Gets the next token, and expects it to be a certain type.
    Token expectNext(TokenType type);

    // Previews the type of the next token without advancing the parser
    TokenType peek();
    
private:
    std::ifstream &infile;
};
