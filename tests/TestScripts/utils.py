import os, os.path, glob
import subprocess
import commands
import time
import sys
import re
from subprocess import Popen
from os.path import basename

DEV_NULL = file("/dev/null","w")

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

def run_app_with_launcher(appname):
    p = subprocess.Popen(appname,
                         shell=False, 
                         stdout=DEV_NULL, stderr=DEV_NULL)
    return p

def get_pid(appname):
    temp = basename(appname)[:14]
    st, op = commands.getstatusoutput("pgrep %s" % temp)
    if st == 0:
        return op
    else:
        return None

def wait_for_app(app = None, timeout = 5, sleep = 0.5):
    """
    Waits for an application to start. Checks periodically if
    the app is running for a maximum wait set in timeout.

    Returns the pid of the application if it was running before
    the timeout finished, otherwise None is returned.
    """

    pid = None
    start = time.time()

    while pid == None and time.time() < start + timeout:
        pid = get_pid(app)

        if pid != None:
            break

        print "waiting %s secs for %s" % (sleep, app)

        time.sleep(sleep)

    return pid


def kill_process(appname=None, apppid=None, signum=9):
    if apppid and appname: 
        return None
    else:
        if apppid: 
            st, op = commands.getstatusoutput("kill -%s %s" % (str(signum), str(apppid)))
        if appname: 
            temp = basename(appname)[:14]
            st, op = commands.getstatusoutput("pkill -%s %s" % (str(signum), temp))

            try:
                os.wait()
            except Exception as e:
                print e

def process_state(processid):
    st, op = commands.getstatusoutput('cat /proc/%s/stat' %processid)
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
                   stdout = subprocess.PIPE)

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
    handle = run_app_with_launcher(path)

    # sleep for a moment to allow applauncherd to start the process
    time.sleep(5)

    # with luck, the process should have correct name by now
    pid = get_pid(path)

    debug("%s has PID %s" % (basename(path), pid,))

    if pid == None:
        print "couldn't launch %s" % basename(path)
        return None

    #self.assert_(pid != None, "Couldn't launch %s" % basename(path))

    creds = get_creds(pid = pid)

    kill_process(path)

    return creds

def get_file_descriptor(booster, type):
    """
    To test that file descriptors are closed before calling application main
    """
    #get fd of booster before launching application
    pid = commands.getoutput("pgrep %s" %booster)
    fd_info = commands.getoutput('ls -l /proc/%s/fd/' % str(pid))
    fd_info = fd_info.split('\n')
    init = {}
    final = {}

    for fd in fd_info:
        if "->" in fd:
            init[fd.split(" -> ")[0].split(' ')[-1]] = fd.split(" -> ")[-1]
    print init

    #launch application using booster
    st = os.system('invoker --type=%s /usr/bin/fala_ft_hello.launch' %type)
    time.sleep(2)

    #get fd of booster after launching the application
    if st == 0:
        fd_info = commands.getoutput('ls -l /proc/%s/fd/' % str(pid))
        fd_info = fd_info.split('\n')
        for fd in fd_info:
            if "->" in fd:
                final[fd.split(" -> ")[0].split(' ')[-1]] = fd.split(" -> ")[-1]
    print final
    pid = commands.getoutput('pgrep fala_ft_hello')    

    mykeys = init.keys()
    count = 0

    for key in mykeys:
        try:
            if init[key] != final[key]:
                count = count + 1
        except KeyError:
            print "some key in init is not in final" 
    time.sleep(2)
    print "The number of changed file descriptors %d" %count
    kill_process(apppid=pid) 
    return count
