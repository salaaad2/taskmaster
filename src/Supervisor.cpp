#include "Supervisor.hpp"
#include <yaml-cpp/yaml.h>

Supervisor::Supervisor() {}

Supervisor::Supervisor(const std::string config_path)
{
    mConfigFilePath = config_path;
    loadConfig(mConfigFilePath);
}

Supervisor::~Supervisor() {}

int Supervisor::isConfigValid()
{return mIsConfigValid;}

void Supervisor::start() {}

int Supervisor::writeToLog()
{return (0);}

int Supervisor::loadConfig(const std::string & config_path) const
{(void)config_path; return (0);}

int Supervisor::killAllProcesses(bool restart)
{(void)restart; return (0);}
