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
        int startProcess(std::shared_ptr<Process> & process);
        int loadConfig(const string & config_path);
        int killAllProcesses(bool restart);
        int writeToLog();

        /*
        ** class members
        */
        bool mIsConfigValid;
        string mConfigFilePath;
        const string mLogFilePath;
        std::vector<string> mCommandHistory;
        std::list<std::shared_ptr<Process> > mProcessList;
        std::map<string, string> mProcessMap;
        std::map<string, std::function<int(std::shared_ptr<Process>&)>> mCommandMap;
};
