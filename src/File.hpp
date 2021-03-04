#include <any>
#include <string>
#include <vector>

class File
{
private:
    size_t length;
    std::any handle;

public:
    File(std::string path, std::string mode);
    virtual ~File();

    size_t Read(void* buffer, size_t size, size_t count);

    std::string ReadString();
    std::string ReadStringPrefixed();

    size_t Length();
};
