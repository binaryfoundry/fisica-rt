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
