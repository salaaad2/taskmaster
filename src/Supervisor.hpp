#pragma once

#include <iostream>
#include <vector>
#include <list>

class Process;

class Supervisor {
    public:
        Supervisor();
        Supervisor(const std::string config_path);
        ~Supervisor();

        int isConfigValid();
        void start();
    private:
        // functions

        int writeToLog();
        int loadConfig(const std::string & config_path) const ;
        int killAllProcesses(bool restart);

        // members
        const std::string          mLogFilePath;
        std::string                mConfigFilePath;
        std::vector<std::string>   mCommandHistory;
        //std::list<Process>         mProcessList;
        bool                       mIsConfigValid;
};
