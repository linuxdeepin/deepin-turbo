import os, os.path, glob
import subprocess
import commands
import time
import sys
import re
import signal
import unittest
import types
from subprocess import Popen
from os.path import basename

DEV_NULL = file("/dev/null","w")
DAEMONS_TO_BE_STOPPED = ['xsession/applifed','xsession/conndlgs']
LAUNCHER_BINARY='/usr/bin/applauncherd'
DEV_NULL = file("/dev/null","w")
LAUNCHABLE_APPS = ['/usr/bin/fala_ft_hello','/usr/bin/fala_ft_hello1', '/usr/bin/fala_ft_hello2']
LAUNCHABLE_APPS_QML = ['/usr/bin/fala_qml_helloworld','/usr/bin/fala_qml_helloworld1', '/usr/bin/fala_qml_helloworld2']
PREFERED_APP = '/usr/bin/fala_ft_hello'
PREFERED_APP_QML = '/usr/bin/fala_qml_helloworld'
GET_COORDINATE_SCRIPT = '/usr/share/applauncherd-testscripts/get-coordinates.rb'
PIXELCHANHED_BINARY = '/usr/bin/fala_pixelchanged'


class CustomTestCase(unittest.TestCase) :
    def waitForAsert(self, periodicCheck, msg="", timeout=20, sleep=1) :
        assert(type(periodicCheck) == types.FunctionType or
               type(periodicCheck) == types.LambdaType)
        start = time.time()
        stop = start + timeout

        debug("Waiting for '%s' to return True value in time of %.1fs." %(periodicCheck.__name__, timeout))
        while stop > time.time() :
            if periodicCheck() :
                debug("'%s' has returned True after %.1fs." %(periodicCheck.__name__, time.time()-start))
                return
            time.sleep(sleep)
        debug("waitForAsert has timed out after %ss." %timeout)
        self.assert_(periodicCheck(), msg)
        return

    def waitForAsertEqual(self, periodicCheck, expectedValue, msg="", timeout=20, sleep=1) :
        assert(type(periodicCheck) == types.FunctionType or
               type(periodicCheck) == types.LambdaType)
        start = time.time()
        stop = start + timeout
        debug("Waiting for '%s' to return expected value: '%s' in time of %.1fs."
              %(periodicCheck.__name__, expectedValue, timeout))

        while stop > time.time() :
            if periodicCheck() == expectedValue :
                debug("'%s' has returned expected value: '%s' after %.1fs."
                      %(periodicCheck.__name__, expectedValue, time.time()-start))
                return
            time.sleep(sleep)
        debug("waitForAsertEqual has timed out after %ss." %timeout)
        value = periodicCheck()
        self.assertEqual(value, expectedValue, "%s\n"
                                               "Values are different, have value: %s\n"
                                               "               expected value is: %s\n"
                                               %(msg, value, expectedValue))
        return

    def waitForAssertLogFileContains(self, fileName, expContent, msg="", findCount = 1, timeout=20, sleep=1) :
        start = time.time()
        stop = start + timeout

        file = None
        # wait until file exists:
        while stop > time.time() :
            try :
                file = open(fileName, "r")
                break
            except IOError:
                time.sleep(sleep)
        self.assert_(file, "Failed to open log file: '%s' in time of %.1f" %(fileName, timeout))

        try :
            if sleep < time.time()-start :
                debug("File '%s' was opened after a period of: %.1fs." %(fileName, time.time() - start))

            debug("File '%s' is opened. Waiting for '%s' to appear in file in time of %.1fs."
                  %(fileName, expContent, timeout))
            while stop > time.time() :
                line = file.readline()
                while line :
                    if expContent in line :
                        findCount = findCount - 1
                        if findCount<=0 :
                            line = line[:-1] #remove tailing end line character
                            debug("String: '%s' has been found inside file: %s after %.1fs.\nDetected line contains: '%s'"
                                  %(expContent, fileName, time.time()-start, line))
                            return line
                    line = file.readline()
                time.sleep(sleep)

        finally :
            file.close()

        debug("waitForAsert has timed out after %ss." %timeout)
        self.assert_(False, "Content '%s' was not found in log file '%s'. %s"
                            %(expContent, fileName, msg))
        return


# Function to stop desired daemons. This is also done in setup function
# if stop_daemons is not called before.
def stop_daemons():
    for daemon in DAEMONS_TO_BE_STOPPED:
        os.system('initctl stop %s'%(daemon))
    wait_for_single_applauncherd()
    get_booster_pid()

# Function to start desired daemons. This is also done in teardown function
# if start_daemons is not called before.
def start_daemons():
    for daemon in DAEMONS_TO_BE_STOPPED:
        os.system('initctl start %s'%(daemon))
    wait_for_single_applauncherd()
    get_booster_pid()

def daemons_running():
     st, op = commands.getstatusoutput('pgrep %s'%DAEMONS_TO_BE_STOPPED[0].split("/")[1])        
     return not(st)

def debug(*msg):
    """
    Debug function
    """
    sys.stderr.write('[DEBUG %s] %s\n' % (time.ctime(), \
            ' '.join([str(s) for s in msg]),))

def error(*msg):
    """
    exit when error, give proper log 
    """
    sys.stderr.write('ERROR %s\n' % (' '.join([str(s) for s in msg]),))
    sys.exit(1)

def remove_applauncherd_runtime_files():
    """
    Removes files that applauncherd leaves behind after it has been stopped
    """
    debug("Removing files that applauncherd leaves behind after it has been stopped")
    files = glob.glob('/tmp/boost*')

    for f in files:
        debug("removing %s" % f)

        try:
            os.remove(f)
        except:
            pass

def start_applauncherd():
    debug("Starting applauncherd")
    handle = Popen(['initctl', 'start', 'xsession/applauncherd'],
                   stdout = DEV_NULL, stderr = DEV_NULL,
                   shell = False, preexec_fn=permit_sigpipe)
    get_booster_pid()
    return handle.wait() == 0

def stop_applauncherd():
    debug("Stoping applauncherd")
    handle = Popen(['initctl', 'stop', 'xsession/applauncherd'],
                   stdout = DEV_NULL, stderr = DEV_NULL,
                   shell = False, preexec_fn=permit_sigpipe)

    time.sleep(1)

    remove_applauncherd_runtime_files()

    return handle.wait()

def restart_applauncherd():
    debug("Restart applauncherd")
    stop_applauncherd()
    start_applauncherd()

def run_app_as_user_with_invoker(appname, booster = 'm', arg = "", out = DEV_NULL, err = DEV_NULL):
    """
    Runs the specified app as a user.
    """
    inv_cmd = "/usr/bin/invoker --type=%s %s %s" %(booster,arg, appname)
    debug("run %s as user" %inv_cmd)
    cmd = ['su', '-', 'user', '-c']
    if type(appname) == list:
        cmd += inv_cmd
    elif type(appname) == str:
        cmd.append(inv_cmd)
    else:
        raise TypeError("List or string expected")
    p = subprocess.Popen(cmd, shell = False, 
            stdout = out, stderr = err, preexec_fn=permit_sigpipe)
    return p

def run_cmd_as_user(cmnd, out = DEV_NULL, err = DEV_NULL):
    """
    Runs the specified command as a user.
    """
    debug("run %s as user" %cmnd)
    cmd = ['su', '-', 'user', '-c']
    if type(cmnd) == list:
        cmd += cmnd 
    elif type(cmnd) == str:
        cmd.append(cmnd)
    else:
        raise TypeError("List or string expected")
    p = subprocess.Popen(cmd, shell = False, 
            stdout = out, stderr = err, preexec_fn=permit_sigpipe)
    return p

def get_pid(appname, printdebug=True):
    temp = basename(appname)[:14]
    st, op = commands.getstatusoutput("pgrep %s" % temp)
    if(printdebug):
        debug("The Pid of %s is %s" %(appname, op))
    if st == 0:
        return op
    else:
        return None

def get_oldest_pid(appname):
    temp = basename(appname)[:14]
    st, op = commands.getstatusoutput("pgrep -o %s" % temp)
    debug("The Pid of %s is %s" %(appname, op))
    if st == 0:
        return op
    else:
        return None
    
def get_newest_pid(app):
    p = subprocess.Popen(['pgrep', '-n', app], shell = False,
                         stdout = subprocess.PIPE, stderr = DEV_NULL, preexec_fn=permit_sigpipe)

    op = p.communicate()[0]

    debug("The New Pid of %s is %s" %(app, op.strip()))
    if p.wait() == 0:
        return op.strip()
    
    return None

def wait_for_app(app = None, timeout = 40, sleep = 1):
    """
    Waits for an application to start. Checks periodically if
    the app is running for a maximum wait set in timeout.

    Returns the pid of the application if it was running before
    the timeout finished, otherwise None is returned.
    """

    pid = None
    start = time.time()

    debug("Waiting for '%s' to startup in %.1fs time" %(app, timeout))
    while pid == None and time.time() < start + timeout:
        p = subprocess.Popen(['pgrep', '-n', app], shell = False,
                         stdout = subprocess.PIPE, stderr = DEV_NULL, preexec_fn=permit_sigpipe)
        op = p.communicate()[0]
        if p.wait() == 0:
            pid = op.strip()
            break

        time.sleep(sleep)

    if (pid==None):
        debug("Failed to fetch PID for '%s' in %ss time" %(app, timeout))
    else:
        debug("Application '%s' has started with PID: %s in time of %.1fs."  %(app, pid, time.time()-start))
    
    return pid

def wait_for_single_applauncherd(timeout = 20, sleep = 1):
    pid = get_pid('applauncherd', False)
    count = len(pid.split("\n"))
    start = time.time()

    while count > 1 and time.time() < start + timeout:
        time.sleep(sleep)
        pid = get_pid('applauncherd', False)
        count = len(pid.split("\n"))
        if count == 1:
            break
    debug("got single applauncherd pid %s in %.1fs" % (pid, time.time()-start))
    return pid

def get_booster_pid(timeout = 40):
    boosters = ('e', 'd', 'q', 'm')
    pids = [None, None, None, None]
    start = time.time()
    debug("Waiting for all booster to appear in time of %.1fs" %(timeout))
    while time.time()<start + timeout :
        p = subprocess.Popen(['pgrep', '-l', r"\bbooster-[mdqe]\b"], shell = False,
                         stdout = subprocess.PIPE, stderr = DEV_NULL)
        op = p.communicate()[0]
        if p.wait() == 0:
            pidInfos = op.splitlines()
            for pidInfo in pidInfos :
                if pidInfo :
                    index = boosters.index(pidInfo[-1])
                    boosterData = pidInfo.split()
                    if pids[index] == None :
                        debug("%s has been found with PID: %s" %(boosterData[1], boosterData[0]))
                    pids[index] = boosterData[0]
            if all(pids) :
                debug("All boosters has been found after %.1fs." %(time.time()-start))
                return pids

        time.sleep(1)

    debug("Not all boosters has been found: %s (order: %s)" %(pids, boosters))
    return pids

def kill_process(appname=None, apppid=None, signum=15):
    # obtained by running 'kill -l'
    signal = {1: "HUP", 2: "INT", 3: "QUIT", 4: "ILL", 5: "TRAP", 6: "ABRT", 7: "BUS", 
              8: "FPE", 9: "KILL", 10: "USR1", 11: "SEGV", 12: "USR2", 13: "PIPE", 
              14: "ALRM", 15: "TERM", 16: "STKFLT", 17: "CHLD", 18: "CONT", 19: "STOP", 
              20: "TSTP", 21: "TTIN", 22: "TTOU", 23: "URG", 24: "XCPU", 25: "XFSZ", 
              26: "VTALRM", 27: "PROF", 28: "WINCH", 29: "POLL", 30: "PWR", 31: "SYS"}
    if signal.has_key(signum):
        signame = "SIG%s" % signal[signum]
    else:
        # should never reach this
        signame = "signal %s" % signum

    if apppid and appname: 
        return None
    else:
        if apppid:
            debug("Now sending %s to the app with pid %s" % (signame, apppid))
            st, op = commands.getstatusoutput("kill -%s %s" % (str(signum), str(apppid)))
        if appname: 
            debug("Now sending %s to %s" % (signame, appname))
            temp = basename(appname)[:14]
            st, op = commands.getstatusoutput("pkill -%s %s" % (str(signum), temp))

def wait_for_process_end(appName = None, appPid = None, timeout = 10, sleep=1) :
    if appName :
        debug("Waiting for termination of '%s' application (time out %.1f)" %(appName, timeout))
        startTime = time.time()
        endTime = startTime + timeout
        op = ""
        while time.time()<endTime :
            st, op = commands.getstatusoutput("pgrep -l %s" %appName)
            if st!=0 :
                debug("Application '%s' has been terminated after %.1fs" %(appName, time.time() - startTime))
                return True
            time.sleep(sleep)
        debug("Application '%s' was not finished/terminated after %.1fs.\npgrep outcome:\n%s" %(appName, timeout, op))
        return False

    if appPid:
        debug("Waiting for termination of process PID=%s (time out %.1f)" %(appPid, timeout))
        startTime = time.time()
        endTime = startTime + timeout
        while time.time()<endTime :
            if not os.path.exists("/proc/%s/cmdline" %appPid) :
                debug("Process with PID=%s has been terminated after %.1fs" %(appPid, time.time() - startTime))
                return True
            time.sleep(sleep)
        debug("Process with PID=%s was not finished/terminated after %.1fs" %(appPid, timeout))
        return False

    raise TypeError("Application name or process PID is expected.")
    
def process_state(processid):
    st, op = commands.getstatusoutput('cat /proc/%s/stat' %processid)
    debug("The Process State of %s is %s" %(processid, op))
    if st == 0:
        return op
    else:
        debug(op)
        return None

def get_creds(path = None, pid = None):
    """
    Tries to retrieve credentials for a running application
    using either the pid or path. Credentials are returned
    as a string list.
    """

    if path != None:
        pid = get_pid(path)

    if pid == None:
        print 'invalid PID'
        return None

    handle = Popen(['/usr/bin/creds-get', '-p', str(pid)],
                   stdout = subprocess.PIPE, preexec_fn=permit_sigpipe)

    op = handle.communicate()[0].strip()
    handle.wait()

    if handle.returncode != 0:
        print 'error retrieving credentials'
        return None
    
    #self.assert_(handle.returncode == 0, "There was no such PID!")
    debug("creds-get gave >>>>>\n%s\n<<<<<" % op)

    creds = op.split("\n")[1:]

    return creds

def launch_and_get_creds(path):
    """
    Tries to launch an application and if successful, returns the
    credentials the application has as a list. 
    """

    # try launch the specified application
    handle = run_app_as_user_with_invoker(path,arg = '--no-wait')

    # sleep for a moment to allow applauncherd to start the process
    time.sleep(3)

    # with luck, the process should have correct name by now
    pid = get_pid(path)

    debug("%s has PID %s" % (basename(path), pid,))

    if pid == None:
        print "couldn't launch %s" % basename(path)
        return None

    creds = get_creds(pid = pid)

    kill_process(path)

    return creds

# returns the fd list of a process as a dict
def get_fd_dict(pid):
    fd_dict = {}
    fd_info = commands.getoutput('ls -l /proc/%s/fd/' % pid).splitlines()
    for fd in fd_info:
        if "->" in fd:
            fd_dict[fd.split(" -> ")[0].split(' ')[-1]] = fd.split(" -> ")[-1]
    return fd_dict
        
def get_groups_for_user():
    # get supplementary groups user belongs to (doesn't return
    # the gid group)
    p = run_cmd_as_user('id -Gn', out = subprocess.PIPE)
    groups = p.communicate()[0].split()
    debug("The groups for users is :%s" %groups)
    p.wait()
    
    return groups

#checks if there is a change in booster pids until 5 seconds
# the param must be a string - output of get_pid('booster')
def wait_for_new_boosters(old_booster_pids):
    new_booster_pids = old_booster_pids
    for count in range(4):
        new_booster_pids = get_pid('booster')
        if(old_booster_pids != new_booster_pids):
            break
        time.sleep(1)
    return new_booster_pids

def send_sighup_to_applauncherd():
    wait_for_single_applauncherd()
    (e1, d1, q1, m1) = get_booster_pid()
    launcher_pid1 = get_oldest_pid('applauncherd')
    debug("before sighup, applauncherd pid = ", launcher_pid1)

    boosterpids1 = get_pid('booster') # get the list of booster pids b4 sighup
    kill_process(None, launcher_pid1, 1) #sending sighup to applauncherd
    wait_for_new_boosters(boosterpids1) # give sometime for applauncherd to react

    wait_for_single_applauncherd()
    (e2, d2, q2, m2) = get_booster_pid()
    launcher_pid2 = get_oldest_pid('applauncherd')
    debug("after sighup, applauncherd pid = ", launcher_pid2)

    #check if applauncherd has same pid before and after sighup
    #check if all boosters have different pids before and after sighup
    return (launcher_pid1==launcher_pid2, m1!=m2 and q1!=q2 and d1!=d2 and e1!=e2)

def wait_for_windows(windowName, minCount=1, timeout=20) :
    """
    Waits 'timeout' seconds of time until at least minCount windows with windowName appears.
    Returns ids of those windows in list.
    """
    debug("Searching for window with name %s (with timeout %ss)" %(windowName, timeout))
    xwininfocommand = "xwininfo -root -tree | awk '/%s/ {print $1}'" %(windowName)

    start = time.time()
    while time.time() < start + timeout :
        st, op = commands.getstatusoutput(xwininfocommand)
        if op :
            op = op.split()
            if (len(op)>=minCount) :
                break
        time.sleep(1)

    if op :
        debug("Window '%s' has been found in time of %.1fs" %(op, time.time()-start))
    else :
        debug("Command '%s' didn't detect any windows with name %s. Timeout in %ss!" %(xwininfocommand, windowName, timeout))
        op = None

    return op

def isPackageInstalled(packageName):
    st, op = commands.getstatusoutput("dpkg -l %s" % packageName)
    if(st == 0): # success, check if version is "<none>" or a valid version
        m = re.search('.*%s\s*(\S*)\s*.*'%packageName, op)
        if(m and m.group(1).find("none")==-1): #m.group(1) contains version
            return True
    return False

#trick to avoid broken pipe in Popen
#use ,preexec_fn=permit_sigpipe in each Popen
def permit_sigpipe():
        signal.signal(signal.SIGPIPE, signal.SIG_DFL)
