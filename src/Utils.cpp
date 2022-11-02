#include "Utils.hpp"

void Utils::PrintError(const string & source, const string & reason)
{
    std::cout << "taskmaster: error: " << source << ": " << reason << std::endl;
}

void Utils::PrintSuccess(const string & source, const string & reason)
{
    std::cout << "taskmaster: success: " << source << ": " << reason << std::endl;
}
