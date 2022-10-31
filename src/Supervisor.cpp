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

void Supervisor::init()
{
    for (auto p : mProcessList)
    {
        int r = 0;
        if (p->getExecOnStartup() == false)
        {
            continue;
        }
        r = p->start();
        if (r != p->getExpectedReturn())
        {
            // FIXME: error printing in functions
            std::cout << "taskmaster: Process [" << p->getProcessName() << "] failed to start\n"
                    << "  Expected: {" << p->getExpectedReturn() << "} Actual: {" << r << "}" << std::endl;
        }
        else
        {
        }
    }
}

int Supervisor::loadConfig(const string & config_path)
{
    YAML::Node config = YAML::LoadFile(config_path);

    auto processes_node = config["supervisor-processes"];
    if (!processes_node)
    {
        mIsConfigValid = false;
        return (1);
    }
    for (auto it = processes_node.begin(); it != processes_node.end(); ++it)
    {
        std::shared_ptr<Process> new_process = std::make_shared<Process>(Process());
        new_process->setFullPath(it->second["full_path"].as<string>());
        new_process->setProcessName(it->second["name"].as<string>());
        new_process->setExpectedReturn(it->second["expected_return"].as<int>());
        new_process->setExecOnStartup(it->second["exec_on_startup"].as<bool>());
        auto start_command = it->second["start_command"];
        for (auto c : start_command)
        {
            new_process->appendCommandArgument(c.as<string>());
        }
        std::cout << *new_process.get();
        mProcessList.push_back(new_process);
    }
    mIsConfigValid = (mProcessList.size() > 0);
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
