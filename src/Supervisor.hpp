#pragma once

#include "Process.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <functional>


using std::string;

class Supervisor {
    public:

        /*
        ** xtors
        */
        Supervisor();
        Supervisor(const string config_path, const string log_file_path);
        ~Supervisor();

        /*
        ** business logic
        */
        int isConfigValid();
        void init();
    private:

        /*
        ** private functions
        */
        int loadConfig(const string & config_path, bool override_existing = false);
        int killAllProcesses(bool restart);

        /*
        ** functions called by REPL
        */
        int startProcess(std::shared_ptr<Process> & process);
        int stopProcess(std::shared_ptr<Process> & process);
        int getProcessStatus(std::shared_ptr<Process> & process);
        int monitorProcess(std::shared_ptr<Process> & process);
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
        std::fstream mLogFile;
        std::vector<string> mCommandHistory;
        std::map<string, std::shared_ptr<Process> > mProcessMap;
        std::map<string, std::function<int(std::shared_ptr<Process>&)> > mCommandMap;
};
