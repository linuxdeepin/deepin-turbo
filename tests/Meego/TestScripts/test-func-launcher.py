#!/usr/bin/env python
#
# Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# Contact: Nokia Corporation (directui@nokia.com)
#
# This file is part of applauncherd.
#
# If you have questions regarding the use of this file, please contact
# Nokia at directui@nokia.com.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation
# and appearing in the file LICENSE.LGPL included in the packaging
# of this file.

"""
This program tests the startup time of the given application with and
without launcher.

Requirements:
1. DISPLAY environment variable must be set correctly.
2. DBus session bus must be running.
3. DBus session bus address must be stored in /tmp/session_bus_address.user.
4. Given application supports launcher with .launcher binary in /usr/bin/.
5. launcher application should be installed.

Usage:    test-func-launcher <launcherable application>

Example:  test-func-launcher /usr/bin/fala_ft_hello

Authors:   ext-nimika.1.keshri@nokia.com
           ext-oskari.timperi@nokia.com
"""

import os, os.path, glob
import subprocess
import commands
import time
import sys
import unittest
import re
from subprocess import Popen
from utils import *
from os.path import basename

LAUNCHER_BINARY='/usr/bin/applauncherd'
DEV_NULL = file("/dev/null","w")
LAUNCHABLE_APPS = ['/usr/bin/fala_ft_hello','/usr/bin/fala_ft_hello1', '/usr/bin/fala_ft_hello2']
PREFERED_APP = '/usr/bin/fala_ft_hello'

using_scratchbox = False

def start_launcher_daemon():
    temp = basename(LAUNCHER_BINARY)
    st, op = commands.getstatusoutput("pgrep %s" %temp)
    if st == 0:
        debug("Launcher already started")
        return op

def check_prerequisites():
    if os.getenv('DISPLAY') == None:
        error("DISPLAY is not set. Check the requirements.")
        
    if os.getenv('DBUS_SESSION_BUS_ADDRESS') == None:
        error("DBUS_SESSION_BUS_ADDRESS is not set.\n" +
              "You probably want to source /tmp/session_bus_address.user")

    for app in LAUNCHABLE_APPS: 
        assert(len(basename(app)) < 15, "For app: %s, base name !<= 14" % app)

class launcher_tests (unittest.TestCase):
    def setUp(self):
        #setup here
        print "Executing SetUp"

    def tearDown(self):
        #teardown here
        print "Executing TearDown"
        if get_pid('applauncherd') == None:
            os.system('initctl start xsession/applauncherd')

    #Testcases
    def test_001_launcher_exist(self):
        """
        To test if the launcher exists and is executable or not
        """
        self.assert_(os.path.isfile(LAUNCHER_BINARY), "Launcher file does not exist")
        self.assert_(os.access(LAUNCHER_BINARY, os.X_OK), "Launcher exists, but is not executable")

    def test_002_applications_exist(self):
        """
        test_launchable_application_exists
        """
        failed_apps = []
        for app in LAUNCHABLE_APPS: 
            temp = "%s.launch" % app
            if not (os.path.isfile(temp) and os.access(temp, os.X_OK)): 
                failed_apps.append(temp)
        self.assert_(failed_apps == [], "Some applications do not have the launch files, list: %s" % str(failed_apps))

    def test_003_zombie_state(self):
        """
        To test that no Zombie process exist after the application is killed
        """
        #launch application with launcher
        #check if the application is running
        #kill the application (pid = p.pid)
        #check if pgrep appname should be nothing
        #self.kill_process(LAUNCHER_BINARY)

        process_handle = run_app_as_user(PREFERED_APP)
        process_id = wait_for_app(PREFERED_APP, 5)
        print process_id
        kill_process(PREFERED_APP)
        time.sleep(4)

        process_handle = run_app_as_user(PREFERED_APP)
        process_id1 = wait_for_app(PREFERED_APP, 5)
        print process_id1
        kill_process(PREFERED_APP)
        time.sleep(4)

        process_id1 = get_pid(PREFERED_APP)
        print process_id1

        self.assert_(process_id != process_id1 , "New Process not launched")
        self.assert_(process_id1 == None , "Process still running")
    
    def test_004_launch_multiple_apps(self):
        """
        To test that more than one applications are launched by the launcher 
        """

        def kill_launched(pids):
            for pid in pids:
                kill_process(apppid = pid)

        pidlist = []

        for app in LAUNCHABLE_APPS:
            p = run_app_as_user(app)
            pid = wait_for_app(app, timeout = 10, sleep = 1)

            if pid == None:
                kill_launched(pidlist)
                self.fail("%s was not launched using applauncherd")

            pidlist.append(pid)

        kill_launched(pidlist)
    
    def test_005_one_instance(self):
        """
        To test that only one instance of a application exist 
        """
        #launch application
        #self.run_app_as_user(appname)
        #get pid of application
        #launch applicatoin again
        #check pgrep application
        #y = commands.getstatusoutput(pgrep appname)
        #len(y[-1].split(' ')) == 1
        process_handle = run_app_as_user(PREFERED_APP)
        process_id = wait_for_app(PREFERED_APP)
        debug("PID of first %s" % process_id)

        process_handle1 = run_app_as_user(PREFERED_APP)
        time.sleep(2)
        process_id = wait_for_app(PREFERED_APP)
        debug("PID of 2nd %s" % process_id)

        kill_process(PREFERED_APP)

        self.assert_( len(process_id.split(' ')) == 1, "Only one instance of app not running")
        
    def test_009_launch_multiple_apps_cont(self):
        """
        To test that more than one applications are launched by the launcher 
        """
        for app in LAUNCHABLE_APPS: 
            #launch application with launcher
            #check if the application is running
            #check if p.pid is same as pgrep appname
            #in a global dictionary, append the pid
            process_handle = run_app_as_user(app)

        time.sleep(8)

        process_id = get_pid('fala_ft_hello')
        pid_list = process_id.split()

        self.assert_(len(pid_list) == len(LAUNCHABLE_APPS), "All Applications were not launched using launcher")

        for pid in pid_list:
            kill_process(apppid=pid)

    def test_010(self):
        """
        NB#179266

        When calling invoker with --wait-term and killing invoker,
        the launched application should die too.
        """

        app_path = '/usr/bin/fala_ft_hello.launch'

        # Launch the app with invoker using --wait-term
        p = run_app_as_user('invoker --type=m --wait-term %s' % app_path)

        time.sleep(2)

        # Retrieve their pids
        invoker_pid = wait_for_app('invoker')
        app_pid = wait_for_app('fala_ft_hello')

        print "invoker_pid '%s'" % invoker_pid
        print "app_pid '%s'" % app_pid

        # Make sure that both apps started
        self.assert_(invoker_pid != None, "invoker not executed?")
        self.assert_(app_pid != None, "%s not launched by invoker?" % app_path)

        # Send SIGTERM to invoker, the launched app should die
        kill_process(None, invoker_pid, 15)

        time.sleep(2)

        # This should be None
        app_pid2 = get_pid('fala_ft_hello')

        if (app_pid2 != None):
            kill_process(None, app_pid2)
            self.assert_(False, "%s was not killed" % app_path)


    def test_011(self):
        """
        Test that the --daemon parameter works for applauncherd
        """

        stop_applauncherd()

        # and for the fun of it let's do it again
        commands.getstatusoutput("pkill applauncherd")

        remove_applauncherd_runtime_files()

        p = run_app_as_user('/usr/bin/applauncherd.bin --daemon')

        time.sleep(5)

        st, op = commands.getstatusoutput('pgrep -lf "applauncherd.bin --daemon"')
        print op

        # filter some cruft out from the output and see how many
        # instances are running
        op = filter(lambda x: x.find("sh ") == -1, op.split("\n"))
        count = len(op)

        print "count = %d" % count

        self.assert_(count == 1, "applauncherd was not daemonized (or too many instances running ..)")

        # try to launch an app
        run_app_as_user('/usr/bin/fala_ft_hello')
        time.sleep(2)

        pid = wait_for_app('fala_ft_hello')

        if pid != None:
            kill_process(apppid = pid)
        else:
            self.assert_(False, "fala_ft_hello was not launched!")

        # only the daemonized applauncherd should be running now
        commands.getstatusoutput('pkill applauncherd')

        remove_applauncherd_runtime_files()

        start_applauncherd()

    def test_012(self):
        """
        Test the --delay parameter of the invoker.
        """

        # launch an app with invoker --delay n
        print "launching fala_ft_hello ..."
        p = Popen(['/usr/bin/invoker', '--delay', '10', '--type=m', '--no-wait',
                   '/usr/bin/fala_ft_hello.launch'],
                  shell=False, 
                  stdout=DEV_NULL, stderr=DEV_NULL)

        # wait a little
        print "waiting ..."
        time.sleep(5)

        success = True

        if p.poll() == None:
            print "NOT DEAD"
        else:
            print "DEAD"
            success = False

        print "waiting for invoker to terminate ..."
        p.wait()

        print "terminating fala_ft_hello ..."
        Popen(['pkill', 'fala_ft_hello']).wait()

        self.assert_(success, "invoker terminated before delay elapsed")

    def test_013_applauncherd_usage(self):
        """
        Test applauncherd.bin help
        """
        st, op = commands.getstatusoutput("applauncherd.bin --help")
        self.assert_(st == 0, "Usage not printed")
        str = op.split('\n')
        self.assert_(str[0] == 'Usage: applauncherd [options]', "usage not printed properly")

        # exec invoker --creds (for coverage)
        run_app_as_user("invoker --creds")

        # exec invoker --help (for coverage)
        run_app_as_user("invoker --help")


    def test_014_fd_booster_m(self):
        """
        File descriptor test for booster-m
        """
        count = get_file_descriptor("booster-m","m")
        self.assert_(count != 0, "None of the file descriptors were changed")

    def test_015_fd_booster_q(self):
        """
        File descriptor test for booster-q
        """
        count = get_file_descriptor("booster-q","qt")
        self.assert_(count != 0, "None of the file descriptors were changed")

    def test_016_restart_booster(self):
        """
        Test that booster is restarted if it is killed 
        """
        #get the pids of boosters and make sure they are running
        qpid = get_pid('booster-q')
        print "Pid of booster-q before killing :%s" %qpid
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        print "Pid of booster-m before killing :%s" %mpid
        self.assert_(mpid != None, "No booster process running")

        #Kill the booster processes
        kill_process(apppid=qpid)
        kill_process(apppid=mpid)
        
        #wait for the boosters to be restarted
        time.sleep(6)

        #check that the new boosters are started
        qpid_new = get_pid('booster-q')
        print "Pid of booster-q after killing :%s" %qpid_new
        self.assert_(qpid_new != None, "No booster process running")
        self.assert_(qpid_new != qpid, "booster process was not killed")

        mpid_new = get_pid('booster-m')
        print "Pid of booster-m after killing :%s" %mpid_new
        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")

    def test_017_invoker_exit_status(self):
        """
        To test that invoker returns the same exit status as the application
        """
        #Run application without invoker and get the exit status
        st, op = commands.getstatusoutput('/usr/bin/fala_status.launch')
        app_st_wo_inv = os.WEXITSTATUS(st)
    
        #Run application with invoker and get the exit status
        st, op = commands.getstatusoutput('invoker --type=m --wait-term /usr/bin/fala_status.launch')
        app_st_w_inv = os.WEXITSTATUS(st)

        self.assert_(app_st_wo_inv == app_st_w_inv, "The invoker returns a wrong exit status")

    def test_018_invoker_gid_uid(self):
        """
        To Test that the set gid and uid is passed from invoker process to launcher
        """
        #get the id in user mode 
        print ("In User Mode \n")
        st, op =  commands.getstatusoutput('su user -c ' "id")
        usr_id1 = op.split(' ')[0].split('(')[0]
        grp_id1 = op.split(' ')[1].split('(')[0]
        print("System %s \nSyetem %s" %(usr_id1, grp_id1))

        #get id by running the application using invoker in user mode
        app = "invoker --type=m --no-wait /usr/bin/fala_status.launch" 
        st, op = commands.getstatusoutput('su user -c "%s"' %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        print("Invoker %s \nInvoker %s" %(usr_id, grp_id))
        
        #get id by running the application without invoker in user mode
        app = "/usr/bin/fala_status.launch"
        st, op = commands.getstatusoutput('su user -c "%s"' %app );
        usr_id2 = op.split('\n')[-2]
        grp_id2 = op.split('\n')[-1]
        print("Application %s \nApplication %s" %(usr_id2, grp_id2))

        self.assert_(usr_id == usr_id1, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id1, "The correct GID is not passed by invoker")

        self.assert_(usr_id == usr_id2, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id2, "The correct GID is not passed by invoker")

        #get the id in root mode 
        print ("In Root Mode \n")
        st, op =  commands.getstatusoutput("id")
        usr_id1 = op.split(' ')[0].split('(')[0]
        grp_id1 = op.split(' ')[1].split('(')[0]
        print("System %s \nSyetem %s" %(usr_id1, grp_id1))

        #get id by running the application using invoker in root mode
        app = "invoker --type=m --no-wait /usr/bin/fala_status.launch" 
        st, op = commands.getstatusoutput("%s" %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        print("Invoker %s \nInvoker %s" %(usr_id, grp_id))
        
        #get id by running the application without invoker in root mode
        app = "/usr/bin/fala_status.launch"
        st, op = commands.getstatusoutput("%s" %app );
        usr_id2 = op.split('\n')[-2]
        grp_id2 = op.split('\n')[-1]
        print("Application %s \nApplication %s" %(usr_id2, grp_id2))

        self.assert_(usr_id == usr_id1, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id1, "The correct GID is not passed by invoker")

        self.assert_(usr_id == usr_id2, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id2, "The correct GID is not passed by invoker")
       

    def test_019_signal_forwarding(self):
        """
        To test that invoker is killed by the same signal as the application
        """
        #Test for m-booster
        st, op = commands.getstatusoutput("/usr/share/applauncherd-M-testscripts/signal-forward/fala_sf_m.py")
        print ("The Invoker killed by : %s" %op)
    
        self.assert_(op == 'Segmentation fault (core dumped)', "The invoker(m-booster) was not killed by the same signal")
        time.sleep(2)
         
        #This case is launching the application in user mode
        #Test for q-booster
        st, op = commands.getstatusoutput("/usr/share/applauncherd-M-testscripts/signal-forward/fala_sf_qt.py")
        print ("The Invoker killed by : %s" %op.split('\n')[-1])
    
        self.assert_(op.split('\n')[-1] == 'Aborted (core dumped)', "The invoker(q-booster) was not killed by the same signal")
        time.sleep(2)

        #Test for w-booster
        st, op = commands.getstatusoutput("/usr/share/applauncherd-M-testscripts/signal-forward/fala_sf_wrt.py")
        print ("The Invoker killed by : %s" %op)
    
        self.assert_(op == 'User defined signal 1', "The invoker(w-booster) was not killed by the same signal")
        time.sleep(2)


    def test_020_launch_wo_applauncherd(self):
        """
        To Test that invoker can launch applications even when the
        applauncherd is not running
        """

        stop_applauncherd()

        handle = run_app_as_user('fala_ft_hello')
        time.sleep(3)
        pid1 = get_pid('fala_ft_hello')
        
        self.assert_(pid1 != None, "Application wasn't executed")

        kill_process('fala_ft_hello')

        start_applauncherd()

    def test_invoker_search_prog(self):
        """
        Test that invoker can find programs from directories listed in
        PATH environment variable and that it doesn't find something
        that isn't there.
        """

        # invoker searches PATH for the executable
        p = run_app_as_user('invoker --type=m --no-wait fala_ft_hello.launch')
        self.assert_(p.wait() == 0, "Couldn't launch fala_ft_hello.launch")
        kill_process('fala_ft_hello')

        # launch with relative path
        cwd = os.getcwd()
        os.chdir('/usr/share')

        p = run_app_as_user('invoker --type=m --no-wait ' + 
                            "../bin/fala_ft_hello.launch")
        self.assert_(p.wait() == 0, "Couldnt launch fala_ft_hello.launch" + 
                    " with relative path")
        kill_process('fala_ft_hello')

        # find a relative path from PATH and launch from there
        oldpath = os.environ['PATH']
        os.environ['PATH'] =  '../bin:' + os.environ['PATH']

        p = run_app_as_user('invoker --type=m --no-wait ' +
                            "fala_ft_hello.launch")
        self.assert_(p.wait() == 0, "Couldnt launch fala_ft_hello.launch" +
                    " with relative path (2)")
        kill_process('fala_ft_hello')
        
        # restore CWD and PATH
        os.chdir(cwd)
        os.environ['PATH'] = oldpath

        # and finally, try to launch something that doesn't exist
        p = run_app_as_user('invoker --type=m --no-wait spam_cake.launch')
        self.assert_(p.wait() != 0, "Found spam_cake.launch for some reason")
        kill_process('spam_cake')
    
    def test_booster_killed_or_restarted(self):
        """
        Test that boosters are killed if applauncherd is stopped
        and restarted if applauncherd is killed
        """
        #get the pids of boosters and make sure they are running
        qpid = get_pid('booster-q')
        print "Pid of booster-q before killing :%s" %qpid
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        print "Pid of booster-m before killing :%s" %mpid
        self.assert_(mpid != None, "No booster process running")

        wpid = get_pid('booster-w')
        print "Pid of booster-w before killing :%s" %mpid
        self.assert_(wpid != None, "No booster process running")

        #stop applauncherd
        os.system("initctl stop xsession/applauncherd")
 
        #wait for the boosters to be killed 
        time.sleep(2)

        #check that the none of the booster is running
        qpid_new = get_pid('booster-q')
        print "Pid of booster-q after killing :%s" %qpid_new
        self.assert_(qpid_new == None, "booster-q still running")
        
               
        mpid_new = get_pid('booster-m')
        print "Pid of booster-m after killing :%s" %mpid_new
        self.assert_(mpid_new == None, "booster-m still running")

        wpid_new = get_pid('booster-w')
        print "Pid of booster-w after killing :%s" %wpid_new
        self.assert_(wpid_new == None, "booster-w still running")

        #Now start the applauncherd
        os.system("initctl start xsession/applauncherd")
        
        #wait for the boosters to be restarted
        time.sleep(6)

        #get the pids of boosters and make sure they are running
        qpid = get_pid('booster-q')
        print "Pid of booster-q before killing :%s" %qpid
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        print "Pid of booster-m before killing :%s" %mpid
        self.assert_(mpid != None, "No booster process running")

        wpid = get_pid('booster-w')
        print "Pid of booster-w before killing :%s" %mpid
        self.assert_(wpid != None, "No booster process running")

        #Now kill applauncherd
        kill_process('applauncherd')
        
        #wait for the boosters to be restarted
        time.sleep(6)
        
        #check that the new boosters are started
        qpid_new = get_pid('booster-q')
        print "Pid of booster-q after killing :%s" %qpid_new
        self.assert_(qpid_new != None, "No booster process running")
        self.assert_(qpid_new != qpid, "booster process was not killed")

        mpid_new = get_pid('booster-m')
        print "Pid of booster-m after killing :%s" %mpid_new
        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")
            
        wpid_new = get_pid('booster-w')
        print "Pid of booster-w after killing :%s" %wpid_new
        self.assert_(wpid_new != None, "No booster process running")
        self.assert_(wpid_new != wpid, "booster process was not killed")


# main
if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    check_prerequisites()
    start_launcher_daemon()
    tests = sys.argv[1:]
    mysuite = unittest.TestSuite(map(launcher_tests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)
    if not result.wasSuccessful():
        sys.exit(1)
    sys.exit(0)
