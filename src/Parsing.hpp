#pragma once

#include <string>
#include <vector>

struct Token
{
    std::string type = "";
    std::string value = "";
    uint32_t from = 0;
    uint32_t to = 0;
};

// Adapted from Douglas Crockford's JS tokenizer
std::vector<Token> tokenize(std::string str);
