#pragma once

#include <iostream>
#include <vector>
#include <list>

class Process;

using std::string;

class Supervisor {
    public:
        Supervisor();
        Supervisor(const string config_path);
        ~Supervisor();

        int isConfigValid();
        void start();
    private:
        // functions
        int writeToLog();
        int loadConfig(const string & config_path) const ;
        int killAllProcesses(bool restart);

        // members
        const string          mLogFilePath;
        string                mConfigFilePath;
        std::vector<string>   mCommandHistory;
        //std::list<Process>         mProcessList;
        bool                       mIsConfigValid;
};
