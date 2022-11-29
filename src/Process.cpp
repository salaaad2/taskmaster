#include "Process.hpp"

#include <sys/signal.h>
#include <vector>

#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "Utils.hpp"

Process::Process() :
    mIsAlive(false),
    mRestartOnError(false),
    mExecOnStartup(false),
    mHasStandardStreams(true),
    mExpectedReturn(0),
    mReturnValue(-1),
    mNumberOfRestarts(0),
    mPid(0),
    mStartTime(1.00),
    mFullPath(""),
    mProcessName(""),
    mWorkingDir(""),
    mAdditionalEnv(""),
    mOutputStreamRedirectPath(""),
    mCommandArguments(std::vector<string>())
{}

Process::Process(
        bool isAlive,
        bool restartOnError,
        bool execOnStartup,
        bool hasStandardStreams,
        int expectedReturn,
        int returnValue,
        int numberOfRestarts,
        long double startTime,
        const string &fullPath,
        const string &name,
        const string &workingDir,
        const string &additionalEnv,
        const string &outputRedirectPath,
        const std::vector<string> &commandArgs) :
    mIsAlive(isAlive),
    mRestartOnError(restartOnError),
    mExecOnStartup(execOnStartup),
    mHasStandardStreams(hasStandardStreams),
    mExpectedReturn(expectedReturn),
    mReturnValue(returnValue),
    mNumberOfRestarts(numberOfRestarts),
    mPid(0),
    mStartTime(startTime),
    mFullPath(fullPath),
    mProcessName(name),
    mWorkingDir(workingDir),
    mAdditionalEnv(additionalEnv),
    mOutputStreamRedirectPath(outputRedirectPath),
    mCommandArguments(commandArgs)
{}

Process::~Process() {}

int Process::start()
{
    int pid;
    int pipe_fds[2];

    if (pipe(pipe_fds) < 0)
    {
        std::cerr << "error: pipe\n";
        return 1;
        // return error(ERROR_FATAL, "Internal Error", "fork()")
    }
    if ((pid = fork()) < 0)
    {
        std::cerr << "error: fork\n";
        return 1;
        // return error(ERROR_FATAL, "Internal Error", "fork()")
    }

    if (pid == 0)
    {
        //handleChildPipes();
        int fd = STDOUT_FILENO;
        if (!getHasStandardStreams())
        {
            fd = open(getOutputRedirectPath().c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
        }
        else
        {
            fd = pipe_fds[1];
        }
        dup2(fd, STDOUT_FILENO);
        close(pipe_fds[0]);
        close(pipe_fds[1]);

        std::vector<const char*> arg_v =
            Utils::ContainerToConstChar(mProcessName, getCommandArguments());
        int exec_return =
            execv(mFullPath.c_str(), const_cast<char*const*>(arg_v.data()));
        // on execv success, this will never happen
        exit(exec_return); /*127*/
    }
    else
    {
        setIsAlive(true);
        close(pipe_fds[1]);
    }
    return getReturnValue();
}

int Process::stop()
{
    int ret = 0;

    if (!isAlive())
    {
        return 0;
    }
    else
    {
        ret = kill(mPid, SIGTERM);
    }
    return 0;
}

std::ostream & operator<<(std::ostream & s, const Process & src)
{
    string out;
    if (src.getCommandArguments().size() != 0)
    {
        out = Utils::JoinStrings(src.getCommandArguments(), ", ");
    }
    s << "Process:{ name: " << src.getProcessName()
      << ", isAlive: " << src.isAlive()
      << ", full_path: " << src.getFullPath()
      << ", start_command: [" << out
      << "] }\n";
    return s;
}

bool Process::isAlive() const
{
    return mIsAlive;
}

void Process::setIsAlive(bool newIsAlive)
{
    mIsAlive = newIsAlive;
}

bool Process::getRestartOnError() const
{
    return mRestartOnError;
}

void Process::setRestartOnError(bool newRestartOnError)
{
    mRestartOnError = newRestartOnError;
}

bool Process::getExecOnStartup() const
{
    return mExecOnStartup;
}

void Process::setExecOnStartup(bool newExecOnStartup)
{
    mExecOnStartup = newExecOnStartup;
}

bool Process::getHasStandardStreams() const
{
    return mHasStandardStreams;
}

void Process::setHasStandardStreams(bool newHasStandardStreams)
{
    mHasStandardStreams = newHasStandardStreams;
}

int Process::getExpectedReturn() const
{
    return mExpectedReturn;
}

void Process::setExpectedReturn(int newExpectedReturn)
{
    mExpectedReturn = newExpectedReturn;
}

int Process::getReturnValue() const
{
    return mReturnValue;
}

void Process::setReturnValue(int newReturnValue)
{
    mReturnValue = newReturnValue;
}

int Process::getNumberOfRestarts() const
{
    return mNumberOfRestarts;
}

void Process::setNumberOfRestarts(int newNumberOfRestarts)
{
    mNumberOfRestarts = newNumberOfRestarts;
}

int Process::getPid() const
{
    return mPid;
}

void Process::setPid(int newPid)
{
    mPid = newPid;
}


long double Process::getStartTime() const
{
    return mStartTime;
}

void Process::setStartTime(long double newStartTime)
{
    mStartTime = newStartTime;
}

const string &Process::getFullPath() const
{
    return mFullPath;
}

void Process::setFullPath(const string &newFullPath)
{
    mFullPath = newFullPath;
}

const string &Process::getProcessName() const
{
    return mProcessName;
}

void Process::setProcessName(const string &newProcessName)
{
    mProcessName = newProcessName;
}

const string &Process::getWorkingDir() const
{
    return mWorkingDir;
}

void Process::setWorkingDir(const string &newWorkingDir)
{
    mWorkingDir = newWorkingDir;
}

const string &Process::getAdditionalEnv() const
{
    return mAdditionalEnv;
}

void Process::setAdditionalEnv(const string &newAdditionalEnv)
{
    mAdditionalEnv = newAdditionalEnv;
}

const string &Process::getOutputRedirectPath() const
{
    return mOutputStreamRedirectPath;
}

void Process::setOutputRedirectPath(const string &newOutputRedirectPath)
{
    mOutputStreamRedirectPath = newOutputRedirectPath;
}

const std::vector<string> &Process::getCommandArguments() const
{
    return mCommandArguments;
}

void Process::appendCommandArgument(const string &newStartCommand)
{
    mCommandArguments.push_back(newStartCommand);
}
