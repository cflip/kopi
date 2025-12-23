#include "token.hpp"

#include <iostream>

Token TokenReader::next() {
    int tokenChar;
    std::string word;

    tokenChar = infile.get();

    if (tokenChar == EOF)
        return {TokenType::EoF, "_done_"};

    // Ignore whitespace characters
    while (isspace(tokenChar))
        tokenChar = infile.get();

    // Read keywords and identifiers
    if (isalpha(tokenChar)) {
        while (isalnum(tokenChar)) {
            word += tokenChar;
            tokenChar = infile.get();
        }

        infile.unget();
        if (word == "public") {
            return {TokenType::Public, word};
        }
        if (word == "int") {
            return {TokenType::Int, word};
        }
        if (word == "return") {
            return {TokenType::Return, word};
        }

        return {TokenType::Identifier, word};
    }

    if (isdigit(tokenChar)) {
        while (isdigit(tokenChar)) {
            word += tokenChar;
            tokenChar = infile.get();
        }
        infile.unget();
        return {TokenType::Number, word};
    }

    switch (tokenChar) {
    case '(':
        return {TokenType::OpenBracket, "("};
    case ')':
        return {TokenType::CloseBracket, ")"};
    case '{':
        return {TokenType::OpenBrace, "{"};
    case '}':
        return {TokenType::CloseBrace, "}"};
    case ';':
        return {TokenType::Semicolon, ";"};
    case '+':
        return {TokenType::Plus, "+"};
    case '-':
        return {TokenType::Minus, "-"};
    case '*':
        return {TokenType::Multiply, "*"};
    case '/':
        return {TokenType::Divide, "/"};
    case '=':
        return {TokenType::Assign, "="};
    default:
        std::cerr << "Unrecognized token" << std::endl;
        return {TokenType::Invalid, ""};
    }
}

bool TokenReader::expectNext(TokenType expectedType, Token *result) {
    static const char *tokenNames[] = {
        // clang-format off
        "end of file",
        "invalid token",
        "(",
        ")",
        "{",
        "}",
        ";",
        "+",
        "-",
        "*",
        "/",
        "=",
        "public",
        "int",
        "return",
        "identifier",
        "number"
        // clang-format on
    };
    static_assert(sizeof(tokenNames) / sizeof(*tokenNames) ==
                  static_cast<size_t>(TokenType::_NumTypes));

    Token tok = next();
    if (tok.type != expectedType) {
        std::cerr << "Expected token '"
                  << tokenNames[static_cast<int>(expectedType)] << "', got '"
                  << tokenNames[static_cast<int>(tok.type)] << '\''
                  << std::endl;
        return false;
    }
    if (result != nullptr) {
        *result = tok;
    }
    return true;
}

TokenType TokenReader::peek() {
    std::streampos pos = infile.tellg();
    Token t = next();
    infile.seekg(pos);
    return t.type;
}
