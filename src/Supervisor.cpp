#include "Supervisor.hpp"
#include <memory>
#include <yaml-cpp/yaml.h>

Supervisor::Supervisor() {}

Supervisor::Supervisor(const string config_path)
    : mIsConfigValid(false),
      mConfigFilePath(config_path),
      mLogFilePath("./")
{
    loadConfig(mConfigFilePath);
}

Supervisor::~Supervisor() {}

int Supervisor::isConfigValid()
{
    return mIsConfigValid;
}

void Supervisor::init() {}

int Supervisor::loadConfig(const string & config_path)
{
    YAML::Node config = YAML::LoadFile(config_path);

    auto processes_node = config["supervisor-processes"];
    if (processes_node)
    {
        for (auto it = processes_node.begin(); it != processes_node.end(); ++it)
        {
            std::shared_ptr<Process> new_process = std::make_shared<Process>(Process());
            new_process->setFullPath(it->second["full_path"].as<std::string>());
            new_process->setProcessName(it->second["name"].as<std::string>());
            new_process->setStartCommand(it->second["start_command"].as<std::string>());
            new_process->setExpectedReturn(it->second["expected_return"].as<int>());
            std::cout << *new_process.get();
            mProcessList.push_back(new_process);
        }
    }
    else
    {
        mIsConfigValid = false;
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

int Supervisor::writeToLog()
{
    return (0);
}
