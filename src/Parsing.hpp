#pragma once

#include <string>
#include <vector>

struct Token
{
    std::string type;
    std::string value;
    uint32_t from;
    uint32_t to;
};

// Adapted from Douglas Crockford's JS tokenizer
static std::vector<Token> tokenize(std::string str);
