#ifndef PRINT_HPP
#define PRINT_HPP

#include <cstdio>
#include <string>

namespace Debug
{

template <typename... Args>
static int printError(const std::string &&module,
                      const std::string &&message,
                      int error,
                      const Args... args)
{
    std::string format("[Error %d] - " + module + " - " + message + "\n");
    return printf(format.c_str(), error, args...);
}

template <typename... Args>
static int printInfo(const std::string &&module,
                     const std::string &&message,
                     const Args... args)
{
    std::string format(module + " - " + message + "\n");
    return printf(format.c_str(), args...);
}

} // namespace Debug

#endif /* PRINT_HPP */
