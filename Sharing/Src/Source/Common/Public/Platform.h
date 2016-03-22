#pragma once

#ifdef __APPLE__

// On other platforms, strtok_s is the POSIX function strtok_r
#define strtok_s strtok_r
// On other platforms, sprintf_s corresponds to std::snprintf
#define sprintf_s std::snprintf
#define vsprintf_s std::vsnprintf

#endif