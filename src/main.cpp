#include <iostream>

#include "Supervisor.hpp"

int main(int ac, char *av[])
{
    (void)av;
    (void)ac;
    std::cout << "taskmaster v0.0.1" << std::endl;
    /* get args */
    Supervisor s("./input.yaml");
    if (!s.isConfigValid())
    {
        return (1);
    }
    else
    {
        s.start();
    }
    return 0;
}
