#include "SDL.hpp"

#include <string>
#include <vector>

class File
{
private:
    size_t length;

#if defined (EMSCRIPTEN)
    FILE* handle;
#else
    SDL_RWops* handle;
#endif

public:
    File(std::string path, std::string mode);
    virtual ~File();

    size_t Read(void* buffer, size_t size, size_t count);

    std::string ReadString(uint16_t count);
    std::string ReadString();

    size_t Length();
};

void FileLoadTexture2D_RGBA8(
    std::string resource_id,
    uint8_t& bpp,
    uint32_t& width,
    uint32_t& height,
    std::shared_ptr<std::vector<uint8_t>> data);
