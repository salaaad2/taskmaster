#include <iostream>
#include "Supervisor.hpp"

int main(void)
{
    std::cout << "taskmaster v0.0.1" << std::endl;
    /* TODO: get args */
    Supervisor s("./testinput.yaml");
    if (!s.isConfigValid())
    {
        return (1);
    }
    else
    {
        s.init();
    }
    return 0;
}
