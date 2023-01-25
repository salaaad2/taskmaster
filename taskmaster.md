#+TITLE: Taskmaster

# TODO:
## readline-cpp

## hot reload:
use GetYAMLValues( is_modified )
## killsig
sanity check:
- process not killed behaviour
## additional/different env values
single function call ? execv() -> execve() + add new values to envp arg
## multiple acceptable return values

# the configuration file must allow the user to specify the following, for each program that is to be supervised:
## The number of processes to start and keep running
## Whether the program should be restarted never, unexpected exit, or always (0, 1, 2)
## How many times a restart should be attempted before aborting
## How long to wait after a graceful stop before killing the program
### DONE: How long the program should be running after it’s started for it to be considered successfully started"
### DONE: The command to use to launch the program
### DONE: Whether to start this program at launch or not
### DONE: Which signal should be used to stop (i.e. exit gracefully) the program
### DONE: Options to discard the program’s stdout/stderr or to redirect them to files
### DONE: Stop the main program
### DONE: See the status of all the programs described in the config file ("status" command)
### DONE: nrestart
### DONE: start path
### DONE: start umask single function call ?->umask()
## TEST: restart n times from monitorProcess
