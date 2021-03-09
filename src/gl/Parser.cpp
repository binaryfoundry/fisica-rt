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
    Token token;
    std::string name = "";
    std::string type = "";
    std::string shader_block_name = "";
    std::vector<TypePair> uniform_list;
};

Parser::Parser(
    const ShaderParseType parse_type,
    const std::string program) :
    program(program)
{
    Parse(parse_type);
}

void Parser::Parse(
    const ShaderParseType parse_type)
{
    const std::vector<Token> tokens = tokenize(program);
    const size_t num_tokens = tokens.size();

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
        if (state.token.type != "name")
            return;
        if (keywords.find(state.token.value) != keywords.end())
            return;
        state.name = state.token.value;
    };

    const auto Step = [&](uint32_t count = 1) {
        position += count;
        num_remaining -= count;
        if (position < num_tokens)
        {
            state.token = tokens[position];
        }
        else
        {
            num_remaining = 0;
            state.token = {};
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
                if (state.token.value == "COMPILING_VS")
                {
                    mode = ParseMode::NONE;
                }
            }
            else if (parse_type == ShaderParseType::FRAGMENT)
            {
                if (state.token.value == "COMPILING_FS")
                {
                    mode = ParseMode::NONE;
                }
            }
            else if (parse_type == ShaderParseType::COMPUTE)
            {
                if (state.token.value == "COMPILING_CS")
                {
                    mode = ParseMode::NONE;
                }
            }
            break;

        case ParseMode::NONE:
            if (state.token.value == "COMPILING_VS" ||
                state.token.value == "COMPILING_FS" ||
                state.token.value == "COMPILING_CS")
            {
                return;
            }
            if (state.token.value == "in")
            {
                PushParseMode(ParseMode::ATTRIBUTE);
            }
            else if (state.token.value == "layout")
            {
                PushParseMode(ParseMode::LAYOUT);
            }
            else if (state.token.value == "uniform")
            {
                PushParseMode(ParseMode::UNIFORM);
            }
            break;

        case ParseMode::LAYOUT:
            if (state.token.value == "uniform")
            {
                PushParseMode(ParseMode::UNIFORM_BLOCK);
            }
            if (state.token.value == "buffer")
            {
                PushParseMode(ParseMode::BUFFER);
            }
            else if (state.token.value == "location")
            {
                PushParseMode(ParseMode::NONE);
            }
            else if (state.token.value == ";")
            {
                ClearState();
            }
            break;

        case ParseMode::ATTRIBUTE:
            if (state.token.value == ";")
            {
                // TODO if current_attribute_type == NONE not found error
                attributes.push_back({
                    state.type,
                    state.name
                });
                ClearState();
            }
            else if (state.type == "")
            {
                state.type = state.token.value;
            }
            else
            {
                CheckName();
            }
            break;

        case ParseMode::UNIFORM:
            if (state.token.value == ";")
            {
                uniforms.push_back({
                    state.type,
                    state.name
                });

                ClearState();
            }
            else if (state.type == "")
            {
                state.type = state.token.value;
            }
            else
            {
                state.name = state.token.value;
            }
            break;

        case ParseMode::UNIFORM_BLOCK:
            if (state.token.value == "{")
            {
                state.shader_block_name = state.name;
                ClearUniformBlockState();
                PushParseMode(ParseMode::UNIFORM_BLOCK_MEMBERS);
            }
            else
            {
                CheckName();
            }
            break;

        case ParseMode::UNIFORM_BLOCK_MEMBERS:
            if (state.token.value == "}")
            {
                uniform_blocks[state.shader_block_name] =
                    state.uniform_list;

                ClearState();
            }
            else if (state.token.value == ";")
            {
                state.uniform_list.push_back({
                    state.type,
                    state.name
                });

                // TODO if current_uniform_type == NONE not found error
                ClearUniformBlockState();
            }
            else if (state.type == "")
            {
                state.type = state.token.value;
            }
            else
            {
                state.name = state.token.value;
            }
            break;

        case ParseMode::BUFFER:
            if (state.token.value == ";")
            {
                //uniform_block_list.push_back({
                //    UniformBlockType::BUFFER,
                //    current_shader_block_name,
                //    current_uniform_list
                //});

                assert(false);

                ClearState();
            }
            if (state.token.value == "{")
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
