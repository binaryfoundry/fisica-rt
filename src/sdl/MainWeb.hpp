#pragma once

#include <memory>

#include "../interfaces/IApplication.hpp"

int sdl_init(std::unique_ptr<IApplication>& app);
