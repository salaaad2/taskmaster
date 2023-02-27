#include "Process.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <vector>

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <linux/limits.h>

#include "Utils.hpp"

int Process::start()
{
    string cool;
    pid_t pid;
    int pipe_fds[2];
    int fork_pipes[2];
    int count, err;

    // pipe for stdout
    if (::pipe(pipe_fds) < 0)
    {return 1;}

    // pipe for the `self-pipe trick`
    if (::pipe(fork_pipes) < 0)
    {return 1;}
    if (::fcntl(fork_pipes[1], F_SETFD, fcntl(fork_pipes[1], F_GETFD) | FD_CLOEXEC) < 0)
    {return 1;}

    // this is a bridge
    if ((pid = ::fork()) < 0)
    {return 1;}
    if (pid == 0)
    {
        mode_t mask = getUmask();
        if ((int)mask != -1)
        {
            ::umask(mask);
        }

        // output redirection
        int fd = STDOUT_FILENO;
        if (getRedirectStreams())
        {
            // if umask() was called, open() is affected.
            fd = ::open(getOutputRedirectPath().c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
            if (fd == -1)
            {
                ::write(fork_pipes[1], &errno, sizeof(int));
                ::exit(1);
            }
        }
        else
        {
            fd = pipe_fds[1];
        }

        // pipes
        ::dup2(fd, STDOUT_FILENO);
        ::dup2(fd, STDERR_FILENO);
        ::close(pipe_fds[0]);
        ::close(pipe_fds[1]);
        ::close(fork_pipes[0]);
        if (getWorkingDir() != "")
        {
            if (::chdir(getWorkingDir().c_str()) < 0)
            {
                ::write(fork_pipes[1], &errno, sizeof(int));
                ::exit(1);
            }
        }

        std::vector<const char*> arg_v =
            Utils::ContainerToConstChar(mProcessName, getCommandArguments());
        std::vector<const char*> env_v =
            Utils::ContainerToConstChar("", getAdditionalEnv());
        int exec_return =
            ::execve(
                mFullPath.c_str(),
                const_cast<char*const*>(arg_v.data()),
                const_cast<char*const*>(env_v.data()));
        // execv error: write errno to the pipe opened in the parent process
        ::write(fork_pipes[1], &errno, sizeof(int));
        ::exit(exec_return);
    }
    else
    {
        // read bytes from the pipe in the child process, which are sent only
        //  if execve failed (eg: upon call to a non-existent file)
        ::close(fork_pipes[1]);
        while ((count = ::read(fork_pipes[0], &err, sizeof(errno))) == -1)
        { if (errno != EAGAIN && errno != EINTR) {break;} }

        ::close(fork_pipes[0]);
        ::close(pipe_fds[1]);
        if (count)
        {
            setStrerror(std::strerror(err));
            setIsAlive(false);
            return -1;
        }

        setExecTime(std::time(nullptr));
        setPid(pid);
        setIsAlive(true);
    }
    return getReturnValue();
}

int Process::stop()
{
    int ret = 0;

    if (!isAlive())
    {
        return -1;
    }
    else
    {
        ret = ::kill(mPid, mKillSignal);
        setIsAlive(ret == 0);
        return (ret == 0) ? 0 : 1;
    }
    return ret;
}

int Process::kill()
{
    if (!isAlive())
    {
        return -1;
    }
    else
    {
        ::kill(mPid, SIGKILL);
        setIsAlive(false);
    }
    return 0;
}

std::ostream & operator<<(std::ostream & s, const Process & src)
{
    string out;
    string working_dir = src.getWorkingDir();
    if (working_dir.empty())
    {
        char path[PATH_MAX];
        getcwd(path, PATH_MAX);
        working_dir = path;
    }
    string rets;
    const std::vector<int>& ret_vals = src.getExpectedReturnValues();
    bool add_comma = ret_vals.size() != 1;
    for (auto it = ret_vals.begin(); it != ret_vals.end(); ++it)
    {
        rets += std::to_string(*it);
        if (add_comma)
        {
            rets += ", ";
        }
    }
    if (src.getCommandArguments().size() != 0)
    {
        out = Utils::JoinStrings(src.getCommandArguments(), ", ");
    }
    s << "[" << src.getProcessName() << "]"
      << "\n\trunning: " << ((src.isAlive()) ? "true, PID: " + std::to_string(src.getPid()) : "false")
      << "\n\tfull_path: " << src.getFullPath()
      << "\n\tstart_command: [" << out << "]"
      << "\n\tlog_to_file: " << ((src.getRedirectStreams()) ? "[" + src.getOutputRedirectPath() + "]" : "false")
      << "\n\tworking_dir: " << "\"" << working_dir << "\""
      << "\n\tvalid return values: " << "[" << rets << "]"
      << "\n";
    return s;
}

Process::Process() :
    mIsAlive(false),
    mExecOnStartup(false),
    mRedirectStreams(true),
    mExpectedReturnValues(std::vector<int>()),
    mReturnValue(-1),
    mNumberOfRestarts(0),
    mPid(0),
    mKillSignal(SIGTERM),
    mForceQuitWaitTime(0.0),
    mUmask(-1),
    mShouldRestart(ShouldRestart::Never),
    mStartTime(0.00),
    mExecTime(0.00),
    mFullPath(""),
    mProcessName(""),
    mWorkingDir(""),
    mOutputStreamRedirectPath(""),
    mCommandArguments(std::vector<string>()),
    mAdditionalEnv(std::vector<string>())
{}

Process::Process(const Process & process)
{
    mIsAlive = process.mIsAlive;
    mExecOnStartup = process.mExecOnStartup;
    mRedirectStreams = process.mRedirectStreams;
    mExpectedReturnValues = process.mExpectedReturnValues;
    mReturnValue = 0;
    mNumberOfRestarts = process.mNumberOfRestarts;
    mPid = process.mPid;
    mKillSignal = process.mKillSignal;
    mForceQuitWaitTime = process.mForceQuitWaitTime;
    mUmask = process.mUmask;
    mShouldRestart = process.mShouldRestart;
    mStartTime =  0.0;
    mExecTime = 0.0;
    mFullPath = process.mFullPath;
    mProcessName = process.mProcessName;
    mWorkingDir = process.mWorkingDir;
    mAdditionalEnv = process.mAdditionalEnv;
    mOutputStreamRedirectPath = process.mOutputStreamRedirectPath;
    mCommandArguments = process.mCommandArguments;
}

Process::Process(
        bool isAlive,
        bool execOnStartup,
        bool hasStandardStreams,
        std::vector<int> expectedReturnValues,
        int returnValue,
        int numberOfRestarts,
        int numberOfProcesses,
        int killSignal,
        double forceQuitWaitTime,
        int umask,
        ShouldRestart shouldRestart,
        const string &fullPath,
        const string &name,
        const string &workingDir,
        const string &outputRedirectPath,
        const std::vector<string> &commandArgs,
        const std::vector<string> &additionalEnv) :
    mIsAlive(isAlive),
    mExecOnStartup(execOnStartup),
    mRedirectStreams(hasStandardStreams),
    mExpectedReturnValues(expectedReturnValues),
    mReturnValue(returnValue),
    mNumberOfRestarts(numberOfRestarts),
    mNumberOfProcesses(numberOfProcesses),
    mPid(0),
    mKillSignal(killSignal),
    mForceQuitWaitTime(forceQuitWaitTime),
    mUmask(umask),
    mShouldRestart(shouldRestart),
    mStartTime(0.0),
    mExecTime(0.0),
    mFullPath(fullPath),
    mProcessName(name),
    mWorkingDir(workingDir),
    mOutputStreamRedirectPath(outputRedirectPath),
    mCommandArguments(commandArgs),
    mAdditionalEnv(additionalEnv)
{}

Process::~Process() {}

bool Process::isAlive() const
{
    return mIsAlive;
}

void Process::setIsAlive(bool newIsAlive)
{
    mIsAlive = newIsAlive;
}

ShouldRestart Process::getShouldRestart() const
{
    return mShouldRestart;
}

void Process::setShouldRestart(int newShouldRestart)
{
    mShouldRestart = (ShouldRestart)newShouldRestart;
}

bool Process::getExecOnStartup() const
{
    return mExecOnStartup;
}

void Process::setExecOnStartup(bool newExecOnStartup)
{
    mExecOnStartup = newExecOnStartup;
}

bool Process::getRedirectStreams() const
{
    return mRedirectStreams;
}

void Process::setRedirectStreams(bool newHasStandardStreams)
{
    mRedirectStreams = newHasStandardStreams;
}

bool Process::isExpectedReturnValue(int ret_val) const
{
    for (auto & r: mExpectedReturnValues)
    {
        if (r == ret_val)
            return true;
    }
    return false;
}

const std::vector<int>& Process::getExpectedReturnValues() const
{
    return mExpectedReturnValues;
}

// sets expected return values to newExpectedReturn,
// returns true if the value changed, false if not.
bool Process::setExpectedReturns(const std::vector<int>& newExpectedReturn)
{
    if (newExpectedReturn == mExpectedReturnValues)
        return false;
    mExpectedReturnValues.clear();
    for (int v : newExpectedReturn)
    {
        mExpectedReturnValues.push_back(v);
    }
    return true;
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

int Process::getNumberOfProcesses() const
{
    return mNumberOfProcesses;
}

void Process::setNumberOfProcesses(int newNumberOfProcesses)
{
    mNumberOfProcesses = newNumberOfProcesses;
}

int Process::getPid() const
{
    return mPid;
}

void Process::setPid(int newPid)
{
    mPid = newPid;
}

int Process::getKillSignal() const
{
    return mKillSignal;
}

void Process::setKillSignal(int newKillSignal)
{
    mKillSignal = newKillSignal;
}

double Process::getForceQuitWaitTime() const
{
    return mForceQuitWaitTime;
}

void Process::setForceQuitWaitTime(double newForceQuitWaitTime)
{
    mForceQuitWaitTime = newForceQuitWaitTime;
}

int Process::getUmask() const
{
    return mUmask;
}

void Process::setUmask(int newUmask)
{
    mUmask = newUmask;
}

long double Process::getStartTime() const
{
    return mStartTime;
}

void Process::setStartTime(long double newStartTime)
{
    mStartTime = newStartTime;
}

long double Process::getExecTime() const
{
    return mExecTime;
}

void Process::setExecTime(long double newExecTime)
{
    mExecTime = newExecTime;
}

const string &Process::getStrerror() const
{
    return mFullPath;
}

void Process::setStrerror(const string &newStrerror)
{
    mStrerror = newStrerror;
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

const std::vector<string> &Process::getAdditionalEnv() const
{
    return mAdditionalEnv;
}

void Process::addAdditionalEnvValue(const string &newAdditionalEnvValue)
{
    auto name = newAdditionalEnvValue.substr(0, newAdditionalEnvValue.find("="));

    // replace value if an envvar with the same name already exists
    for (auto& v: mAdditionalEnv)
    {
        if (name == v.substr(0, v.find("=")))
        {
            v = newAdditionalEnvValue;
        }
    }
    mAdditionalEnv.push_back(newAdditionalEnvValue);
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

