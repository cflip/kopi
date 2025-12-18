#include "token.hpp"

Token TokenReader::next()
{
    int tokenChar;
    std::string word;

    tokenChar = infile.get();

    if (tokenChar == EOF)
        return { TokenType::EoF, "_done_" };

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
            return { TokenType::Public, word };
        }
        if (word == "int") {
            return { TokenType::Int, word };
        }
        if (word == "return") {
            return { TokenType::Return, word };
        }

        return { TokenType::Identifier, word };
    }

    if (isdigit(tokenChar)) {
        while (isdigit(tokenChar)) {
            word += tokenChar;
            tokenChar = infile.get();
        }
        infile.unget();
        return { TokenType::Number, word };
    }

    switch (tokenChar) {
    case '(': return { TokenType::OpenBracket, "(" };
    case ')': return { TokenType::CloseBracket, ")" };
    case '{': return { TokenType::OpenBrace, "{" };
    case '}': return { TokenType::CloseBrace, "}" };
    case ';': return { TokenType::Semicolon, ";" };
    default:
        throw std::runtime_error("Unrecognized token");
    }
}

Token TokenReader::expectNext(TokenType expectedType)
{
    static const char *tokenNames[static_cast<int>(TokenType::_NumTypes)] = {
        "end of file",
        "(",
        ")",
        "{",
        "}",
        ";",
        "public",
        "int",
        "return",
        "identifier",
        "number"
    };
    Token result = next();
    if (result.type != expectedType) {
        std::string errmsg = "Expected token '";
        errmsg.append(tokenNames[static_cast<int>(expectedType)]);
        errmsg.append("', got '");
        errmsg.append(tokenNames[static_cast<int>(result.type)]);
        errmsg += '\'';
        throw std::runtime_error(errmsg);
    }
    return result;
}
