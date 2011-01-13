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
        assert len(basename(app)) <= 15, "For app: %s , base name !<= 14" %app

class launcher_tests (unittest.TestCase):
    def setUp(self):
        #setup here
        debug("Executing SetUp")

    def tearDown(self):
        #teardown here
        debug("Executing TearDown")
        if get_pid('applauncherd') == None:
            os.system('initctl start xsession/applauncherd')
        time.sleep(5)

    #Testcases
    def test_launcher_exist(self):
        """
        To test if the launcher exists and is executable or not
        """
        self.assert_(os.path.isfile(LAUNCHER_BINARY), "Launcher file does not exist")
        self.assert_(os.access(LAUNCHER_BINARY, os.X_OK), "Launcher exists, but is not executable")

    def test_applications_exist(self):
        """
        test_launchable_application_exists
        """
        failed_apps = []
        for app in LAUNCHABLE_APPS: 
            temp = "%s.launch" % app
            if not (os.path.isfile(temp) and os.access(temp, os.X_OK)): 
                failed_apps.append(temp)
        self.assert_(failed_apps == [], "Some applications do not have the launch files, list: %s" % str(failed_apps))

    def test_zombie_state(self):
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
        debug("The pid of %s id %s" %(PREFERED_APP, process_id))
        kill_process(PREFERED_APP)
        time.sleep(4)

        process_handle = run_app_as_user(PREFERED_APP)
        process_id1 = wait_for_app(PREFERED_APP, 5)
        debug("The pid of %s id %s" %(PREFERED_APP, process_id1))
        kill_process(PREFERED_APP)
        time.sleep(4)

        process_id1 = get_pid(PREFERED_APP)
        debug("The pid of %s id %s" %(PREFERED_APP, process_id1))

        self.assert_(process_id != process_id1 , "New Process not launched")
        self.assert_(process_id1 == None , "Process still running")
    
    def test_launch_multiple_apps(self):
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
    
    def test_one_instance(self):
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
        
    def test_launch_multiple_apps_cont(self):
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

    def test_wait_term(self):
        """
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

        # Make sure that both apps started
        self.assert_(invoker_pid != None, "invoker not executed?")
        self.assert_(app_pid != None, "%s not launched by invoker?" % app_path)

        # Send SIGTERM to invoker, the launched app should die
        debug(" Send SIGTERM to invoker, the launched app should die")
        kill_process(None, invoker_pid, 15)

        time.sleep(2)

        # This should be None
        app_pid2 = get_pid('fala_ft_hello')
        self.assert_(app_pid2 == None, "%s was not killed" % app_path)



    def test_daemon(self):
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
        debug("The pid of applauncherd --daemon is %s" %op)

        # filter some cruft out from the output and see how many
        # instances are running
        op = filter(lambda x: x.find("sh ") == -1, op.split("\n"))
        count = len(op)

        debug("count = %d" % count)

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

    def test_invoker_delay(self):
        """
        Test the --delay parameter of the invoker.
        """

        # launch an app with invoker --delay n
        debug("launching fala_ft_hello ...")
        p = Popen(['/usr/bin/invoker', '--delay', '10', '--type=m', '--no-wait',
                   '/usr/bin/fala_ft_hello.launch'],
                  shell=False, 
                  stdout=DEV_NULL, stderr=DEV_NULL)

        # wait a little
        debug("waiting ...")
        time.sleep(5)

        success = True

        if p.poll() == None:
            debug("NOT DEAD")
        else:
            debug("DEAD")
            success = False

        debug("waiting for invoker to terminate ...")
        p.wait()

        debug("terminating fala_ft_hello ...")
        Popen(['pkill', 'fala_ft_hello']).wait()

        self.assert_(success, "invoker terminated before delay elapsed")


    def test_fd_booster_m(self):
        """
        File descriptor test for booster-m
        """
        count = get_file_descriptor("booster-m","m")
        self.assert_(count != 0, "None of the file descriptors were changed")

    def test_fd_booster_q(self):
        """
        File descriptor test for booster-q
        """
        count = get_file_descriptor("booster-q","qt")
        self.assert_(count != 0, "None of the file descriptors were changed")

    def test_restart_booster(self):
        """
        Test that booster is restarted if it is killed 
        """
        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        qpid = get_pid('booster-q')
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(mpid != None, "No booster process running")

        #Kill the booster processes
        debug("Kill the booster processes")
        kill_process(apppid=qpid)
        kill_process(apppid=mpid)
        
        #wait for the boosters to be restarted
        debug("wait for the boosters to be restarted")
        time.sleep(6)

        #check that the new boosters are started
        debug("check that the new boosters are started")
        qpid_new = get_pid('booster-q')
        self.assert_(qpid_new != None, "No booster process running")
        self.assert_(qpid_new != qpid, "booster process was not killed")

        mpid_new = get_pid('booster-m')
        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")

    def test_invoker_exit_status(self):
        """
        To test that invoker returns the same exit status as the application
        """
        #Run application without invoker and get the exit status
        debug("Run application without invoker and get the exit status")
        st, op = commands.getstatusoutput('/usr/bin/fala_status.launch')
        app_st_wo_inv = os.WEXITSTATUS(st)
        debug("The exit status of app without invoker is : %d" %app_st_wo_inv)
    
        #Run application with invoker and get the exit status
        debug("Run application with invoker and get the exit status")
        st, op = commands.getstatusoutput('invoker --type=m --wait-term /usr/bin/fala_status.launch')
        app_st_w_inv = os.WEXITSTATUS(st)
        debug("The exit status of app with invoker is : %d" %app_st_w_inv)
        
        self.assert_(app_st_wo_inv == app_st_w_inv, "The invoker returns a wrong exit status")

    def test_invoker_gid_uid(self):
        """
        To Test that the set gid and uid is passed from invoker process to launcher
        """
        debug("This test uses a test application that returns the uid and gid and exits")
        #get the id in user mode 
        debug("Get the system's uid and gid in User Mode \n")
        st, op =  commands.getstatusoutput('su user -c ' "id")
        usr_id1 = op.split(' ')[0].split('(')[0]
        grp_id1 = op.split(' ')[1].split('(')[0]
        debug("System %s \tSyetem %s" %(usr_id1, grp_id1))

        #get invokers's uid and gid  by running the application using invoker in user mode
        debug("get invoker's uid and gid by running the application using invoker in user mode")
        app = "invoker --type=m --no-wait /usr/bin/fala_status.launch" 
        st, op = commands.getstatusoutput('su user -c "%s"' %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        debug("Invoker %s \tInvoker %s" %(usr_id, grp_id))
        
        #get application's uid and gid by running the application without invoker in user mode
        debug("get application's uid and gid by running the application without invoker in user mode")
        app = "/usr/bin/fala_status.launch"
        st, op = commands.getstatusoutput('su user -c "%s"' %app );
        usr_id2 = op.split('\n')[-2]
        grp_id2 = op.split('\n')[-1]
        debug("Application %s \tApplication %s" %(usr_id2, grp_id2))

        self.assert_(usr_id == usr_id1, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id1, "The correct GID is not passed by invoker")

        self.assert_(usr_id == usr_id2, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id2, "The correct GID is not passed by invoker")

        #get the id in root mode 
        debug("Get the Sysem's uid and gid in Root Mode \n")
        st, op =  commands.getstatusoutput("id")
        usr_id1 = op.split(' ')[0].split('(')[0]
        grp_id1 = op.split(' ')[1].split('(')[0]
        debug("System %s \tSyetem %s" %(usr_id1, grp_id1))

        #get id by running the application using invoker in root mode
        debug("get invoker's uid and gid by running the application using invoker in root mode")
        app = "invoker --type=m --no-wait /usr/bin/fala_status.launch" 
        st, op = commands.getstatusoutput("%s" %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        debug("Invoker %s \tInvoker %s" %(usr_id, grp_id))
        
        #get id by running the application without invoker in root mode
        debug("get application's uid and gid  by running the application without invoker in root mode")
        app = "/usr/bin/fala_status.launch"
        st, op = commands.getstatusoutput("%s" %app );
        usr_id2 = op.split('\n')[-2]
        grp_id2 = op.split('\n')[-1]
        debug("Application %s \tApplication %s" %(usr_id2, grp_id2))

        self.assert_(usr_id == usr_id1, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id1, "The correct GID is not passed by invoker")

        self.assert_(usr_id == usr_id2, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id2, "The correct GID is not passed by invoker")
       

    def test_signal_forwarding(self):
        """
        To test that invoker is killed by the same signal as the application
        """
        #Test for m-booster
        debug("Test for m-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_m.py")
        debug("The Invoker killed by : %s" %op)
    
        self.assert_(op == 'Segmentation fault (core dumped)', "The invoker(m-booster) was not killed by the same signal")
        time.sleep(2)
         
        #This case is launching the application in user mode
        #Test for q-booster
        debug("Test for q-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_qt.py")
        debug("The Invoker killed by : %s" %op.split('\n')[-1])
    
        self.assert_(op.split('\n')[-1] == 'Aborted (core dumped)', "The invoker(q-booster) was not killed by the same signal")
        time.sleep(2)

    def test_launch_wo_applauncherd(self):
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

        time.sleep(4)

    def test_invoker_search_prog(self):
        """
        Test that invoker can find programs from directories listed in
        PATH environment variable and that it doesn't find something
        that isn't there.
        """

        # invoker searches PATH for the executable
        p = run_app_as_user('invoker --type=m --no-wait fala_ft_hello.launch')
        self.assert_(p.wait() == 0, "Couldn't launch fala_ft_hello.launch")
        time.sleep(2)
        kill_process('fala_ft_hello')

        # launch with relative path

        p = run_app_as_user('cd /usr/share/; invoker --type=m --no-wait ' + 
                            "../bin/fala_ft_hello.launch")
        self.assert_(p.wait() == 0, "Couldnt launch fala_ft_hello.launch" + 
                    " with relative path")
        time.sleep(2)
        kill_process('fala_ft_hello')

        # and finally, try to launch something that doesn't exist
        p = run_app_as_user('invoker --type=m --no-wait spam_cake.launch')
        self.assert_(p.wait() != 127, "Found spam_cake.launch for some reason")
        time.sleep(2)
        kill_process('spam_cake')
    
    def test_booster_killed_or_restarted(self):
        """
        Test that boosters are killed if applauncherd is stopped
        and restarted if applauncherd is killed
        """
        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        qpid = get_pid('booster-q')
        self.assert_(len(qpid.split("\n")) == 1, "multiple instances of booster-q running")
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(len(mpid.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid != None, "No booster process running")

        #stop applauncherd
        stop_applauncherd()
 
        #wait for the boosters to be killed 
        time.sleep(2)

        #check that the none of the booster is running
        debug("check that the none of the booster is running")
        qpid_new = get_pid('booster-q')
        self.assert_(qpid_new == None, "booster-q still running")
        
               
        mpid_new = get_pid('booster-m')
        self.assert_(mpid_new == None, "booster-m still running")

        #Now start the applauncherd
        start_applauncherd()
        
        #wait for the boosters to be restarted
        time.sleep(6)

        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        qpid = get_pid('booster-q')
        self.assert_(len(qpid.split("\n")) == 1, "multiple instances of booster-q running")
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(len(mpid.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid != None, "No booster process running")

        #Now kill applauncherd
        debug("Now kill applauncherd")
        kill_process('applauncherd')
        
        #wait for the boosters to be restarted
        time.sleep(6)
        
        #check that the new boosters are started
        debug("check that the new boosters are started")
        qpid_new = get_pid('booster-q')
        self.assert_(len(qpid_new.split("\n")) == 1, "multiple instances of booster-q running")
        self.assert_(qpid_new != None, "No booster process running")
        self.assert_(qpid_new != qpid, "booster process was not killed")

        mpid_new = get_pid('booster-m')
        self.assert_(len(mpid_new.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")
            
    def test_invoker_param_creds(self):
        p = run_app_as_user('invoker --creds')
        self.assert_(p.wait() == 0, "'invoker --creds' failed")

    def test_invoker_param_respawn_delay(self):
        p = run_app_as_user('invoker --respawn 10 --type=q --no-wait fala_ft_hello.launch')

        time.sleep(7)

        pid = get_pid('booster-q')
        self.assert_(pid == None, "'booster-q' was respawned too soon")

        time.sleep(7)

        pid = get_pid('booster-q')
        self.assert_(pid != None, "'booster-q' was not respawned in time")

        p = run_app_as_user('invoker --respawn 256 --type=q --no-wait fala_ft_hello.launch')
        self.assert_(p.wait() != 0, "invoker didn't die with too big respawn delay")
        kill_process('fala_ft_hello')

    def test_invoker_bogus_apptype(self):
        p = run_app_as_user('invoker --type=foobar fala_ft_hello.launch')
        self.assert_(p.wait() != 0, "invoker didn't die with bogus apptype")

        p = run_app_as_user('invoker fala_ft_hello.launch')
        self.assert_(p.wait() != 0, "invoker didn't die with empty apptype")

    def test_booster_pid_change(self):
        """
        Test that application pid changes to the booster 'x' when application 
        is started with invoker --type='x'
        """
        for i in xrange(3):
            #Launching application with booster-m
            mpid = get_pid('booster-m') 
            p = run_app_as_user('invoker --type=m --no-wait fala_ft_hello.launch')
            time.sleep(4)
            app_pid = get_pid('fala_ft_hello')
            mpid_new = get_pid('booster-m')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == mpid, "Application is not assigned the booster-m pid")
            self.assert_(mpid_new != None, "No booster process running")
            self.assert_(mpid_new != mpid, "booster-m process did not receive the new pid")
            kill_process('fala_ft_hello')
            
            #Launching application with booster-q
            qpid = get_pid('booster-q') 
            p = run_app_as_user('invoker --type=q --no-wait fala_ft_hello.launch')
            time.sleep(4)
            app_pid = get_pid('fala_ft_hello')
            qpid_new = get_pid('booster-q')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == qpid, "Application is not assigned the booster-q pid")
            self.assert_(qpid_new != None, "No booster process running")
            self.assert_(qpid_new != qpid, "booster-q process did not receive the new pid")
            kill_process('fala_ft_hello')

    def test_stress_boosted_apps(self):
        """
        Stress test for boosted applications to check only one instance is running.
        """
        if get_pid('fala_ft_hello') != None:
            kill_process('fala_ft_hello')
        time.sleep(2)
        count = 0
        p = run_app_as_user('invoker --type=m --no-wait fala_ft_hello.launch')
        pid = get_pid('fala_ft_hello')
        for i in xrange(10):
            p = run_app_as_user('invoker --type=m --no-wait fala_ft_hello.launch')
            app_pid = get_pid('fala_ft_hello')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(pid == app_pid, "Same instance of application not running")
        st, op = commands.getstatusoutput('ps ax | grep invoker | grep fala_ft_hello.launch | grep -v -- -sh | wc -l')
        count = int(op)
        while count != 0:
            debug("The value of queue is %d" %count)
            time.sleep(3)
            debug("Sleeping for 3 secs")
            st, op = commands.getstatusoutput('ps ax | grep invoker | grep fala_ft_hello.launch | grep -v -- -sh | wc -l')
            count = int(op)
        kill_process('fala_ft_hello')
        
    def test_launched_app_name(self):
        """
        Test that launched application name  have .launch at the end 
        """
        #For booster-m        
        #Check though the process list
        p = run_app_as_user('invoker --type=m --no-wait fala_wl.launch -faulty')
        time.sleep(2)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('0')[0] == "fala_wl.launch",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        self.assert_(op1.split(",")[0] == '"fala_wl.launch"','Application name is incorrect')   
        kill_process('fala_wl') 
        
        #For booster-q        
        #Check though the process list
        p = run_app_as_user('invoker --type=qt --no-wait fala_wl.launch -faulty')
        time.sleep(2)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('0')[0] == "fala_wl.launch",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        self.assert_(op1.split(",")[0] == '"fala_wl.launch"','Application name is incorrect')   
        kill_process('fala_wl') 

    def test_oom_adj(self):
        """
        Test that oom.adj is 0 for launched application process 
        """
        p = run_app_as_user('invoker --type=qt --no-wait fala_wl.launch')
        time.sleep(2)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/oom_adj' %pid)
        self.assert_(op == '0',"oom.adj of the launched process is not 0")
        kill_process('fala_wl') 

    def test_single_instance_window_raise_without_invoker(self):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Minimize it with xsendevent
        # 3. Start another multi-instance application with single-instance binary
        # 4. Check that there in only one app and its application pid is the same than with 1. launch
        # 5. Check that window is raised with correct pid (from log files written by test application)
        # 6. Close the application
        # 7. Start the multi-instance application with single-instance binary
        # 8. Check that the pid has changed

    def test_single_instance_and_non_single_instance_without_invoker(self):
        # 1. Start the multi-instance application without single-instance binary
        # 2. Start another multi-instance application with single-instance binary
        # 3. Check that both  application pids exist

    def test_single_instance_stress_test_without_invoker(self):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Minimize it with xsendevent
        # 3. Start the multi-instance app with single-instance binary 20 times very fast
        #    - check the return value of each launch is zero
        # 4. Check that there in only one application pid and it has the same than with 1. launch
        # 5. Check that window is raised with correct pid (from log files written by test application)

    def test_single_instance_abnormal_lock_release_without_invoker(self):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Kill the application with -9
        # 3. Start the multi-instance application with single-instance binary
        # 4. Check that application can be started and pid has changed

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
