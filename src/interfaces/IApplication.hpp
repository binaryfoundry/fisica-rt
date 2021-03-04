#pragma once

class IApplication
{
public:
    virtual void Init() = 0;
    virtual void Deinit() = 0;
    virtual void Update() = 0;

    virtual ~IApplication() {};
};
