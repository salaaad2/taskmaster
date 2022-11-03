#pragma once

#include "Process.hpp"

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
        Supervisor(const string config_path);
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
        int loadConfig(const string & config_path);
        int killAllProcesses(bool restart);
        int writeToLog();

        /*
        ** functions called by REPL
        */
        int startProcess(std::shared_ptr<Process> & process);
        int printHelp(std::shared_ptr<Process> & process);
        int reloadConfig(std::shared_ptr<Process> &process);
        int exit(std::shared_ptr<Process> &process);

        /*
        ** class members
        */
        bool mIsConfigValid;
        string mConfigFilePath;
        const string mLogFilePath;
        std::vector<string> mCommandHistory;
        std::map<string, std::shared_ptr<Process>> mProcessMap;
        std::map<string, std::function<int(std::shared_ptr<Process>&)>> mCommandMap;
};
