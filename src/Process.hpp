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
            bool hasStandardStreams,
            int expectedReturn,
            int returnValue,
            int numberOfRestarts,
            int killSignal,
            const string &fullPath,
            const string &processName,
            const string &workingDir,
            const string &additionalEnv,
            const string &outputRedirectPath,
            const std::vector<string> &commandArgs);
        ~Process();

        /*
        ** business logic
        */
        int start() ;
        int stop() ;

        /*
        ** get/setters
        */
        bool isAlive() const;
        void setIsAlive(bool newIsAlive);
        bool getRestartOnError() const;
        void setRestartOnError(bool newRestartOnError);
        bool getExecOnStartup() const;
        void setExecOnStartup(bool newExecOnStartup);
        bool getRedirectStreams() const;
        void setRedirectStreams(bool newHasStandardStreams);
        int  getReturnValue() const;
        void setReturnValue(int newReturnValue);
        int  getExpectedReturn() const;
        void setExpectedReturn(int newExpectedReturn);
        int  getNumberOfRestarts() const;
        void setNumberOfRestarts(int newNumberOfRestarts);
        int  getPid() const;
        void setPid(int newPid);
        int  getKillSignal() const;
        void setKillSignal(int killSignal);
        int  getUmask() const;
        void setUmask(int umask);
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
        const string &getAdditionalEnv() const;
        void setAdditionalEnv(const string &newAdditionalEnv);
        const string &getOutputRedirectPath() const;
        void setOutputRedirectPath(const string &newOutputRedirectPath);
        const std::vector<string> &getCommandArguments() const;
        void appendCommandArgument(const string &newStartCommand);
private:
        /*
        ** private functions
        */

        /*
        ** class members
        */
        bool mIsAlive;
        bool mRestartOnError;
        bool mExecOnStartup;
        bool mRedirectStreams;
        int mExpectedReturn;
        int mReturnValue;
        int mNumberOfRestarts;
        int mPid;
        int mKillSignal;
        int mUmask;
        long double mStartTime;
        long double mExecTime;
        string mStrerror;
        string mFullPath;
        string mProcessName;
        string mWorkingDir;
        string mAdditionalEnv;
        string mOutputStreamRedirectPath;
        std::vector<string> mCommandArguments;
};

std::ostream & operator<<(std::ostream & s, const Process & src);
