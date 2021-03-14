#include "Parser.hpp"

#include <set>
#include <assert.h>

enum class ParseMode
{
    INIT,
    NONE,
    LAYOUT,
    ATTRIBUTE,
    UNIFORM,
    UNIFORM_BLOCK,
    UNIFORM_BLOCK_MEMBERS,
    BUFFER,
    BUFFER_META,
};

const std::set<std::string> keywords = {
    "float",
    "vec2",
    "vec3",
    "vec4",
    "int",
    "dvec2",
    "dvec3",
    "dvec4",
    "mat2",
    "mat3",
    "mat4",
    "sampler2D",
    "sampler2DArray",
    "sampler3D",
    "samplerCube",
    "in",
    "out",
    "void",
    "layout",
    "std140",
    "std430",
    "uniform",
    "{",
    "}",
    "(",
    ")",
    ";",
};

struct State
{
    std::string name = "";
    std::string type = "";
    std::string shader_block_name = "";
    std::string shader_block_type = "";
    std::vector<TypePair> uniform_list;
};

Parser::Parser(
    const ShaderParseType parse_type,
    const std::string program)
{
    Parse(parse_type, program);
}

void Parser::Parse(
    const ShaderParseType parse_type,
    const std::string program)
{
    const std::vector<Token> tokens = tokenize(program);
    const size_t num_tokens = tokens.size();

    Token token;
    State state;
    ParseMode mode = ParseMode::INIT;
    std::vector<ParseMode> mode_stack;

    size_t position = 0;
    size_t num_remaining = num_tokens;

    const auto PushParseMode = [&](ParseMode new_mode) {
        mode_stack.push_back(mode);
        mode = new_mode;
    };

    const auto PopParseMode = [&]() {
        mode = mode_stack.back();
        mode_stack.pop_back();
    };

    const auto ClearState = [&](ParseMode new_state = ParseMode::NONE) {
        state.name = "";
        state.shader_block_name = "";
        state.type = "";
        state.uniform_list.clear();
        mode = new_state;
    };

    const auto ClearUniformBlockState = [&]() {
        state.name = "";
        state.type = "";
    };

    const auto CheckName = [&]() {
        state.name = "";
        if (token.type != "name")
            return;
        if (keywords.find(token.value) != keywords.end())
            return;
        state.name = token.value;
    };

    const auto Step = [&](uint32_t count = 1) {
        position += count;
        num_remaining -= count;
        if (position < num_tokens)
        {
            token = tokens[position];
        }
        else
        {
            num_remaining = 0;
            token = {};
        }
    };

    Step(0);

    while (num_remaining > 0)
    {
        switch (mode)
        {
        case ParseMode::INIT:
            if (parse_type == ShaderParseType::VERTEX)
            {
                if (token.value == "COMPILING_VS")
                {
                    mode = ParseMode::NONE;
                }
            }
            else if (parse_type == ShaderParseType::FRAGMENT)
            {
                if (token.value == "COMPILING_FS")
                {
                    mode = ParseMode::NONE;
                }
            }
            else if (parse_type == ShaderParseType::COMPUTE)
            {
                if (token.value == "COMPILING_CS")
                {
                    mode = ParseMode::NONE;
                }
            }
            break;

        case ParseMode::NONE:
            if (token.value == "COMPILING_VS" ||
                token.value == "COMPILING_FS" ||
                token.value == "COMPILING_CS")
            {
                return;
            }
            if (token.value == "in")
            {
                PushParseMode(ParseMode::ATTRIBUTE);
            }
            else if (token.value == "layout")
            {
                PushParseMode(ParseMode::LAYOUT);
            }
            else if (token.value == "uniform")
            {
                PushParseMode(ParseMode::UNIFORM);
            }
            break;

        case ParseMode::LAYOUT:
            if (token.value == "uniform")
            {
                PushParseMode(ParseMode::UNIFORM_BLOCK);
            }
            if (token.value == "buffer")
            {
                PushParseMode(ParseMode::BUFFER);
            }
            else if (token.value == "location")
            {
                PushParseMode(ParseMode::NONE);
            }
            if (token.value == "std140")
            {
                state.type = token.value;
            }
            else if (token.value == "std430")
            {
                state.type = token.value;
            }
            else if (token.value == ";")
            {
                ClearState();
            }
            break;

        case ParseMode::ATTRIBUTE:
            if (token.value == ";")
            {
                attributes.push_back({
                    state.type,
                    state.name
                });
                ClearState();
            }
            else if (state.type == "")
            {
                state.type = token.value;
            }
            else
            {
                CheckName();
            }
            break;

        case ParseMode::UNIFORM:
            if (token.value == ";")
            {
                if (state.type == "sampler2D")
                {
                    uniform_sampler2Ds.push_back({
                        state.type,
                        state.name
                    });
                }
                else if (state.type == "sampler2DArray")
                {
                    uniform_sampler2D_arrays.push_back({
                        state.type,
                        state.name
                    });
                }
                else if (state.type == "mat4")
                {
                    uniform_mat4s.push_back({
                        state.type,
                        state.name
                    });
                }
                else
                {
                    throw std::runtime_error(
                        "Unsupported uniform type.");
                }

                ClearState();
            }
            else if (state.type == "")
            {
                state.type = token.value;
            }
            else
            {
                state.name = token.value;
            }
            break;

        case ParseMode::UNIFORM_BLOCK:
            if (token.value == "{")
            {
                state.shader_block_name = state.name;
                state.shader_block_type = state.type;
                ClearUniformBlockState();
                PushParseMode(ParseMode::UNIFORM_BLOCK_MEMBERS);
            }
            else
            {
                CheckName();
            }
            break;

        case ParseMode::UNIFORM_BLOCK_MEMBERS:
            if (token.value == "}")
            {
                uniform_blocks.push_back({
                    state.shader_block_type,
                    state.shader_block_name,
                    state.uniform_list
                });

                ClearState();
            }
            else if (token.value == ";")
            {
                state.uniform_list.push_back({
                    state.type,
                    state.name
                });

                ClearUniformBlockState();
            }
            else if (state.type == "")
            {
                state.type = token.value;
            }
            else
            {
                state.name = token.value;
            }
            break;

        case ParseMode::BUFFER:
            if (token.value == ";")
            {
                //uniform_block_list.push_back({
                //    UniformBlockType::BUFFER,
                //    current_shader_block_name,
                //    current_uniform_list
                //});

                assert(false);

                ClearState();
            }
            if (token.value == "{")
            {
                state.shader_block_name = state.name;
                ClearUniformBlockState();
            }
            else
            {
                CheckName();
            }
            break;

        default:
            break;
        }
        Step();
    }
}
