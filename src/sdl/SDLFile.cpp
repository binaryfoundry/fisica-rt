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

void FileLoadTexture2D_RGBA8(
    std::string resource_id,
    uint8_t& bpp,
    uint32_t& width,
    uint32_t& height,
    std::unique_ptr<std::vector<uint8_t>>& data)
{
    std::string file_path = resource_id;
    file_path = std::string("resources/").append(file_path);

    SDL_Surface* surface_raw = IMG_Load(
        file_path.c_str());

    if (surface_raw == nullptr)
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_PixelFormat pixel_format;
    pixel_format.palette = 0;
    pixel_format.BitsPerPixel = 32;
    pixel_format.BytesPerPixel = 4;
    pixel_format.Rshift = 0;
    pixel_format.Rloss = 0;
    pixel_format.Gloss = 0;
    pixel_format.Bloss = 0;
    pixel_format.Aloss = 0;
    pixel_format.Rmask = 0x000000ff;
    pixel_format.Gshift = 8;
    pixel_format.Gmask = 0x0000ff00;
    pixel_format.Bshift = 16;
    pixel_format.Bmask = 0x00ff0000;
    pixel_format.Ashift = 24;
    pixel_format.Amask = 0xff000000;

    SDL_Surface* surface_converted = SDL_ConvertSurface(
        surface_raw,
        &pixel_format,
        SDL_SWSURFACE);

    SDL_FreeSurface(
        surface_raw);

    bpp = surface_converted->format->BytesPerPixel;
    width = surface_converted->w;
    height = surface_converted->h;
    uint8_t* pixels = static_cast<uint8_t*>(surface_converted->pixels);

    size_t total_size = width * height * bpp;
    data->resize(total_size);

    memcpy(&(*data)[0], pixels, total_size);

    SDL_FreeSurface(
        surface_converted);
}
