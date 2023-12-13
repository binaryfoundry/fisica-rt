#include "Parsing.hpp"
#include <stdexcept>

std::vector<Token> tokenize(std::string str)
{
    const std::string prefix = "<>+-&";
    const std::string suffix = "=>&:";

    const size_t length = str.size();

    // The current character.
    char character = ' ';

    // The index of the start of the token.
    uint32_t from = 0;

    // The index of the current character.
    uint32_t index = 0;

    // The number value.
    double n_val;

    // The quote character.
    char q_val;

    // The string value.
    std::string s_val;

    // An array to hold the results.
    std::vector<Token> result;

    if (str == "")
    {
        return result;
    }

    str = str + " ";

    // Loop through this text, one character at a time.

    character = str.at(index);
    while (character)
    {
        from = index;

        // Ignore whitespace.

        if (character <= ' ')
        {
            if (index == length)
            {
                return result;
            }
            else
            {
                index += 1;
                character = str.at(index);
            }
            // name.
        }
        else if (
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z'))
        {
            s_val = character;
            index += 1;
            for (;;)
            {
                character = str.at(index);
                if ((character >= 'a' && character <= 'z') ||
                    (character >= 'A' && character <= 'Z') ||
                    (character >= '0' && character <= '9') ||
                    (character == '_'))
                {
                    s_val += character;
                    index += 1;
                }
                else
                {
                    break;
                }
            }

            result.push_back({
                "name",
                s_val,
                from,
                index
            });

            // number.

            // A number cannot start with a decimal point.
            // It must start with a digit, possibly '0'.
        }
        else if (character >= '0' && character <= '9')
        {
            s_val = character;
            index += 1;

            // Look for more digits.

            for (;;)
            {
                character = str.at(index);
                if (character < '0' || character > '9')
                {
                    break;
                }
                index += 1;
                s_val += character;
            }

            // Look for a decimal fraction part.

            if (character == '.')
            {
                index += 1;
                s_val += character;
                for (;;)
                {
                    character = str.at(index);
                    if (character < '0' || character > '9')
                    {
                        break;
                    }
                    index += 1;
                    s_val += character;
                }
            }

            // Look for an exponent part.

            if (character == 'e' || character == 'E')
            {
                index += 1;
                s_val += character;
                character = str.at(index);
                if (character == '-' || character == '+')
                {
                    index += 1;
                    s_val += character;
                    character = str.at(index);
                }
                if (character < '0' || character > '9')
                {
                    throw std::runtime_error(
                        "Bad exponent");
                }
                do
                {
                    index += 1;
                    s_val += character;
                    character = str.at(index);
                }
                while (character >= '0' && character <= '9');
            }

            // Make sure the next character is not a letter.

            if (character >= 'a' && character <= 'z')
            {
                if (character != 'f')
                {
                    s_val += character;
                    index += 1;
                    throw std::runtime_error(
                        "Bad number");
                }
            }

            // Convert the string value to a number.
            // If it is finite, then it is a good token.

            try
            {
                n_val = stof(s_val);
            }
            catch (const std::invalid_argument& e)
            {
                throw std::runtime_error(e.what());
            }
            catch (const std::out_of_range& e)
            {
                throw std::runtime_error(e.what());
            }

            if (isfinite(n_val))
            {
                result.push_back({
                    "number",
                    s_val,
                    from,
                    index
                });
            }
            else
            {
                throw std::runtime_error(
                    "Bad Number.");
            }

            // string

        }
        else if (character == '\'' || character == '"')
        {
            s_val = ' ';
            q_val = character;
            index += 1;
            for (;;)
            {
                character = str.at(index);
                if (character < ' ')
                {
                    if (character == '\n' || character == '\r' || !character)
                    {
                        throw std::runtime_error(
                            "Unterminated string.");
                    }
                    else
                    {
                        throw std::runtime_error(
                            "Control character in string.");
                    }
                }

                // Look for the closing quote.
                if (character == q_val)
                {
                    break;
                }

                // Look for escapement.

                if (character == '\\')
                {
                    index += 1;
                    if (index >= length)
                    {
                        throw std::runtime_error(
                            "Unterminated string.");
                    }
                    character = str.at(index);
                    switch (character)
                    {
                    case 'b':
                        character = '\b';
                        break;
                    case 'f':
                        character = '\f';
                        break;
                    case 'n':
                        character = '\n';
                        break;
                    case 'r':
                        character = '\r';
                        break;
                    case 't':
                        character = '\t';
                        break;
                    case 'u':
                        if (index >= length)
                        {
                            throw std::runtime_error(
                                "Unterminated string.");
                        }

                        uint16_t c = 0;

                        try
                        {
                            c = std::stoi(str.substr(index + 1, 4));
                        }
                        catch (const std::invalid_argument& e)
                        {
                            throw std::runtime_error(e.what());
                        }
                        catch (const std::out_of_range& e)
                        {
                            throw std::runtime_error(e.what());
                        }

                        if (character < 0)
                        {
                            throw std::runtime_error(
                                "Unterminated string.");
                        }

                        //character = String.fromCharCode(c);
                        throw std::runtime_error(
                            "Unicode unsupported.");
                        index += 4;
                        break;
                    }
                }
                s_val += character;
                index += 1;
            }
            index += 1;
            result.push_back({
                "string",
                s_val,
                from,
                index
            });
            character = str.at(index);
            // comment.
        }
        else if (character == '/' && str.at(index + 1) == '/')
        {
            index += 1;
            for (;;)
            {
                character = str.at(index);
                if (character == '\n' || character == '\r' || !character)
                {
                    break;
                }
                index += 1;
            }
            // combining
        }
        else if (prefix.find(character) != std::string::npos)
        {
            s_val = character;
            index += 1;
            while (true)
            {
                character = str.at(index);
                if (index >= length || suffix.find(character) ==
                    std::string::npos)
                {
                    break;
                }
                str += character;
                index += 1;
            }
            result.push_back({
                "operator",
                s_val,
                from,
                index
            });
            // single-character operator
        }
        else
        {
            index += 1;
            result.push_back({
                "operator",
                std::string(&character, 1),
                from,
                index
            });
            character = str.at(index);
        }
    }
    return result;
}
