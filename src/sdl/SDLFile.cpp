#include "SDLFile.hpp"

#include "SDL.hpp"

#include <iostream>

#if defined(EMSCRIPTEN)

File::File(std::string path, std::string mode)
{
    handle = fopen(path.c_str(), mode.c_str());

    if (handle == nullptr)
    {
        std::cout << "Failed to open file: " << path << std::endl;
        throw std::runtime_error("Failed to open file.");
    }

    fseek(handle, 0, SEEK_END);
    length = static_cast<size_t>(ftell(handle));
    rewind(handle);
}

File::~File()
{
    fclose(handle);
}

size_t File::Read(void* buffer, size_t size, size_t count)
{
    return fread(buffer, size, count, handle);
}

size_t File::Length()
{
    return length;
}

#else

File::File(std::string path, std::string mode)
{
    handle = SDL_RWFromFile(
        path.c_str(),
        mode.c_str());

    if (handle == nullptr)
    {
        std::cout << "Failed to open file: " << path << std::endl;
        throw std::runtime_error("Failed to open file.");
    }

    length = static_cast<size_t>(SDL_RWseek(
        handle,
        0,
        SEEK_END));

    SDL_RWseek(
        handle,
        0,
        SEEK_SET);
}

File::~File()
{
    SDL_RWclose(handle);
}

size_t File::Read(void* buffer, size_t size, size_t count)
{
    return SDL_RWread(handle, buffer, size, count);
}

size_t File::Length()
{
    return length;
}

#endif

std::string File::ReadString(uint16_t count)
{
    char* buffer = new char[count + 1];

    Read(buffer, sizeof(char), count);
    buffer[count] = 0x00;

    std::string str = std::string(buffer);

    delete[] buffer;

    return str;
}

// string prefixed with length (uint16_t)
std::string File::ReadString()
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
