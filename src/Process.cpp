#include "Process.hpp"

Process::Process() :
    mIsAlive(false),
    mRestartOnError(false),
    mExpectedReturn(0),
    mStartTime(0.00),
    mFullPath(""),
    mProcessName(""),
    mStartCommand(""),
    mWorkingDir(""),
    mAdditionalEnv("")
{}

Process::Process(
        bool isAlive,
        bool restartOnError,
        int expectedReturn,
        long double startTime,
        const string &fullPath,
        const string &name,
        const string &startCommand,
        const string &workingDir,
        const string &additionalEnv) :
    mIsAlive(isAlive),
    mRestartOnError(restartOnError),
    mExpectedReturn(expectedReturn),
    mStartTime(startTime),
    mFullPath(fullPath),
    mProcessName(name),
    mStartCommand(startCommand),
    mWorkingDir(workingDir),
    mAdditionalEnv(additionalEnv)
{}

Process::~Process() {}

std::ostream & operator<<(std::ostream & s, const Process & src)
{
    s << "Process:{ name: " << src.getProcessName()
      << ", isAlive: " << src.isAlive()
      << ", full_path: " << src.getFullPath()
      << ", start_command: " << src.getStartCommand()
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

int Process::getExpectedReturn() const
{
    return mExpectedReturn;
}

void Process::setExpectedReturn(int newExpectedReturn)
{
    mExpectedReturn = newExpectedReturn;
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

const string &Process::getStartCommand() const
{
    return mStartCommand;
}

void Process::setStartCommand(const string &newStartCommand)
{
    mStartCommand = newStartCommand;
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
