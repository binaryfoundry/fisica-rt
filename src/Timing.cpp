#include "Timing.hpp"

hrc::time_point timer_start()
{
    return hrc::now();
}

float timer_end(hrc::time_point& timer_start_point)
{
    const auto end = hrc::now();
    const auto time_span = std::chrono::duration_cast<std::chrono::duration<float>>(
        end - timer_start_point);
    return time_span.count() * 1000;
}
