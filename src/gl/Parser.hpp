#pragma once

#include "../Parsing.hpp"

#include <map>
#include <tuple>
#include <string>
#include <vector>

enum class ShaderParseType
{
    VERTEX,
    FRAGMENT,
    COMPUTE
};

using TypePair = std::tuple<std::string, std::string>;

class Parser
{
private:
    std::string program;

    void Parse(
        const ShaderParseType parse_type);

public:
    Parser(
        const ShaderParseType parse_type,
        const std::string program);

    std::string Program() const
    {
        return program;
    }

    std::vector<TypePair> attributes;
    std::vector<TypePair> uniforms;
    std::map<std::string, std::vector<TypePair>> uniform_blocks;
};
