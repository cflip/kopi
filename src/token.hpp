#pragma once

#include <string>

enum class TokenType {
    EoF,
    Invalid,

    // Symbols
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    Semicolon,
    Comma,

    // Operators
    Plus,
    Minus,
    Multiply,
    Divide,
    Assign,

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
    explicit TokenReader(std::istream &inf) : infile(inf) {}

    Token next();
    // Gets the next token, and expects it to be a certain type.
    [[nodiscard]] bool expectNext(TokenType type, Token *result = nullptr);

    // Previews the type of the next token without advancing the parser
    [[nodiscard]] TokenType peek();

  private:
    std::istream &infile;
};
