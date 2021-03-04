#include "File.hpp"

#include <iostream>
#include <assert.h>

#if defined(EMSCRIPTEN)

File::File(std::string path, std::string mode)
{
    auto h = fopen(path.c_str(), mode.c_str());

    if (h == nullptr)
    {
        std::cout << "Failed to open file: " << path << std::endl;
        throw std::runtime_error("Failed to open file.");
    }

    fseek(h, 0, SEEK_END);
    length = static_cast<size_t>(ftell(h));
    rewind(h);

    handle = h;
}

File::~File()
{
    fclose(std::any_cast<FILE*>(handle));
}

size_t File::Read(void* buffer, size_t size, size_t count)
{
    return fread(buffer, size, count, std::any_cast<FILE*>(handle));
}

size_t File::Length()
{
    return length;
}

#else

#include "sdl/SDL.hpp"

File::File(std::string path, std::string mode)
{
    auto h = SDL_RWFromFile(
        path.c_str(),
        mode.c_str());

    if (h == nullptr)
    {
        std::cout << "Failed to open file: " << path << std::endl;
        throw std::runtime_error("Failed to open file.");
    }

    length = static_cast<size_t>(SDL_RWseek(
        h,
        0,
        SEEK_END));

    SDL_RWseek(
        h,
        0,
        SEEK_SET);

    handle = h;
}

File::~File()
{
    SDL_RWclose(
        std::any_cast<SDL_RWops*>(handle));
}

size_t File::Read(void* buffer, size_t size, size_t count)
{
    return SDL_RWread(
        std::any_cast<SDL_RWops*>(handle), buffer, size, count);
}

size_t File::Length()
{
    return length;
}

#endif

std::string File::ReadString()
{
    const auto count = Length();
    char* buffer = new char[count + 1];

    Read(buffer, sizeof(char), count);
    buffer[count] = 0x00;

    std::string str = std::string(buffer);

    delete[] buffer;

    return str;
}

// string prefixed with length (uint16_t)
std::string File::ReadStringPrefixed()
{
    uint16_t count;
    Read(&count, sizeof(uint16_t), 1);

    char* buffer = new char[count + 1];

    Read(buffer, sizeof(char), count);
    buffer[count] = 0x00;

    std::string str = std::string(buffer);

    delete[] buffer;

    return str;
}
