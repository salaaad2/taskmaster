#include "Process.hpp"
#include "Supervisor.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
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
#include <readline/readline.h>
#include <readline/history.h>

using namespace std::chrono_literals;
static bool sig_test = false;

// anonymous namespace
namespace {
std::function<void(int)> sighup_handler;
void SignalLambdaWrapper(int signal)
{
    sig_test = true;
    sighup_handler(signal);
}

/*
** Get node by value, check it's existence and return a
** default constructed value if not found, as well as setting is_node_valid to false
** you can pass initial_value as a parameter to find out if the read value is different
*/
template <typename T>
[[nodiscard]]
static auto GetYAMLNode(
    const YAML::iterator &node,
    const string &node_name,
    const T& initial_value,
    bool *is_node_valid,
    bool *value_changed,
    T default_value = T()) -> T
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
static auto GetYAMLNode(
    const YAML::iterator &node,
    const string &node_name,
    bool *is_node_valid) -> T
{
    bool useless_bool;
    T useless_T = T();
    return (GetYAMLNode(node, node_name, useless_T, is_node_valid, &useless_bool));
}

static auto GetUniqueName(const string & base_name, int number) -> string
{
    return base_name + "_" + std::to_string(number);
}

static auto SetProcessEnvironment(std::shared_ptr<Process>& new_process, YAML::Node env_vars, char **env_ptr) -> void
{
    while (*env_ptr)
    {
        new_process->addAdditionalEnvValue(*env_ptr);
        env_ptr++;
    }
    for (auto i : env_vars)
    {
        if (i.Type() != YAML::NodeType::Map)
            break;
        auto value_map = i.as<std::map<string, string> >();
        for (auto & v : value_map)
        {
            new_process->addAdditionalEnvValue(v.first + "=" + v.second);
        }
    }
}

static auto AddMultipleProcessesToList(
    int n_processes,
    std::unordered_map<string, std::shared_ptr<Process>>& process_map,
    std::shared_ptr<Process> new_process) -> void
{
    auto initial_name = new_process->getProcessName();
    process_map[new_process->getProcessName()] = new_process;
    for (int i = 1; i <= n_processes; ++i)
    {
        auto name = GetUniqueName(initial_name, i);
        auto copy_process = std::make_shared<Process>(Process(*new_process.get()));
        copy_process->setProcessName(name);
        process_map[name] = copy_process;
    }
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
    Utils::LogStatus(mLogFile, "COOL");
    //start all processes that have exec_on_startup set to true
    for (auto& [key, p]: mProcessMap)
    {
        if (p->getExecOnStartup() == false)
        {
            continue;
        }
        startProcess(p);
    }

    // init REPL
    mCommandMap["help"]    = std::bind(&Supervisor::printHelp, this, std::placeholders::_1);
    mCommandMap["reload"]  = std::bind(&Supervisor::reloadConfig, this, std::placeholders::_1);
    mCommandMap["start"]   = std::bind(&Supervisor::startProcess, this, std::placeholders::_1);
    mCommandMap["restart"] = std::bind(&Supervisor::restartProcess, this, std::placeholders::_1);
    mCommandMap["stop"]    = std::bind(&Supervisor::stopProcess, this, std::placeholders::_1);
    mCommandMap["status"]  = std::bind(&Supervisor::getProcessStatus, this, std::placeholders::_1);
    mCommandMap["exit"]    = std::bind(&Supervisor::exit, this, std::placeholders::_1);
    mCommandMap["history"] = std::bind(&Supervisor::history, this, std::placeholders::_1);
    mCommandMap["list"]    = std::bind(&Supervisor::listProcesses, this, std::placeholders::_1);

    // add signal to reload config
    struct sigaction shup_handler;
    sigemptyset(&shup_handler.sa_mask);
    shup_handler.sa_handler = SignalLambdaWrapper;
    shup_handler.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &shup_handler, NULL);

    sighup_handler = [&] (int signal) {
        IGNORE(signal);
        reloadConfig(mProcessMap[""]);
    };

    // start REPL
    for (;;)
    {
        bool found_command = false;
        char* input = ::readline("taskmasterctl>$ ");
        string line;
        if (!input)
        {
            // readline sets input to nullptr upon receiving a 
            //  signal
            if (sig_test)
            {
                std::cout << "\n";
                sig_test = false;
                continue;
            }
            return ;
        }
        line = input;
        add_history(input);
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
                    found_command = true;
                    if (split_command.front() == "exit")
                    {
                        return ;
                    }
                }
            }
        }
        if (!found_command)
        {
            std::cout << "Command not found: " << line << "\n";
        }
        if (input)
        {
            free(input);
        }
    }
}

/*
** call _start in another thread and allow the user back into the REPL.
** return value is discarded.
*/
int Supervisor::startProcess(std::shared_ptr<Process> & process)
{
    if (process->isAlive())
    {
        return 0;
    }
    std::thread start_thread(&Supervisor::_start, this, std::ref(process));
    start_thread.detach();
    return 0;
}

int Supervisor::restartProcess(std::shared_ptr<Process> & process)
{
    if (!process->isAlive())
    {
        startProcess(process);
        return 0;
    }
    stopProcess(process);
    startProcess(process);
    return 0;
}


/*
** start and/or restart processes
*/
void Supervisor::_start(std::shared_ptr<Process> & process)
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
        }

        // the process managed to start, monitor it until it ends
        int ret = _monitor(process);
        switch (process->getShouldRestart())
        {
        case ShouldRestart::Never:
            return ;
        case ShouldRestart::Always:
            continue;
        case ShouldRestart::UnexpectedExit:
            if (ret != 0)
            {
                // restart if there was an error
                ++i;
                continue;
            }
            return ;
        default:
            Utils::LogError(
                mLogFile,
                process->getProcessName(),
                "Invalid should_restart value provided. Exiting.");
            return ;
        }
    }
    return ;
}

[[nodiscard]]
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

[[nodiscard]]
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
            "kill(" + std::to_string(process->getKillSignal()) + ") did not return as expected. Force quitting (using SIGKILL).");
        std::this_thread::sleep_for(std::chrono::duration<double>(process->getForceQuitWaitTime()));
        process->kill();
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
    int n = killAllProcesses();
    Utils::LogStatus(mLogFile, "Killed: " + std::to_string(n) + " processe(s);\n");
    return 0;
}

int Supervisor::history(std::shared_ptr<Process>& process)
{
    IGNORE(process);
    string out =
        "==== taskmaster command history ====\n";

    HIST_ENTRY** history_entry_list = history_list();
    if (history_entry_list)
    {
        for (int i = 0; history_entry_list[i] != nullptr; i++)
        {
            out += string(history_entry_list[i]->line) + "\n";
        }
    }
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
    for (auto it = processes_node.begin(); it != processes_node.end(); ++it)
    {
        // three cases options:
        // I:  fatal, go to the next node
        // II: set default value
        // III:set incorrect value
        // IV: if value changed, program should restart
        bool is_node_valid = false;
        bool value_changed = false;
        bool restart = false;
        std::shared_ptr<Process> new_process = std::make_shared<Process>(Process());

        // options that might trigger a skip
        new_process->setProcessName(GetYAMLNode<string>(it, "name", &is_node_valid));
        if (!is_node_valid)
        {Utils::LogError(mLogFile, "name", "does not exist or is invalid"); continue;}

        auto old_process_it = mProcessMap.find(new_process->getProcessName());
        if (old_process_it != mProcessMap.end() && !override_existing)
        {Utils::LogError(mLogFile, new_process->getProcessName(), "already exists in process list."); continue; }

        // get pointer to the existing process if it exists as we might want to restart it
        auto old_process = (old_process_it == mProcessMap.end()) ?
            new_process :
            old_process_it->second;

        //SetPotentialRestartOptions();
        // options that might trigger a restart
        new_process->setFullPath(GetYAMLNode<string>(it, "full_path", old_process->getFullPath(), &is_node_valid, &value_changed));
        if (!is_node_valid)
        {Utils::LogError(mLogFile, "full_path", "does not exist or is invalid"); continue;}
        else if (override_existing && value_changed)
        {Utils::LogStatus(mLogFile, "restarting process\n"); restart = true;}
        value_changed = false;

        new_process->setNumberOfRestarts(GetYAMLNode<int>(it, "number_of_restarts", old_process->getNumberOfRestarts(), &is_node_valid, &value_changed));
        if (!is_node_valid)
        {new_process->setNumberOfRestarts(1);}
        else if (override_existing && value_changed && restart == false)
        {Utils::LogStatus(mLogFile, "restarting process\n"); restart = true;}
        value_changed = false;

        auto expected_return_node = it->second["expected_return"];
        if (!expected_return_node)
        { Utils::LogError(mLogFile, new_process->getProcessName(), "Invalid return value set."); continue; }

        new_process->setExpectedReturns(old_process->getExpectedReturnValues());
        std::vector<int> return_values;
        switch(expected_return_node.Type()) {
        case YAML::NodeType::Scalar:
            return_values.push_back(expected_return_node.as<int>());
            value_changed = new_process->setExpectedReturns(return_values);
            break;
        case YAML::NodeType::Sequence:
            for (auto item : expected_return_node)
            {
                return_values.push_back(item.as<int>());
            }
            value_changed = new_process->setExpectedReturns(return_values);
            break;
        default:
            break;
        }
        if (!is_node_valid)
        {Utils::LogError(mLogFile, "expected_return", "does not exist or is invalid"); continue;}
        else if (override_existing && value_changed)
        {Utils::LogStatus(mLogFile, "restarting process\n"); restart = true;}
        value_changed = false;

        // setValuesWithNoErrorChecking
        new_process->setStartTime(GetYAMLNode<double>(it, "start_time", &is_node_valid));
        new_process->setRedirectStreams(GetYAMLNode<bool>(it, "redirect_streams", &is_node_valid));
        new_process->setOutputRedirectPath(GetYAMLNode<string>(it, "output_redirect_path", &is_node_valid));
        new_process->setExecOnStartup(GetYAMLNode<bool>(it, "exec_on_startup", &is_node_valid));
        // see Process.hpp for possible values and usage
        new_process->setShouldRestart(GetYAMLNode<int>(it, "should_restart", &is_node_valid));
        new_process->setWorkingDir(GetYAMLNode<string>(it, "working_directory", old_process->getWorkingDir(), &is_node_valid, &value_changed));
        new_process->setKillSignal(GetYAMLNode<int>(it, "kill_signal", 0, &is_node_valid, &value_changed, SIGTERM));
        new_process->setUmask(GetYAMLNode<int>(it, "umask", 0, &is_node_valid, &value_changed, -1));
        new_process->setForceQuitWaitTime(GetYAMLNode<double>(it, "force_quit_wait_time", 0, &is_node_valid, &value_changed, 0.0));
        is_node_valid = false;

        auto start_command = it->second["start_command"];
        for (auto c : start_command) {
            new_process->appendCommandArgument(c.as<string>());
        }

        // setEnvironment
        auto env_vars = it->second["additional_env"];
        char** env_ptr = mInitialEnvironment;
        if (env_vars){
            SetProcessEnvironment(new_process, env_vars, env_ptr);
        }

        // if we want to create multiple processes, create a new one using the copy constructor,
        //  give it a unique name and add it to mProcessMap
        int n_processes = GetYAMLNode<int>(it, "number_of_processes", &is_node_valid);
        if (is_node_valid && n_processes > 1)
        {
            AddMultipleProcessesToList(n_processes, mProcessMap, new_process);
        }
        else
        {
            mProcessMap[new_process->getProcessName()] = new_process;
        }

        if (restart)
        {
            new_process->stop();
            _start(new_process);
        }
    }
    mIsConfigValid = (mProcessMap.size() > 0);
    return (0);
}

// 
// kills all active processes, returns the number of killed
//
int Supervisor::killAllProcesses()
{
    int n = 0;
    for (auto & [key, process] : mProcessMap)
    {
        if (process.get() != nullptr && process->isAlive())
        {
            process->kill();
            Utils::LogStatus(mLogFile, "killing " + key + "\n");
            n++;
        }
    }
    return (n);
}
