#pragma once

#include <iostream>
#include <vector>
#include <list>

class Process;

class Supervisor {
    public:
        Supervisor(const std::string & config_path);
        ~Supervisor();

        int isValidConfig();
        void start();
    private:
        // functions

        int writeToLog();
        int loadConfig();
        int killAllProcesses(bool restart);

        // members
        const std::string        & mLogFilePath;
        const std::string        & mConfigFilePath;
        std::vector<std::string> & mCommandHistory;
        std::list<Process>       & mProcessList;
        bool                       mIsConfigValid;
};
