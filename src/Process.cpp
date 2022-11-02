#include "Process.hpp"
#include "StringUtils.hpp"

#include <cstdlib>
#include <unistd.h>
#include <vector>

#include <sys/wait.h>

Process::Process() :
    mIsAlive(false),
    mRestartOnError(false),
    mExecOnStartup(false),
    mHasStandardStreams(true),
    mExpectedReturn(0),
    mNumberOfRestarts(0),
    mStartTime(0.00),
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
    mNumberOfRestarts(numberOfRestarts),
    mStartTime(startTime),
    mFullPath(fullPath),
    mProcessName(name),
    mWorkingDir(workingDir),
    mAdditionalEnv(additionalEnv),
    mOutputStreamRedirectPath(outputRedirectPath),
    mCommandArguments(commandArgs)
{}

Process::~Process() {}

int Process::start() const
{
    int pid;
    int pipe_fds[2];
    char out[4096];

    if (getHasStandardStreams())
    {
       if (pipe(pipe_fds) < 0)
       {
        std::cerr << "error: pipe\n";
        return 1;
        // return error(ERROR_FATAL, "Internal Error", "fork()")
       }
    }
    if ((pid = fork()) < 0)
    {
       std::cerr << "error: fork\n";
       return 1;
       // return error(ERROR_FATAL, "Internal Error", "fork()")
    }

    int ret = 0;
    if (pid == 0)
    {
       if (!getHasStandardStreams() && getOutputRedirectPath().size() != 0)
       {
           //handleChildPipes();
           dup2(pipe_fds[1], 1);
           close(pipe_fds[0]);
           close(pipe_fds[1]);
       }
       std::vector<const char*> arg_v = JoinStrings(mProcessName, getCommandArguments(), "" , "");
       execv(mFullPath.c_str(),
           const_cast<char*const*>(arg_v.data()));
    }
    else
    {
       if (!getHasStandardStreams() && getOutputRedirectPath().size() != 0)
       {
           // handleParentPipes();
           close(pipe_fds[1]);
           int nbytes = read(pipe_fds[0], out, sizeof(out));
           printf("process_output:\n%.*s\n", nbytes, out);
           return 0;
       }
       else
       {
           waitpid(pid, &ret, 0);
       }
       if (WIFEXITED(ret))
       {
          ret = WEXITSTATUS(ret);
       }
    }
    return ret;
}

int Process::attemptLaunch() const
{
    return (0);
}

std::ostream & operator<<(std::ostream & s, const Process & src)
{
    s << "Process:{ name: " << src.getProcessName()
      << ", isAlive: " << src.isAlive()
      << ", full_path: " << src.getFullPath()
      << ", start_command: " << src.getCommandArguments().front()
      << " }\n";
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

int Process::getNumberOfRestarts() const
{
    return mNumberOfRestarts;
}

void Process::setNumberOfRestarts(int newNumberOfRestarts)
{
    mNumberOfRestarts = newNumberOfRestarts;
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
