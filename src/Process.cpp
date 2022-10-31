#include "Process.hpp"

#include <unistd.h>
#include <vector>

Process::Process() :
    mIsAlive(false),
    mRestartOnError(false),
    mExecOnStartup(false),
    mExpectedReturn(0),
    mStartTime(0.00),
    mFullPath(""),
    mProcessName(""),
    mWorkingDir(""),
    mAdditionalEnv(""),
    mCommandArguments(std::vector<string>())
{}

Process::Process(
        bool isAlive,
        bool restartOnError,
        bool execOnStartup,
        int expectedReturn,
        long double startTime,
        const string &fullPath,
        const string &name,
        const string &workingDir,
        const string &additionalEnv,
        const std::vector<string> &commandArgs) :
    mIsAlive(isAlive),
    mRestartOnError(restartOnError),
    mExecOnStartup(execOnStartup),
    mExpectedReturn(expectedReturn),
    mStartTime(startTime),
    mFullPath(fullPath),
    mProcessName(name),
    mWorkingDir(workingDir),
    mAdditionalEnv(additionalEnv),
    mCommandArguments(commandArgs)
{}

Process::~Process() {}

int Process::start()
{
    int pid;
	int pipe_fds[2];
	char out[4096];

	if (pipe(pipe_fds) < 0)
	{
		std::cerr << "error: pipe\n";
		return 1;
		//return error(ERROR_FATAL, "Internal Error", "fork()")
	}
	if ((pid = fork()) < 0)
	{
		std::cerr << "error: fork\n";
		return 1;
		//return error(ERROR_FATAL, "Internal Error", "fork()")
	}

	if (pid == 0)
	{
		dup2 (pipe_fds[1], 1);
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		execl(mFullPath.c_str(), mProcessName.c_str(), mCommandArguments.back().c_str(), (char *)0);
	} else {
		close(pipe_fds[1]);
		int nbytes = read(pipe_fds[0], out, sizeof(out));
		printf("Output: (%.*s)\n", nbytes, out);
		wait(NULL);
	}
    return 0;
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

const std::vector<string> &Process::getCommandArguments() const
{
    return mCommandArguments;
}

void Process::appendCommandArgument(const string &newStartCommand)
{
    mCommandArguments.push_back(newStartCommand);
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
