#pragma once

#include <memory>

#include "ast.hpp"
#include "token.hpp"

std::unique_ptr<ASTNode> parse(TokenReader &tokenizer);
