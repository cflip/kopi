#pragma once

#include "ast.hpp"
#include "token.hpp"

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer);
