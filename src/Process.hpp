#pragma once

#include <iostream>
#include <vector>

using std::string;

class Process {
public:

        /*
        ** xtors
        */
        Process();
        Process(
            bool isAlive,
            bool restartOnError,
            bool execOnStartup,
            int expectedReturn,
            long double startTime,
            const string &fullPath,
            const string &processName,
            const string &workingDir,
            const string &additionalEnv,
            const std::vector<string> &commandArgs);
        ~Process();

        /*
        ** business logic
        */
        int start();

        /*
        ** get/setters
        */
        bool isAlive() const;
        void setIsAlive(bool newIsAlive);
        bool getRestartOnError() const;
        void setRestartOnError(bool newRestartOnError);
        bool getExecOnStartup() const;
        void setExecOnStartup(bool newExecOnStartup);
        int getExpectedReturn() const;
        void setExpectedReturn(int newExpectedReturn);
        long double getStartTime() const;
        void setStartTime(long double newStartTime);
        const string &getFullPath() const;
        void setFullPath(const string &newFullPath);
        const string &getProcessName() const;
        void setProcessName(const string &newProcessName);
        const string &getWorkingDir() const;
        void setWorkingDir(const string &newWorkingDir);
        const string &getAdditionalEnv() const;
        void setAdditionalEnv(const string &newAdditionalEnv);
        const std::vector<string> &getCommandArguments() const;
        void appendCommandArgument(const string &newStartCommand);
private:

        /*
        ** class members
        */
        bool mIsAlive;
        bool mRestartOnError;
        bool mExecOnStartup;
        int mExpectedReturn;
        long double mStartTime;
        string mFullPath;
        string mProcessName;
        string mWorkingDir;
        string mAdditionalEnv;
        std::vector<string> mCommandArguments;
};

std::ostream & operator<<(std::ostream & s, const Process & src);
