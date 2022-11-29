#include "Process.hpp"
#include "Supervisor.hpp"
#include "Utils.hpp"

#include <functional>
#include <memory>
#include <thread>
#include <string>
#include <yaml-cpp/yaml.h>

Supervisor::Supervisor() {}

Supervisor::Supervisor(
    const string config_path,
    const string log_file_path) :
      mIsConfigValid(false),
      mConfigFilePath(config_path)
{
    loadConfig(mConfigFilePath);
    mLogFilePath = (log_file_path.empty()) ?
        "./taskmaster.log" :
        log_file_path;
    mLogFile.open(mLogFilePath, std::fstream::out);
}

Supervisor::~Supervisor()
{}

int Supervisor::isConfigValid()
{
    return mIsConfigValid;
}

void Supervisor::init()
{
    for (auto& [key, p]: mProcessMap)
    {
        if (p->getExecOnStartup() == false)
        {
            continue;
        }
        startProcess(p);
    }
    // init REPL
    mCommandMap["help"] = std::bind(&Supervisor::printHelp, this, std::placeholders::_1);
    mCommandMap["reload"] = std::bind(&Supervisor::reloadConfig, this, std::placeholders::_1);
    mCommandMap["start"] = std::bind(&Supervisor::startProcess, this, std::placeholders::_1);
    mCommandMap["status"] = std::bind(&Supervisor::getProcessStatus, this, std::placeholders::_1);
    mCommandMap["exit"] = std::bind(&Supervisor::exit, this, std::placeholders::_1);
    mCommandMap["history"] = std::bind(&Supervisor::history, this, std::placeholders::_1);

    // start REPL
    std::string line;
    std::cout << "taskmasterctl>$ ";
    for (std::string line;
         std::getline(std::cin, line);
         std::cout << "taskmasterctl>$ ")
    {
        std::cout << line << "\n";
        auto split_command = Utils::SplitString(line, " ");
        if (split_command.size() == 0)
        {
            continue ;
        }
        for (auto it = mCommandMap.begin(); it != mCommandMap.end(); ++it)
        {
            if ((*it).first == split_command.front())
            {
                if (mProcessMap.find(split_command.back()) != mProcessMap.end() ||
                    split_command.front() == "help" ||
                    split_command.front() == "reload" ||
                    split_command.front() == "status" ||
                    split_command.front() == "list" ||
                    split_command.front() == "history") {
                    mCommandMap[(*it).first](mProcessMap[split_command.back()]);
                    mCommandHistory.push_back(line);
                }
            }
        }
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
        process->start();
        std::thread monitor_thread(&Supervisor::monitorProcess, this, std::ref(process));
        monitor_thread.detach();
    }
    return process_return_val != process->getExpectedReturn();
}

int Supervisor::monitorProcess(std::shared_ptr<Process>& process)
{
    int ret = 0;

    if (!process->isAlive())
    {
        Utils::LogError(
            mLogFile,
            process->getProcessName(),
            "ended prematurely");
        process->setReturnValue(-123);
    }
    else
    {
        waitpid(process->getPid(), &ret, 0);
        process->setIsAlive(false);
        process->setReturnValue(ret);
    }
    if (process->getReturnValue() != process->getExpectedReturn())
    {
        Utils::LogError(
            mLogFile,
            process->getProcessName(),
            std::to_string(process->getExpectedReturn()));
    }
    else
    {
        Utils::LogSuccess(
            mLogFile,
            process->getProcessName(),
            "Ended successfully.");
    }
    return 0;
}

int Supervisor::getProcessStatus(std::shared_ptr<Process> & process)
{
    if (process.get() != nullptr)
    {
        std::cout << *process.get() << "\n";
    }
    else
    {
        for (auto & [key, proc]: mProcessMap)
        {
            std::cout << *proc.get() << "\n";
        }
    }
    return 0;
}

int Supervisor::printHelp(std::shared_ptr<Process> & process)
{
    IGNORE(process);
    std::string out;
    out += "=========Taskmaster========\n";
    out += "----available commands:----\n";
    out += "help          : Print this help\n";
    out += "reload        : reload config file (" + mConfigFilePath + ")\n";
    out += "start  <name> : Start process by name\n";
    out += "status <name> : get status of program \n";
    out += "list          : TODO: List processes\n";
    out += "history       : command history\n";
    out += "exit          : terminate all programs and exit\n";
    std::cout << out;
    return 0;
}

/*
** reload process configuration, or, if no process is specified,
**  reload full config
 */
int Supervisor::reloadConfig(std::shared_ptr<Process> & process)
{
    if (process)
    {
        // loadConfig("", process->getName());
    }
    return loadConfig(mConfigFilePath);
}

int Supervisor::exit(std::shared_ptr<Process>& process)
{
    IGNORE(process);
    int n = 0;
    int original_size = mProcessMap.size();

    for (auto & [key, process] : mProcessMap)
    {
        Utils::LogStatus(mLogFile, "killing " + key + "\n");
        if (process->isAlive())
        {
            process->stop();
            n++;
        }
    }
    Utils::LogStatus(mLogFile, "killed: " + std::to_string(n) + "processes, " + std::to_string(original_size - n) + "were already stopped\n");
    return 0;
}

int Supervisor::history(std::shared_ptr<Process>& process)
{
    IGNORE(process);
    std::string out =
        "==== taskmaster command history ====\n";

    out += Utils::JoinStrings(mCommandHistory, "\n");
    std::cout << out;
    return 0;
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
        new_process->setHasStandardStreams(it->second["has_standard_streams"].as<bool>());
        new_process->setOutputRedirectPath(it->second["output_redirect_path"].as<string>());
        new_process->setExpectedReturn(it->second["expected_return"].as<int>());
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
        mProcessMap[new_process->getProcessName()] = new_process;
    }
    mIsConfigValid = (mProcessMap.size() > 0);
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
