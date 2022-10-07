#pragma once

#include <iostream>

using std::string;

class Process {
public:
        Process();
        Process(
            bool isAlive,
            bool restartOnError,
            int expectedReturn,
            long double startTime,
            const string &fullPath,
            const string &startCommand,
            const string &workingDir,
            const string &additionalEnv);

        ~Process();

        bool isAlive() const;
        void setIsAlive(bool newIsAlive);

        bool getRestartOnError() const;
        void setRestartOnError(bool newRestartOnError);

        int getExpectedReturn() const;
        void setExpectedReturn(int newExpectedReturn);

        long double getStartTime() const;
        void setStartTime(long double newStartTime);

        const string &getFullPath() const;
        void setFullPath(const string &newFullPath);

        const string &getStartCommand() const;
        void setStartCommand(const string &newStartCommand);

        const string &getWorkingDir() const;
        void setWorkingDir(const string &newWorkingDir);

        const string &getAdditionalEnv() const;
        void setAdditionalEnv(const string &newAdditionalEnv);

private:
        bool mIsAlive;
        bool mRestartOnError;
        int mExpectedReturn;
        long double mStartTime;
        string mFullPath;
        string mStartCommand;
        string mWorkingDir;
        string mAdditionalEnv;
};
