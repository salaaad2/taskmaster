#include "Process.hpp"
#include "Supervisor.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <sys/signal.h>
#include <sys/wait.h>
#include <thread>
#include <string>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <ctime>

// anonymous namespace
namespace {
std::function<void(int)> sighup_handler;
void SignalLambdaWrapper(int signal)
{
    sighup_handler(signal);
}

/*
** Get node by value, check it's existence and return a
** default constructed value if not found, as well as setting is_node_valid to false
** you can pass initial_value as a parameter to find out if the read value is different
*/
template <typename T>
[[nodiscard]]
T GetYAMLNode(
    const YAML::iterator &node,
    const string &node_name,
    const T& initial_value,
    bool *is_node_valid,
    bool *value_changed,
    T default_value = T())
{
    if (node->second[node_name])
    {
        T new_value = node->second[node_name].as<T>();

        if (new_value != initial_value)
        {
            *value_changed = true;
        }
        *is_node_valid = true;
        return new_value;
    }
    return (default_value == T()) ? T() : default_value;
}

/*
** discard modification arguments
*/
template <typename T>
[[nodiscard]]
T GetYAMLNode(
    const YAML::iterator &node,
    const string &node_name,
    bool *is_node_valid)
{
    bool useless_bool;
    T useless_T = T();
    return (GetYAMLNode(node, node_name, useless_T, is_node_valid, &useless_bool));
}

string GetUniqueName(const string & base_name, int number)
{
    return base_name + "_" + std::to_string(number);
}
};

Supervisor::Supervisor()
{
    std::cout << "If you see this, you are in grave danger.\n";
}

Supervisor::Supervisor(
    const string config_path,
    const string log_file_path,
    char *envp[]) :
      mIsConfigValid(false),
      mConfigFilePath(config_path),
      mInitialEnvironment(envp)
{
    loadConfig(mConfigFilePath);
    mLogFilePath = (log_file_path.empty()) ?
        "./taskmaster.log" :
        log_file_path;
    mLogFile.open(mLogFilePath, std::fstream::out);
    Utils::LogStatus(mLogFile, "Starting taskmaster...\n");
}

Supervisor::~Supervisor()
{
    // make sure to stop all started programs if we exit the interpreter
    this->exit(mProcessMap[""]);
    Utils::LogStatus(mLogFile, "Exiting taskmaster...\n");
    for (auto p : mProcessMap)
    {
        p.second.reset();
    }
    ::exit(0);
}

[[nodiscard]]
int Supervisor::isConfigValid()
{return mIsConfigValid;}

void Supervisor::init()
{
    // start all processes that have exec_on_startup set to true
    for (auto& [key, p]: mProcessMap)
    {
        if (p->getExecOnStartup() == false)
        {
            continue;
        }
        startProcess(p);
    }

    // add signal to reload config
    sighup_handler = [&] (int signal){
        IGNORE(signal);
        reloadConfig(mProcessMap[""]);
    };
    signal(SIGHUP, SignalLambdaWrapper);
    // init REPL
    mCommandMap["help"]    = std::bind(&Supervisor::printHelp, this, std::placeholders::_1);
    mCommandMap["reload"]  = std::bind(&Supervisor::reloadConfig, this, std::placeholders::_1);
    mCommandMap["start"]   = std::bind(&Supervisor::startProcess, this, std::placeholders::_1);
    mCommandMap["stop"]    = std::bind(&Supervisor::stopProcess, this, std::placeholders::_1);
    mCommandMap["status"]  = std::bind(&Supervisor::getProcessStatus, this, std::placeholders::_1);
    mCommandMap["exit"]    = std::bind(&Supervisor::exit, this, std::placeholders::_1);
    mCommandMap["history"] = std::bind(&Supervisor::history, this, std::placeholders::_1);
    mCommandMap["list"]    = std::bind(&Supervisor::listProcesses, this, std::placeholders::_1);

    // start REPL
    string line;
    std::cout << "taskmasterctl>$ ";
    for (string line;
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
                    split_command.front() == "exit" ||
                    split_command.front() == "history") {
                    mCommandMap[(*it).first](mProcessMap[split_command.back()]);
                    mCommandHistory.push_back(line);
                }
            }
        }
    }
}

/*
** call _start in another thread and allow the user back into the REPL
*/
int Supervisor::startProcess(std::shared_ptr<Process> & process)
{
    std::thread start_thread(&Supervisor::_start, this, std::ref(process));
    start_thread.detach();
    return 0;
}

/*
** start and/or restart processes
*/
int Supervisor::_start(std::shared_ptr<Process> & process)
{
    int number_of_restarts = (process->getShouldRestart() != 0) ?
        process->getNumberOfRestarts() :
        1;

    // restart n times if 
    auto i = 0;
    while (i < number_of_restarts)
    {
        process->start();
        if (!process->isAlive())
        {
            Utils::LogError(
                mLogFile,
                process->getProcessName(),
                "Did not start. strerror: " + process->getStrerror());
            ++i;
            continue;
        }
        // the process managed to start, monitor it until it ends
        int ret = _monitor(process);
        switch (process->getShouldRestart())
        {
        case ShouldRestart::Never:
            { return 0; }
        case ShouldRestart::UnexpectedExit:
            if (ret != 0)
            {
                // restart if there was an error
                ++i;
                continue;
            }
            break;
        case ShouldRestart::Always:
            {
                // infinite loop
                i = 0;
                continue;
            }
        default:
            {
                Utils::LogError(
                    mLogFile,
                    process->getProcessName(),
                    "Invalid should_restart value provided. Exiting.");
            }
        }
        return 0;
    }
    return 0;
}

int Supervisor::_monitor(std::shared_ptr<Process>& process)
{
    int ret = 0;
    bool has_error = false;

    waitpid(process->getPid(), &ret, 0);

    process->setIsAlive(false);
    if (WIFEXITED(ret))
    {
        process->setReturnValue(WEXITSTATUS(ret));
    }
    else if (WIFSIGNALED(ret))
    {
        Utils::LogStatus(
            mLogFile,
            "Process " + process->getProcessName() +
            " killed by signal: " + std::to_string(WTERMSIG(ret)) + "\n");
    }

    // if a start_time was set in config, we need to make sure that we did not return 
    //  too early by doing current ?> exec_time + start_time
    if (process->getStartTime() != 0.0 &&
        std::time(nullptr) < (process->getExecTime() + process->getStartTime()))
    {
        Utils::LogError(
            mLogFile,
            process->getProcessName(),
            "Returned too early.");
        has_error = true;
    }
    if (!process->isExpectedReturnValue(process->getReturnValue()))
    {
        Utils::LogError(
            mLogFile,
            process->getProcessName(),
            "Unexpected return value: " + std::to_string(process->getReturnValue()) +
            " expected: " + std::to_string(process->getExpectedReturnValues().front()));
        has_error = true;
    }
    if (!has_error)
    {
        Utils::LogSuccess(
            mLogFile,
            process->getProcessName(),
            "Terminated without errors.");
    }
    else 
    {
        Utils::LogError(
            mLogFile,
            process->getProcessName(),
            "Encountered problems.");
    }
    return has_error;
}

int Supervisor::stopProcess(std::shared_ptr<Process> & process)
{
    int stop_return_val = 0;

    stop_return_val = process->stop();
    if (stop_return_val == -1)
    {
        Utils::LogError(mLogFile, process->getProcessName(), "is not running.");
    }
    else if (stop_return_val == 1)
    {
        // should we wait and force kill ?
        Utils::LogError(mLogFile, process->getProcessName(), 
            "kill(" + std::to_string(process->getKillSignal()) + ") did not return as expected.");
    }
    else
    {
        Utils::LogSuccess(mLogFile, process->getProcessName(), "Terminated.");
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
            if (proc.get() != nullptr)
            {
                std::cout << *proc.get() << "\n";
            }
        }
    }
    return 0;
}

int Supervisor::printHelp(std::shared_ptr<Process> & process)
{
    IGNORE(process);
    string out;
    out += "=========Taskmaster========\n";
    out += "----available commands:----\n";
    out += "help          : Print this help\n";
    out += "reload        : reload config file (" + mConfigFilePath + ")\n";
    out += "start  <name> : Start process by name\n";
    out += "status <name> : get status of program \n";
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
    IGNORE(process);
    return loadConfig(mConfigFilePath, true);
}

int Supervisor::exit(std::shared_ptr<Process>& process)
{
    IGNORE(process);

    std::cout.flush();
    int n = killAllProcesses(false);
    Utils::LogStatus(mLogFile, "Killed: " + std::to_string(n) + " processe(s);\n");
    return 0;
}

int Supervisor::history(std::shared_ptr<Process>& process)
{
    IGNORE(process);
    string out =
        "==== taskmaster command history ====\n";

    if (!mCommandHistory.empty())
        out += Utils::JoinStrings(mCommandHistory, "\n");
    std::cout << out;
    return 0;
}

int Supervisor::listProcesses(std::shared_ptr<Process>& process)
{
    IGNORE(process);
    string out =
        "==== taskmaster configured programs list ====\n";
    for (auto & v : mProcessMap)
    {
        if (v.second.get() != nullptr)
        {
            out += v.second->getProcessName() + "\n";
        }
    }
    std::cout << out;
    return 0;
}

/*
** load configuration from the provided .yaml file;
** some options are mandatoru and their absence will raise an error
**
** upon reload, override_existing is set to true; for processes whose parameters were changed,
** only a few of them will cause the program to restart them.
** these are:
** ['', '']
*/
int Supervisor::loadConfig(const string & config_path, bool override_existing)
{
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_path);
    } catch (std::exception e) {
        mIsConfigValid = false;
        Utils::LogError(mLogFile, config_path, "YAML::BadFile.");
        return (1);
    }

    auto processes_node = config["supervisor-processes"];
    if (!processes_node)
    {
        mIsConfigValid = false;
        Utils::LogError(mLogFile, config_path, "supervisor-processes node not found.");
        return (1);
    }
    bool is_node_valid = false;
    bool value_changed = false;
    bool restart = false;
    for (auto it = processes_node.begin(); it != processes_node.end(); ++it)
    {
        std::shared_ptr<Process> new_process = std::make_shared<Process>(Process());
        new_process->setProcessName(GetYAMLNode<string>(it, "name", &is_node_valid));
        if (!is_node_valid) {Utils::LogError(mLogFile, "name", "does not exist or is invalid"); continue;}

        auto old_process_it = mProcessMap.find(new_process->getProcessName());
        if (old_process_it != mProcessMap.end() && !override_existing)
        {
            Utils::LogError(mLogFile, new_process->getProcessName(), "already exists in process list.");
            continue;
        }
        auto old_process = (old_process_it == mProcessMap.end()) ?
            new_process :
            old_process_it->second;

        new_process->setFullPath(GetYAMLNode<string>(it, "full_path", old_process->getFullPath(), &is_node_valid, &value_changed));
        new_process->setWorkingDir(GetYAMLNode<string>(it, "working_directory", old_process->getWorkingDir(), &is_node_valid, &value_changed));

        if (!is_node_valid)
        {Utils::LogError(mLogFile, "full_path", "does not exist or is invalid"); continue;}
        else if (override_existing && value_changed)
        {Utils::LogStatus(mLogFile, "restarting process"); restart = true;}
        
        // 
        auto expected_return_node = it->second["expected_return"];
        switch(expected_return_node.Type())
        {
            case YAML::NodeType::Scalar:
                new_process->addExpectedReturn(expected_return_node.as<int>());
                break;
            case YAML::NodeType::Sequence:
                for (auto item : expected_return_node)
                {
                    new_process->addExpectedReturn(item.as<int>());
                }
                break;
            default:
                break;
        }

        if (!is_node_valid)
        {Utils::LogError(mLogFile, "expected_return", "does not exist or is invalid"); continue;}
        else if (override_existing && value_changed)
        {Utils::LogStatus(mLogFile, "restarting process"); restart = true;}

        new_process->setNumberOfRestarts(GetYAMLNode<int>(it, "number_of_restarts", old_process->getNumberOfRestarts(), &is_node_valid, &value_changed));
        if (!is_node_valid)
        {new_process->setNumberOfRestarts(1);}
        else if (override_existing && value_changed)
        {Utils::LogStatus(mLogFile, "restarting process"); restart = true;}

        new_process->setStartTime(GetYAMLNode<double>(it, "start_time", &is_node_valid));
        new_process->setRedirectStreams(GetYAMLNode<bool>(it, "redirect_streams", &is_node_valid));
        new_process->setOutputRedirectPath(GetYAMLNode<string>(it, "output_redirect_path", &is_node_valid));
        new_process->setExecOnStartup(GetYAMLNode<bool>(it, "exec_on_startup", &is_node_valid));

        // see Process.hpp for possible values and usage
        new_process->setShouldRestart(GetYAMLNode<int>(it, "should_restart", &is_node_valid));

        new_process->setKillSignal(GetYAMLNode<int>(it, "kill_signal", 0, &is_node_valid, &value_changed, SIGTERM));
        is_node_valid = false;
        auto umask = GetYAMLNode<int>(it, "umask", 0, &is_node_valid, &value_changed);
        if (is_node_valid)
        {new_process->setUmask(umask);}

        auto start_command = it->second["start_command"];
        for (auto c : start_command)
        {
            new_process->appendCommandArgument(c.as<string>());
        }
        is_node_valid = true;

        // if we want to create multiple processes, create a new one using the copy constructor,
        //  give it a unique name and add it to mProcessMap
        int n_processes = GetYAMLNode<int>(it, "number_of_processes", &is_node_valid);
        if (is_node_valid && n_processes > 1)
        {
            auto initial_name = new_process->getProcessName();
            mProcessMap[new_process->getProcessName()] = new_process;

            for (int i = 1; i <= n_processes; ++i)
            {
                auto name = GetUniqueName(initial_name, i);
                auto copy_process = std::make_shared<Process>(Process(*new_process.get()));
                copy_process->setProcessName(name);
                mProcessMap[name] = copy_process;
            }
        }
        else
        {
            mProcessMap[new_process->getProcessName()] = new_process;
        }
    
        // environment variables
        auto env_vars = it->second["additional_env"];
        if (!env_vars)
        {
            ;
        }
        char** env_ptr = mInitialEnvironment;
        while (*env_ptr)
        {
            new_process->addAdditionalEnvValue(*env_ptr);
            env_ptr++;
        }
        for (auto i : env_vars)
        {
            if (i.Type() != YAML::NodeType::Map)
            {
                break;
            }
            auto value_map = i.as<std::map<string, string> >();
            for (auto & v : value_map)
            {
                new_process->addAdditionalEnvValue(v.first + "=" + v.second);
            }
        }


        if (restart)
        {
            //std::cout << *new_process.get();
            new_process->stop();
            new_process->start();
        }
        restart = false;
    }
    mIsConfigValid = (mProcessMap.size() > 0);
    return (0);
}

// 
// kills all active processes, returns the number of killed
//
int Supervisor::killAllProcesses(bool restart)
{
    if (restart)
    {
        return (0);
    }
    int n = 0;
    for (auto & [key, process] : mProcessMap)
    {
        if (process.get() != nullptr && process->isAlive())
        {
            process->stop();
            Utils::LogStatus(mLogFile, "killing " + key + "\n");
            n++;
        }
    }
    return (n);
}
