#pragma once

#include "Process.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>

using std::string;

class Supervisor {
    public:

        /*
        ** xtors
        */
        Supervisor();
        Supervisor(const string config_path, const string log_file_path, char *env[]);
        ~Supervisor();

        /*
        ** business logic
        */
        int loadConfig(const string & config_path, bool override_existing = false);
        int isConfigValid();
        void init();
        void restart();
    private:

        /*
        ** private functions
        */
        int killAllProcesses();

        void _start(std::shared_ptr<Process> & process);
        int _monitor(std::shared_ptr<Process> & process);

        /*
        ** functions called by REPL
        */
        int startProcess(std::shared_ptr<Process> & process);
        int restartProcess(std::shared_ptr<Process> & process);
        int stopProcess(std::shared_ptr<Process> & process);
        int getProcessStatus(std::shared_ptr<Process> & process);

        /* process param can be ignored in these functions */
        int printHelp(std::shared_ptr<Process> & process);
        int reloadConfig(std::shared_ptr<Process> &process);
        int exit(std::shared_ptr<Process> &process);
        int history(std::shared_ptr<Process> &process);
        int listProcesses(std::shared_ptr<Process> &process);

        /*
        ** class members
        */
        bool mIsConfigValid;
        string mConfigFilePath;
        string mLogFilePath;
        char ** mInitialEnvironment;
        std::fstream mLogFile;
        std::unordered_map<string, std::shared_ptr<Process> > mProcessMap;
        std::unordered_map<string, std::function<int(std::shared_ptr<Process>&)> > mCommandMap;
};
