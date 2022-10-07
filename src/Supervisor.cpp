#include "Supervisor.hpp"
#include <memory>
#include <yaml-cpp/yaml.h>

Supervisor::Supervisor() {}

Supervisor::Supervisor(const string config_path)
    : mLogFilePath("./"), mConfigFilePath(config_path), mIsConfigValid(false)
{
    loadConfig(mConfigFilePath);
}

Supervisor::~Supervisor() {}

int Supervisor::isConfigValid()
{return mIsConfigValid;}

void Supervisor::start() {}

int Supervisor::writeToLog()
{return (0);}

int Supervisor::loadConfig(const string & config_path) const
{
    YAML::Node config = YAML::LoadFile(config_path);

    auto processes_node = config["supervisor-processes"];
    if (processes_node)
    {
        for (auto it = processes_node.begin(); it != processes_node.end(); ++it)
        {
            // std::shared_ptr<Process> new_process = new Process();
            std::cout << it->first << ":" << it->second["full_path"] << "\n";

            // mProcessList.push_back(new_process);
        }
    }
    return (0);
}

int Supervisor::killAllProcesses(bool restart)
{
    if (restart)
    {
        //mProcessList.clear();
        return (0);
    }
    else
    {
        // for (auto : )
    }
    return (0);
}
