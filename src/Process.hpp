#pragma once

#include <iostream>
#include <vector>

using std::string;

typedef enum ShouldRestart {
    Never,
    UnexpectedExit,
    Always
} ShouldRestart;

class Process {
public:
        /*
        ** xtors
        */
        Process();
        Process(const Process & process);
        Process(
            bool isAlive,
            bool execOnStartup,
            bool hasStandardStreams,
            std::vector<int> expectedReturn,
            int returnValue,
            int numberOfRestarts,
            int numberOfProcesses,
            int killSignal,
            double forceQuitWaitTime,
            int umask,
            ShouldRestart shouldRestart,
            const string &fullPath,
            const string &processName,
            const string &workingDir,
            const string &outputRedirectPath,
            const std::vector<string> &commandArgs,
            const std::vector<string> &additionalEnv);
        ~Process();

        /*
        ** business logic
        */
        int start();
        int stop();
        int kill();

        /*
        ** get/setters
        */
        bool isAlive() const;
        void setIsAlive(bool newIsAlive);
        bool getExecOnStartup() const;
        void setExecOnStartup(bool newExecOnStartup);
        bool getRedirectStreams() const;
        void setRedirectStreams(bool newHasStandardStreams);
        int  getReturnValue() const;
        void setReturnValue(int newReturnValue);
        bool isExpectedReturnValue(int ret_val) const;
        const std::vector<int>& getExpectedReturnValues() const;
        bool setExpectedReturns(const std::vector<int>& newExpectedReturn);
        int  getNumberOfRestarts() const;
        void setNumberOfRestarts(int newNumberOfRestarts);
        int  getNumberOfProcesses() const;
        void setNumberOfProcesses(int newNumberOfProcesses);
        int  getPid() const;
        void setPid(int newPid);
        int  getKillSignal() const;
        void setKillSignal(int killSignal);
        double getForceQuitWaitTime() const;
        void setForceQuitWaitTime(double forceQuitWaitTime);
        int  getUmask() const;
        void setUmask(int umask);
        ShouldRestart getShouldRestart() const;
        void setShouldRestart(int newShouldRestart);
        long double getStartTime() const;
        void setStartTime(long double newStartTime);
        long double getExecTime() const;
        void setExecTime(long double newExecTime);
        const string &getFullPath() const;
        void setFullPath(const string &newFullPath);
        const string &getStrerror() const;
        void setStrerror(const string &newStrerror);
        const string &getProcessName() const;
        void setProcessName(const string &newProcessName);
        const string &getWorkingDir() const;
        void setWorkingDir(const string &newWorkingDir);
        const string &getOutputRedirectPath() const;
        void setOutputRedirectPath(const string &newOutputRedirectPath);
        const std::vector<string> &getCommandArguments() const;
        void appendCommandArgument(const string &newStartCommand);
        const std::vector<string> &getAdditionalEnv() const;
        void addAdditionalEnvValue(const string &newAdditionalEnv);
private:
        /*
        ** private functions
        */

        /*
        ** class members
        */
        bool mIsAlive;
        bool mExecOnStartup;
        bool mRedirectStreams;
        std::vector<int> mExpectedReturnValues;
        int mReturnValue;
        int mNumberOfRestarts;
        int mNumberOfProcesses;
        int mPid;
        int mKillSignal;
        double mForceQuitWaitTime;
        int mUmask;
        ShouldRestart mShouldRestart;
        long double mStartTime;
        long double mExecTime;
        string mStrerror;
        string mFullPath;
        string mProcessName;
        string mWorkingDir;
        string mOutputStreamRedirectPath;
        std::vector<string> mCommandArguments;
        std::vector<string> mAdditionalEnv;
};

std::ostream & operator<<(std::ostream & s, const Process & src);
