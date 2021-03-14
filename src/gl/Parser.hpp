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

struct UniformBlock
{
    std::string type;
    std::string name;
    std::vector<TypePair> members;
};

class Parser
{
private:
    void Parse(
        const ShaderParseType parse_type,
        const std::string program);

public:
    Parser(
        const ShaderParseType parse_type,
        const std::string program);

    std::vector<TypePair> attributes;
    std::vector<TypePair> uniforms;
    std::vector<UniformBlock> uniform_blocks;
};
