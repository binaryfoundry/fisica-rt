#pragma once

#include <chrono>

using hrc = std::chrono::high_resolution_clock;

hrc::time_point timer_start();
float timer_end(hrc::time_point& timer_start_point);
