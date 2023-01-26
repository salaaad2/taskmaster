#+TITLE: Taskmaster

# TODO:
## readline-cpp
## hot reload:
use GetYAMLValues( is_modified )
## killsig
sanity check:
- process not killed behaviour
- How long to wait after a graceful stop before killing the program
## additional/different env values
single function call ? execv() -> execve() + add new values to envp arg
## multiple acceptable return values

# In Progress
## TEST: The number of processes to start and keep running

# Done
###  How long the program should be running after it’s started for it to be considered successfully started"
###  The command to use to launch the program
###  Whether to start this program at launch or not
###  Which signal should be used to stop (i.e. exit gracefully) the program
###  Options to discard the program’s stdout/stderr or to redirect them to files
###  Stop the main program
###  See the status of all the programs described in the config file ("status" command)
###  nrestart
###  start path
###  start umask single function call ?->umask()
###  restart n times from monitorProcess
###  Whether the program should be restarted never, unexpected exit, or always (0, 1, 2)
