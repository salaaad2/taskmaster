#include "Supervisor.hpp"
#include "Process.hpp"
#include "Utils.hpp"
#include <functional>
#include <memory>
#include <string>
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
        if (p->getExecOnStartup() == false)
        {
            continue;
        }
        startProcess(p);
    }
    // init REPL
    mCommandMap["start"] = std::bind(&Supervisor::startProcess, this, std::placeholders::_1);
    // start REPL
    std::string line;
    std::cout << "taskmasterctl>$ ";
    for (std::string line;
         std::getline(std::cin, line);
         std::cout << "taskmasterctl>$ ")
    {
        if (mCommandMap.find("start") != mCommandMap.end())
        {
            //mCommandMap["start"](mProcessList.findWithName())
        }
        std::cout << line << std::endl;
    }
}

int Supervisor::startProcess(std::shared_ptr<Process> & process)
{
    int process_return_val = 0;
    int number_of_restarts = process->getRestartOnError() ?
        process->getNumberOfRestarts() :
        1;

    for (auto i = 0; i < number_of_restarts; i++)
    {
        process_return_val = process->start();
        if (process_return_val != process->getExpectedReturn())
        {
            Utils::PrintError(process->getProcessName(), std::to_string(process->getExpectedReturn()));
        }
        else
        {
            Utils::PrintSuccess(process->getProcessName(), "started successfully");
            break;
        }
    }
    return process_return_val != process->getExpectedReturn();
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
        new_process->setNumberOfRestarts(it->second["number_of_restarts"].as<int>());
        new_process->setRestartOnError(it->second["restart_on_error"].as<bool>());
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
