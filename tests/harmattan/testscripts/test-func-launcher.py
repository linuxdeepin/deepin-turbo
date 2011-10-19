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

Authors:   nimika.1.keshri@nokia.com
           ext-oskari.timperi@nokia.com
"""

import os, os.path, glob
import subprocess
import commands
import time
import sys
import unittest
import re
import string
from subprocess import Popen
from utils import *
from os.path import basename

using_scratchbox = False

def check_prerequisites():
    if os.getenv('DISPLAY') == None:
        error("DISPLAY is not set. Check the requirements.")
        
    if os.getenv('DBUS_SESSION_BUS_ADDRESS') == None:
        error("DBUS_SESSION_BUS_ADDRESS is not set.\n" +
              "You probably want to source /tmp/session_bus_address.user")

    for app in LAUNCHABLE_APPS: 
        assert len(basename(app)) <= 15, "For app: %s , base name !<= 14" %app

def number_of_file_descriptors_for_pid(pid) :
    command = 'lsof -p %s' %(pid)
    p = run_cmd_as_user(command, out = subprocess.PIPE)
    output, errors = p.communicate()
    debug("File descriptor count: %s for proces PID: %s" %(output.count('\n')-1, pid))
    return output.count('\n')-1

class daemon_handling (unittest.TestCase):

    def stop_daemons(self):
        stop_daemons()

    def start_daemons(self):
        start_daemons()

def has_GL_context(processId):
    processMapsFile = open("/proc/" + processId + "/maps")
    processMapsLines = processMapsFile.readlines()
    for i in processMapsLines :
        if "/dev/pvrsrvkm" in i :
            return True

    return False

class launcher_tests (unittest.TestCase):
    def setUp(self):
        if daemons_running():
            stop_daemons()
            self.START_DAEMONS_AT_TEARDOWN = True
        else:
            self.START_DAEMONS_AT_TEARDOWN = False

        if get_pid('applauncherd') == None:
            start_applauncherd()
            
        #setup here
        debug("Executing SetUp")

    def tearDown(self):
        #teardown here
        debug("Executing TearDown")
        if get_pid('applauncherd') == None:
            start_applauncherd()

        wait_for_single_applauncherd()
        if self.START_DAEMONS_AT_TEARDOWN:
            start_daemons()
    
    def sighup_applauncherd(self):
        same_pid, booster_status = send_sighup_to_applauncherd()
        self.assert_(same_pid, "Applauncherd has new pid after SIGHUP")
        self.assert_(booster_status, "Atleast one of the boosters is not restarted")

    #Testcases

    def test_applications_exist(self):
        """
        test_launchable_application_exists
        """
        failed_apps = []
        for app in LAUNCHABLE_APPS: 
            if not (os.path.isfile(app) and os.access(app, os.X_OK)): 
                failed_apps.append(app)
        self.assert_(failed_apps == [], "Some applications do not have the launch files, list: %s" % str(failed_apps))

    def test_only_one_applauncherd_exist(self):
        """
        test_only_one_applauncherd_can_start
        """
        stop_applauncherd()
        st, op = commands.getstatusoutput("initctl start xsession/applauncherd")
        time.sleep(2)
        st_new, op = commands.getstatusoutput("initctl start xsession/applauncherd")
 
        self.assert_(st == 0, "Applauncherd do not start")
        self.assert_(st_new != 0, "Applauncherd even force start second time")

    def test_zombie_state_m(self):
        self._test_zombie_state(PREFERED_APP,'m')

    def test_zombie_state_qml(self):
        self._test_zombie_state(PREFERED_APP_QML,'d')

    def test_zombie_state_e(self):
        self._test_zombie_state(PREFERED_APP, 'e')

    def _test_zombie_state(self, prefered_app, btype, sighup = True):
        """
        To test that no Zombie process exist after the application is killed
        """
        #launch application with launcher
        #check if the application is running
        #kill the application (pid = p.pid)
        #check if pgrep appname should be nothing
        #self.kill_process(LAUNCHER_BINARY)

        process_handle = run_app_as_user_with_invoker(prefered_app,booster = btype )
        process_id = wait_for_app(prefered_app, 5)
        debug("The pid of %s id %s" %(prefered_app, process_id))
        kill_process(prefered_app)
        time.sleep(4)

        process_handle = run_app_as_user_with_invoker(prefered_app,booster = btype)
        process_id1 = wait_for_app(prefered_app, 5)
        debug("The pid of %s id %s" %(prefered_app, process_id1))
        kill_process(prefered_app)
        time.sleep(4)

        process_id1 = get_pid(prefered_app)
        debug("The pid of %s id %s" %(prefered_app, process_id1))

        self.assert_(process_id != process_id1 , "New Process not launched")
        self.assert_(process_id1 == None , "Process still running")
        if(sighup):
            self.sighup_applauncherd()
            self._test_zombie_state(prefered_app, btype, False)
    
    def test_launch_multiple_apps_m(self):
        self._test_launch_multiple_apps(LAUNCHABLE_APPS, 'm')

    def test_launch_multiple_apps_qml(self):
        self._test_launch_multiple_apps(LAUNCHABLE_APPS_QML, 'd')

    def test_launch_multiple_apps_e(self):
        self._test_launch_multiple_apps(LAUNCHABLE_APPS, 'e')

    def _test_launch_multiple_apps(self, launchable_apps, btype, sighup = True):
        """
        To test that more than one applications are launched by the launcher 
        """

        def kill_launched(pids):
            for pid in pids:
                kill_process(apppid = pid)

        pidlist = []

        for app in launchable_apps:
            p = run_app_as_user_with_invoker(app, booster = btype)
            pid = wait_for_app(app, timeout = 10, sleep = 1)

            if pid == None:
                kill_launched(pidlist)
                self.fail("%s was not launched using applauncherd")

            pidlist.append(pid)
        kill_launched(pidlist)
        self.assert_(len(pidlist) == len(launchable_apps),"All apps were not launched")
        if(sighup):
            self.sighup_applauncherd()
            self._test_launch_multiple_apps(launchable_apps, btype, False)
    
    def test_one_instance_m(self):
        self._test_one_instance(PREFERED_APP, 'm')

    def test_one_instance_qml(self):
        self._test_one_instance(PREFERED_APP_QML, 'd')

    def test_one_instance_e(self):
        self._test_one_instance(PREFERED_APP, 'e')

    def _test_one_instance(self, prefered_app, btype, sighup = True):
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
        process_handle = run_app_as_user_with_invoker(prefered_app, booster=btype)
        process_id = wait_for_app(prefered_app)
        debug("PID of first %s" % process_id)
        time.sleep(3)

        process_handle1 = run_app_as_user_with_invoker(prefered_app, booster=btype)
        time.sleep(3)
        process_id = wait_for_app(prefered_app)
        debug("PID of 2nd %s" % process_id)

        kill_process(prefered_app)

        self.assert_( len(process_id.split(' ')) == 1, "Only one instance of app not running")
        if(sighup):
            self.sighup_applauncherd()
            self._test_one_instance(prefered_app, btype, False)
        
    def test_launch_multiple_apps_cont_m(self):
        self._test_launch_multiple_apps_cont(LAUNCHABLE_APPS, 'fala_ft_hello', 'm')

    def test_launch_multiple_apps_cont_qml(self):
        self._test_launch_multiple_apps_cont(LAUNCHABLE_APPS_QML, 'fala_qml_helloworld','d')

    def test_launch_multiple_apps_cont_e(self):
        self._test_launch_multiple_apps_cont(LAUNCHABLE_APPS, 'fala_ft_hello', 'e')

    def _test_launch_multiple_apps_cont(self, launchable_apps, app_common_prefix, btype, sighup = True):
        """
        To test that more than one applications are launched by the launcher 
        """
        for app in launchable_apps: 
            #launch application with launcher
            #check if the application is running
            #check if p.pid is same as pgrep appname
            #in a global dictionary, append the pid
            process_handle = run_app_as_user_with_invoker(app, booster = btype)

        time.sleep(15)

        process_id = get_pid(app_common_prefix)
        pid_list = process_id.split()
        for pid in pid_list:
            kill_process(apppid=pid)

        self.assert_(len(pid_list) == len(launchable_apps), "All Applications were not launched using launcher")
        if(sighup):
            self.sighup_applauncherd()
            self._test_launch_multiple_apps_cont(launchable_apps, app_common_prefix, btype, False)


    def test_fd_booster_m(self, sighup = True):
        """
        File descriptor test for booster-m
        """
        count = get_file_descriptor("booster-m", "m", "fala_ft_hello")
        self.assert_(count != 0, "None of the file descriptors were changed")
        if(sighup):
            self.sighup_applauncherd()
            self.test_fd_booster_m(False)

    def test_fd_booster_q(self, sighup = True):
        """
        File descriptor test for booster-q
        """
        count = get_file_descriptor("booster-q", "qt", "fala_ft_hello")
        self.assert_(count != 0, "None of the file descriptors were changed")
        if(sighup):
            self.sighup_applauncherd()
            self.test_fd_booster_q(False)

    def test_fd_booster_d(self, sighup = True):
        """
        File descriptor test for booster-d
        """
        count = get_file_descriptor("booster-d", "d", "fala_qml_helloworld")
        self.assert_(count != 0, "None of the file descriptors were changed")
        if(sighup):
            self.sighup_applauncherd()
            self.test_fd_booster_d(False)

    def test_fd_booster_e(self, sighup = True):
        """
        File descriptor test for booster-e
        """
        count = get_file_descriptor("booster-e", "e", "fala_ft_hello")
        self.assert_(count != 0, "None of the file descriptors were changed")
        if(sighup):
            self.sighup_applauncherd()
            self.test_fd_booster_e(False)

    def test_restart_booster(self, sighup = True):
        """
        Test that booster is restarted if it is killed 
        """
        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        qpid = get_pid('booster-q')
        self.assert_(qpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(mpid != None, "No booster process running")

        dpid = get_pid('booster-d')
        self.assert_(dpid != None, "No booster process running")

        epid = get_pid('booster-e')
        self.assert_(epid != None, "No booster process running")

        #Kill the booster processes
        debug("Kill the booster processes")
        kill_process(apppid=qpid)
        kill_process(apppid=mpid)
        kill_process(apppid=dpid)
        kill_process(apppid=epid)
        
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

        dpid_new = get_pid('booster-d')
        self.assert_(dpid_new != None, "No booster process running")
        self.assert_(dpid_new != dpid, "booster process was not killed")

        epid_new = get_pid('booster-e')
        self.assert_(epid_new != None, "No booster process running")
        self.assert_(epid_new != epid, "booster process was not killed")

        if(sighup):
            self.sighup_applauncherd()
            self.test_restart_booster(False)

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

        dpid = get_pid('booster-d')
        self.assert_(len(dpid.split("\n")) == 1, "multiple instances of booster-d running")
        self.assert_(dpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(len(mpid.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid != None, "No booster process running")

        epid = get_pid('booster-e')
        self.assert_(len(epid.split("\n")) == 1, "multiple instances of booster-e running")
        self.assert_(epid != None, "No booster process running")

        #stop applauncherd
        stop_applauncherd()
 
        #wait for the boosters to be killed 
        time.sleep(2)

        #check that the none of the booster is running
        debug("check that the none of the booster is running")
        qpid_new = get_pid('booster-q')
        self.assert_(qpid_new == None, "booster-q still running")
        
        dpid_new = get_pid('booster-d')
        self.assert_(dpid_new == None, "booster-d still running")
        
        mpid_new = get_pid('booster-m')
        self.assert_(mpid_new == None, "booster-m still running")

        epid_new = get_pid('booster-e')
        self.assert_(epid_new == None, "booster-e still running")

        #Now start the applauncherd
        start_applauncherd()
        
        #wait for the boosters to be restarted
        time.sleep(6)

        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        qpid = get_pid('booster-q')
        self.assert_(len(qpid.split("\n")) == 1, "multiple instances of booster-q running")
        self.assert_(qpid != None, "No booster process running")

        dpid = get_pid('booster-d')
        self.assert_(len(dpid.split("\n")) == 1, "multiple instances of booster-d running")
        self.assert_(dpid != None, "No booster process running")

        mpid = get_pid('booster-m')
        self.assert_(len(mpid.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid != None, "No booster process running")

        epid = get_pid('booster-e')
        self.assert_(len(epid.split("\n")) == 1, "multiple instances of booster-e running")
        self.assert_(epid != None, "No booster process running")

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

        dpid_new = get_pid('booster-d')
        self.assert_(len(dpid_new.split("\n")) == 1, "multiple instances of booster-d running")
        self.assert_(dpid_new != None, "No booster process running")
        self.assert_(dpid_new != dpid, "booster process was not killed")

        mpid_new = get_pid('booster-m')
        self.assert_(len(mpid_new.split("\n")) == 1, "multiple instances of booster-m running")
        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")
            
        epid_new = get_pid('booster-e')
        self.assert_(len(epid_new.split("\n")) == 1, "multiple instances of booster-e running")
        self.assert_(epid_new != None, "No booster process running")
        self.assert_(epid_new != epid, "booster process was not killed")

    def test_booster_pid_change(self, sighup = True):
        """
        Test that application pid changes to the booster 'x' when application 
        is started with invoker --type='x'
        """
        for i in xrange(3):
            #Launching application with booster-m
            mpid = get_pid('booster-m') 
            p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'm', arg = '--no-wait')
            time.sleep(4)
            app_pid = get_pid('fala_ft_hello')
            mpid_new = get_pid('booster-m')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == mpid, "Application is not assigned the booster-m pid")
            self.assert_(mpid_new != None, "No booster process running")
            self.assert_(mpid_new != mpid, "booster-m process did not receive the new pid")
            kill_process('fala_ft_hello')
            
            #Launching application with booster-d
            dpid = get_pid('booster-d') 
            p = run_app_as_user_with_invoker(PREFERED_APP_QML, booster = 'd', arg = '--no-wait')
            time.sleep(4)
            app_pid = get_pid('fala_qml_helloworld')
            dpid_new = get_pid('booster-d')
            kill_process('fala_qml_helloworld')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == dpid, "Application is not assigned the booster-d pid")
            self.assert_(dpid_new != None, "No booster process running")
            self.assert_(dpid_new != dpid, "booster-d process did not receive the new pid")
            
            #Launching application with booster-e
            epid = get_pid('booster-e') 
            p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'e', arg = '--no-wait')
            time.sleep(4)
            app_pid = get_pid('fala_ft_hello')
            epid_new = get_pid('booster-e')
            kill_process('fala_ft_hello')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == epid, "Application is not assigned the booster-e pid")
            self.assert_(epid_new != None, "No booster process running")
            self.assert_(epid_new != epid, "booster-e process did not receive the new pid")

            #Launching application with booster-q
            qpid = get_pid('booster-q') 
            p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'q', arg = '--no-wait')
            time.sleep(4)
            app_pid = get_pid('fala_ft_hello')
            qpid_new = get_pid('booster-q')
            kill_process('fala_ft_hello')
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(app_pid == qpid, "Application is not assigned the booster-q pid")
            self.assert_(qpid_new != None, "No booster process running")
            self.assert_(qpid_new != qpid, "booster-q process did not receive the new pid")

        if(sighup):
            self.sighup_applauncherd()
            self.test_booster_pid_change(False)

        
    def test_stress_boosted_apps(self):
        self._test_stress_boosted_apps('m', 'fala_ft_hello')
        time.sleep(5)
        self._test_stress_boosted_apps('d', 'fala_qml_helloworld', invoker_extra_flags='--single-instance')
        time.sleep(5)
        self._test_stress_boosted_apps('e', 'fala_ft_hello')
        time.sleep(5)

    def _test_stress_boosted_apps(self, booster_type, app_name, invoker_extra_flags='', sighup = True):
        """
        Stress test for boosted applications to check only one instance is running.
        """
        if get_pid(app_name) != None:
            kill_process(app_name)
        time.sleep(2)
        count = 0
        p = run_cmd_as_user('invoker --type=%s --no-wait %s %s' % (booster_type, invoker_extra_flags, app_name))
        pid = get_pid(app_name)
        for i in xrange(10):
            p = run_cmd_as_user('invoker --type=%s --no-wait %s %s' % (booster_type, invoker_extra_flags, app_name))
            app_pid = get_pid(app_name)
            self.assert_(app_pid != None, "Application is not running")
            self.assert_(pid == app_pid, "Same instance of application not running")
        st, op = commands.getstatusoutput('ps ax | grep invoker | grep %s| grep -v -- -sh | wc -l' % app_name)
        count = int(op)
        while count != 0:
            debug("The value of queue is %d" %count)
            time.sleep(3)
            debug("Sleeping for 3 secs")
            app_pid = get_pid(app_name)
            kill_process(app_name)
            self.assert_(pid == app_pid, "Same instance of application not running")
            st, op = commands.getstatusoutput('ps ax | grep invoker | grep %s| grep -v -- -sh | wc -l' % app_name)
            count = int(op)
        if(sighup):
            self.sighup_applauncherd()
            self._test_stress_boosted_apps(booster_type, app_name, invoker_extra_flags, False)

    def test_launched_app_name(self, sighup = True):
        """
        Test that launched application have correct applicationname 
        """
        #For booster-m        
        #Check though the process list
        p = run_cmd_as_user('invoker --type=m --no-wait fala_wl -faulty')
        time.sleep(5)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('\0')[0] == "fala_wl",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        kill_process('fala_wl') 
        self.assert_(op1.split(",")[0] == '"fala_wl"','Application name is incorrect')   
        
        #For booster-d
        #Check though the process list
        p = run_cmd_as_user('invoker --type=d --no-wait fala_qml_helloworld -faulty')
        time.sleep(2)
        pid = get_pid('fala_qml_helloworld')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('\0')[0] == "fala_qml_helloworld",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd QML testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        kill_process('fala_qml_helloworld') 
        self.assert_(op1.split(",")[0] == '"fala_qml_helloworld"','Application name is incorrect')   
        
        #For booster-e
        #Check though the process list
        p = run_cmd_as_user('invoker --type=e --no-wait fala_wl -faulty')
        time.sleep(2)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('\0')[0] == "fala_wl",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        kill_process('fala_wl') 
        self.assert_(op1.split(",")[0] == '"fala_wl"','Application name is incorrect')   

        #For booster-q        
        #Check though the process list
        p = run_cmd_as_user( 'invoker --type=qt --no-wait fala_wl -faulty')
        time.sleep(2)
        pid = get_pid('fala_wl')
        st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
        self.assert_(op.split('\0')[0] == "fala_wl",'Application name is incorrect')    
    
        #check through the window property
        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
        st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %op)
        kill_process('fala_wl') 
        self.assert_(op1.split(",")[0] == '"fala_wl"','Application name is incorrect')   

        if(sighup):
            self.sighup_applauncherd()
            self.test_launched_app_name(False)


    def test_unix_signal_handlers(self, sighup = True):
        """
        Test unixSignalHandlers by killing booster-m and booster-d, signal hup
        """

        mpid = get_pid('booster-m')
        st, op = commands.getstatusoutput('kill -hup %s' % mpid)
        time.sleep(2)
        mpid_new = wait_for_app('booster-m')
        self.assert_(mpid != mpid_new, "booster-m pid is not changed")
        
        
        dpid = get_pid('booster-d')
        st, op = commands.getstatusoutput('kill -hup %s' % dpid)
        time.sleep(2)
        dpid_new = wait_for_app('booster-d')
        self.assert_(dpid != dpid_new, "booster-d pid is not changed")

        if(sighup):
            self.sighup_applauncherd()
            self.test_unix_signal_handlers(False)

    def test_qttas_load_booster_d(self):
        self._test_qttas_load_booster(PREFERED_APP_QML, 'd')

    def test_qttas_load_booster_m(self):
        self._test_qttas_load_booster(PREFERED_APP, 'm')

    def _test_qttas_load_booster(self, testapp, btype, sighup = True):
        """
        To test invoker that qttestability plugin is loaded with -testability argument for booster-d
        """
        for i in range(2):
                debug("Running cycle %s" %i)
                p = run_app_as_user_with_invoker("%s -testability" %testapp, booster = btype)
                time.sleep(2)

                pid = get_pid(testapp)
                self.assert_(pid != None, "Can't start application %s" %testapp)

                st_tas, op_tas = commands.getstatusoutput("grep -c libtestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas)
                debug("The value of output is %s" %op_tas)

                st_tas_plug, op_tas_plug = commands.getstatusoutput("grep -c libqttestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas_plug)
                debug("The value of output is %s" %op_tas_plug)

                kill_process(apppid=pid)

                self.assert_(st_tas == 0,"libtestability.so not loaded")
                self.assert_(st_tas_plug == 0,"libqttestability.so not loaded")

                time.sleep(2)

        if(sighup):
            self.sighup_applauncherd()
            self._test_qttas_load_booster(testapp, btype, False)

    def test_qttas_load_env_booster_d(self):
        self._test_qttas_load_env_booster(PREFERED_APP_QML, 'd')

    def test_qttas_load_env_booster_m(self):
        self._test_qttas_load_env_booster(PREFERED_APP, 'm')

    def _test_qttas_load_env_booster(self, testapp, btype, sighup = True):
        """
        To test invoker that qttestability plugin is loaded with QT_LOAD_TESTABILITY env variable for booster-d
        """
        for i in range(2):
                debug("Running cycle %s" %i)
                cmd = ['su', '-', 'user', '-c'] 
                invoke="export QT_LOAD_TESTABILITY=1; /usr/bin/invoker --type=%s %s" %(btype, testapp)
                cmd.append(invoke)

                p = subprocess.Popen(cmd, shell = False, stdout = DEV_NULL, stderr = DEV_NULL)

                time.sleep(2)

                pid = get_pid(testapp)
                self.assert_(pid != None, "Can't start application %s" %testapp)

                st_tas, op_tas = commands.getstatusoutput("grep -c libtestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas)
                debug("The value of output is %s" %op_tas)

                st_tas_plug, op_tas_plug = commands.getstatusoutput("grep -c libqttestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas_plug)
                debug("The value of output is %s" %op_tas_plug)

                kill_process(apppid=pid)

                self.assert_(st_tas == 0,"libtestability.so not loaded")
                self.assert_(st_tas_plug == 0,"libqttestability.so not loaded")

                time.sleep(2)

        if(sighup):
            self.sighup_applauncherd()
            self._test_qttas_load_env_booster(testapp, btype, False)


    def test_dirPath_filePath_m(self):
        self._test_dirPath_filePath('m', "/usr/share/fala_images", "fala_hello")

    def test_dirPath_filePath_d(self):
        self._test_dirPath_filePath('d', "/usr/share/fala_images", "fala_qml_helloworld")

    def test_dirPath_filePath_q(self):
        self._test_dirPath_filePath('q', "/usr/share/fala_images", "fala_qml_helloworld")

    def test_dirPath_filePath_e(self):
        self._test_dirPath_filePath('e', "/usr/share/fala_images", "fala_qml_helloworld")

    def _test_dirPath_filePath(self, btype, path, testapp, sighup = True):
        """
        Test that correct file path and dir path is passed
        """
        if os.path.isfile("/tmp/%s.log" % testapp):
            os.system("rm /tmp/%s.log" % testapp)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s %s/%s' % (btype, path, testapp))
        time.sleep(4)
        pid = get_pid(testapp)
        self.assert_(pid != None, "The application was not launched")
        debug("get filePath and dirPath from log file")
        st, op = commands.getstatusoutput("grep Path /tmp/%s.log | tail -2" % testapp)
        dirpath = op.split("\n")[0].split(" ")[2]
        self.assert_(dirpath == path, "Wrong dirPath: %s" % dirpath)
        filepath = op.split("\n")[1].split(" ")[2]
        kill_process(apppid=pid)
        self.assert_(filepath == "%s/%s" % (path, testapp), "Wrong filePath: %s" % filepath)

        if(sighup):
            self.sighup_applauncherd()
            self._test_dirPath_filePath(btype, path, testapp, False)
        
    def test_argv_mbooster_limit(self):
        self._test_argv_booster_limit("m", "fala_wl")

    def test_argv_dbooster_limit(self):
        self._test_argv_booster_limit("d", "fala_qml_helloworld")

    def _test_argv_booster_limit(self, btype, testapp, sighup = True):
        """
        Test that ARGV_LIMIT (32) arguments are successfully passed to cached [QM]Application.
        """
        if os.path.isfile("/tmp/%s.log" % testapp):
            os.system("rm /tmp/%s.log" % testapp)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s /usr/bin/%s --log-args 0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t' % (btype, testapp))
        time.sleep(4)
        pid = get_pid(testapp)
        self.assert_(pid != None, "The application was not launched")
        debug("get arguments from log file")
        st, op = commands.getstatusoutput("grep argv: /tmp/%s.log | tail -2" % testapp)
        original_argv = op.split("\n")[0]
        cache_argv = op.split("\n")[1]
        kill_process(apppid=pid)
        self.assert_(original_argv == cache_argv, "Wrong arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))
        
        if(sighup):
            self.sighup_applauncherd()
            self._test_argv_booster_limit(btype, testapp, False)
        
    def test_argv_mbooster_over_limit(self):
        self._test_argv_booster_over_limit("m", "fala_wl")

    def test_argv_dbooster_over_limit(self):
        self._test_argv_booster_over_limit("d", "fala_qml_helloworld")

    def _test_argv_booster_over_limit(self, btype, testapp, sighup = True):
        """
        Test that if more than ARGV_LIMIT (32) arguments are passed to cached [QM]Application,
        the application is still launched and ARGV_LIMIT arguments are successfully passed.
        """
        ARGV_LIMIT = 32
        if os.path.isfile("/tmp/%s.log" % testapp):
            os.system("rm /tmp/%s.log" % testapp)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s /usr/bin/%s --log-args 0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t u v w x y z' % (btype, testapp))
        time.sleep(4)
        pid = get_pid(testapp)
        kill_process(apppid=pid)
        self.assert_(pid != None, "The application was not launched")
        debug("get arguments from log file")
        st, op = commands.getstatusoutput("grep argv: /tmp/%s.log | tail -2" % testapp)
        original_argv = op.split("\n")[0].split(" ")[1:]
        cache_argv = op.split("\n")[1].split(" ")[1:]
        self.assert_(len(cache_argv) == ARGV_LIMIT, "Wrong number of arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))
        for i in range(ARGV_LIMIT):
            self.assert_(original_argv[i] == cache_argv[i], "Wrong arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))

        if(sighup):
            self.sighup_applauncherd()
            self._test_argv_booster_over_limit(btype, testapp, False)

    def test_signal_status_m(self):
        self._test_signal_status("fala_wl", "fala_wol")

    def test_signal_status_qml(self):
        self._test_signal_status("fala_qml_wl", "fala_qml_wol")

    def _test_signal_status(self, app_wl, app_wol):
        """
        Test that values of SigBlk, SigIgn and SigCgt in /proc/pid/status 
        is same for both boosted and non boosted applications
        """
        #Get status for non boosted apps
        debug("Restart home ")
        os.system("initctl restart xsession/mthome")
        time.sleep(5)
        debug("Get the co-ordinates of the application from the grid")
        st, op = commands.getstatusoutput("%s -a %s" %(GET_COORDINATE_SCRIPT, app_wol))
        time.sleep(2)
        pos = op.split("\n")[-1]
        debug("The co-ordinates of %s is %s" %(app_wol, pos))
        
        debug("Now tap to launch the application")
        os.system("%s -c %s -q" %(PIXELCHANHED_BINARY, pos))
        time.sleep(2)

        pid = wait_for_app(app_wol)
        self.assert_(pid != None, "The application was not launched")

        st, SigBlk_wol = commands.getstatusoutput("cat /proc/%s/status | grep SigBlk" %pid)
        st, SigIgn_wol = commands.getstatusoutput("cat /proc/%s/status | grep SigIgn" %pid)
        st, SigCgt_wol = commands.getstatusoutput("cat /proc/%s/status | grep SigCgt" %pid)

        debug("The SigBlk is %s, SigIgn is %s and SigCgt is %s for %s" %(SigBlk_wol, SigIgn_wol, SigCgt_wol, app_wol))

        kill_process(app_wol)
        time.sleep(2)
        
        #Get status for booster application
        debug("Restart home ")
        os.system("initctl restart xsession/mthome")
        time.sleep(5)
        debug("Get the co-ordinates of the application from the grid")
        st, op = commands.getstatusoutput("%s -a %s" %(GET_COORDINATE_SCRIPT, app_wl))
        time.sleep(2)
        pos = op.split("\n")[-1]
        debug("The co-ordinates of %s is %s" %(app_wl, pos))

        debug("Now tap to launch the application")
        os.system("%s -c %s -q" %(PIXELCHANHED_BINARY, pos))
        time.sleep(2)

        pid = wait_for_app(app_wl)
        self.assert_(pid != None, "The application was not launched")
        st, SigBlk_wl = commands.getstatusoutput("cat /proc/%s/status | grep SigBlk" %pid)
        st, SigIgn_wl = commands.getstatusoutput("cat /proc/%s/status | grep SigIgn" %pid)
        st, SigCgt_wl = commands.getstatusoutput("cat /proc/%s/status | grep SigCgt" %pid)

        debug("The SigBlk is %s, SigIgn is %s and SigCgt is %s for %s" %(SigBlk_wl, SigIgn_wl, SigCgt_wl, app_wl))

        kill_process(app_wl)
        time.sleep(2)
        
        self.assert_(SigBlk_wol == SigBlk_wl, "The SigBlk is not same for both apps")
        self.assert_(SigIgn_wol == SigIgn_wl, "The SigIgn is not same for both apps")
        self.assert_(SigCgt_wol == SigCgt_wl, "The SigCgt is not same for both apps")


    def test_launched_app_wm_class_m(self):
        """
        Test that launched application have correct WM_CLASS Xproperty booster m
        """
        #For booster-m MApplicationWindow from MComponentCache
        self._test_launched_app_wm_class_helper("m","fala_wl","-faulty","fala_wl",2)

        #For booster-m MApplicationWindow NOT from cache. 3 windows (1 for application + 1 is created by cache but not used)
        self._test_launched_app_wm_class_helper("m","fala_ft_hello","-window-not-from-cache","fala_ft_hello",3)

        #For booster-m multiple MApplicationWindow (3 windows + 1)
        self._test_launched_app_wm_class_helper("m","fala_multi-window","","fala_multi-window",4)

        #For booster-m multiple MApplicationWindow NOT from cache (3 windows + 1 + 1 created by cache but not used)
        self._test_launched_app_wm_class_helper("m","fala_multi-window","-window-not-from-cache","fala_multi-window",5)

    def test_launched_app_wm_class_d(self):
        """
        Test that launched application have correct WM_CLASS Xproperty booster d
        """
        #For booster-d QDeclarativeView from MDeclarativeCache
        self._test_launched_app_wm_class_helper("d","fala_qml_helloworld","-faulty","fala_qml_helloworld",2)

        #For booster-d QDeclarativeView NOT from cache (2 windows + 1 is created by cache but not used)
        self._test_launched_app_wm_class_helper("d","fala_qml_helloworld","-window-not-from-cache","fala_qml_helloworld",3)


    def test_launched_app_wm_class_e(self):
        """
        Test that launched application have correct WM_CLASS Xproperty booster e
        """
        #For booster-e MApplicationWindow from MComponentCache
        self._test_launched_app_wm_class_helper("e","fala_wl","-faulty","fala_wl",2)

        #For booster-e MApplicationWindow NOT from cache
        self._test_launched_app_wm_class_helper("e","fala_ft_hello","-window-not-from-cache","fala_ft_hello",2)

        #For booster-e multiple MApplicationWindow (3 windows + 1)
        self._test_launched_app_wm_class_helper("e","fala_multi-window","","fala_multi-window",4)



    def test_launched_app_wm_class_q(self):
        """
        Test that launched application have correct WM_CLASS Xproperty booster q
        """
        #For booster-q MApplicationWindow from MComponentCache
        self._test_launched_app_wm_class_helper("q","fala_wl","-faulty","fala_wl",2)

        #For booster-q MApplicationWindow NOT from cache
        self._test_launched_app_wm_class_helper("q","fala_ft_hello","-window-not-from-cache","fala_ft_hello",2)

        #For booster-q multiple MApplicationWindow (3 windows + 1)
        self._test_launched_app_wm_class_helper("q","fala_multi-window","","fala_multi-window",4)


    def _test_launched_app_wm_class_helper(self,btype,test_application,cmd_arguments,window_name,window_count):
        run_command = 'invoker --type=%s --no-wait %s %s' %(btype, test_application, cmd_arguments)
        p = run_cmd_as_user(run_command)
        time.sleep(5)

        pid = get_pid(test_application)
        self.assert_(pid != None, "Can't start application %s" %test_application)

        st, op = commands.getstatusoutput("xwininfo -root -tree| awk '/%s/ {print $1}'" %window_name)
        ids = op.split("\n")
        xProperties=[]
        for wid in ids:
                st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_CLASS/{print $3$4}'" %wid)
                xProperties.append(op1)

        kill_process(apppid=pid)

        #check that we catch exac number of windows
        numwind = len(ids)
        self.assert_(window_count == numwind, 'Got wrong number of windows: %s' %numwind)

        wm_class_xproperty_string = '"' + test_application + '","' + string.capwords(test_application) + '"'
        debug("Looking for '%s'" %wm_class_xproperty_string)

        for property in xProperties:
                self.assert_(op1 == wm_class_xproperty_string,'Application WM_CLASS 1 is incorrect: %s' %property)


    def test_q_booster_dont_have_GL_context(self):
        self._test_booster_dont_have_glcontext('q')

    def test_m_booster_dont_have_GL_context(self):
        self._test_booster_dont_have_glcontext('m')

    def test_e_booster_dont_have_GL_context(self):
        self._test_booster_dont_have_glcontext('e')

    def test_d_booster_dont_have_GL_context(self):
        self._test_booster_dont_have_glcontext('d')

    def _test_booster_dont_have_glcontext(self, btype, sighup = True):
        pid = get_pid('booster-%s'%btype)
        self.assert_(pid != None, "Process 'booster-%s' is not running"%btype)
        self.assert_(not has_GL_context(pid), "booster-%s has GL context!"%btype)
        if(sighup):
            self.sighup_applauncherd()
            self._test_booster_dont_have_glcontext(btype, False)
        
    def test_q_boosted_has_glcontext(self):
        self._test_boosted_app_has_glcontext(PREFERED_APP, 'q')
        
    def test_m_boosted_has_glcontext(self):
        self._test_boosted_app_has_glcontext(PREFERED_APP, 'm')
        
    def test_e_boosted_has_glcontext(self):
        self._test_boosted_app_has_glcontext(PREFERED_APP, 'e')

    def test_d_boosted_has_glcontext(self):
        self._test_boosted_app_has_glcontext(PREFERED_APP_QML, 'd')

    def _test_boosted_app_has_glcontext(self, testapp, btype, sighup = True):
        #check booster does not have GL context
        booster_pid = get_pid('booster-%s'%btype)
        self.assert_(booster_pid != None, "Process 'booster-%s' is not running"%btype)
        self.assert_(not has_GL_context(booster_pid), "booster-%s has GL context!"%btype)
        
        # run app which has GL context
        p = run_app_as_user_with_invoker(testapp, booster = btype, arg = '--no-wait')
        time.sleep(4)
        app_pid = get_pid(testapp)
        self.assert_(app_pid != None, "Process '%s' is not running"%testapp)
        self.assert_(app_pid == booster_pid, "Process '%s' is not a boosted app"%testapp)
        
        #check if app has GL context
        glcontext = has_GL_context(app_pid)
        kill_process(apppid=app_pid)
        self.assert_(glcontext, "%s does not have GL context!"%testapp)
        if(sighup):
            self.sighup_applauncherd()
            self._test_boosted_app_has_glcontext(testapp, btype, False)

    # detects bug
    def test_detect_booster_m_leaks_file_descriptor_when_invoker_is_using_app_directly(self) :
        self._test_detect_booster_leaks_file_descriptor_when_invoker_is_using_app_directly('m', 'fala_ft_hello')

    def test_detect_booster_d_leaks_file_descriptor_when_invoker_is_using_app_directly(self) :
        self._test_detect_booster_leaks_file_descriptor_when_invoker_is_using_app_directly('d', 'fala_qml_helloworld')

    def test_detect_booster_q_leaks_file_descriptor_when_invoker_is_using_app_directly(self) :
        self._test_detect_booster_leaks_file_descriptor_when_invoker_is_using_app_directly('q', 'fala_ft_hello')

    def test_detect_booster_e_leaks_file_descriptor_when_invoker_is_using_app_directly(self) :
        self._test_detect_booster_leaks_file_descriptor_when_invoker_is_using_app_directly('e', 'fala_ft_hello')

    def _test_detect_booster_leaks_file_descriptor_when_invoker_is_using_app_directly(self, boosterType, testAppName) :
        boosterName = 'booster-%s' %(boosterType)

        debug("Boster name: %s    app name: %s" %(boosterName, testAppName))

        # note that application is run by direct invacation not by calling the service
        run_command = 'invoker --single-instance --type=%s /usr/bin/%s' %(boosterType, testAppName)
        p = run_cmd_as_user(run_command)
        time.sleep(1)

        debug("First run command result is: %s" %(p))

        boosterPid = wait_for_app(boosterName, 10)
        self.assertNotEqual(boosterPid, None, "Test incoclusive, the booster is not ready - timeout.")

        startFileDescriptorCount = number_of_file_descriptors_for_pid(boosterPid)

        p = run_cmd_as_user(run_command)
        time.sleep(1)
        p = run_cmd_as_user(run_command)
        time.sleep(1)
        p = run_cmd_as_user(run_command)
        time.sleep(1)

        debug("Last run command result is: %s" %(p))

        self.assertEqual(boosterPid, get_pid(boosterName), "Test incoclusive, the booster has been changed.")
        
        endFileDescriptorCount = number_of_file_descriptors_for_pid(boosterPid)

        kill_process(testAppName)
        self.assertEqual(startFileDescriptorCount, endFileDescriptorCount, 
                         "State of booster should remain unchanged (no file descriptor leaks). File descriptor count changed by: %s." 
                             %(endFileDescriptorCount-startFileDescriptorCount))

    def test_check_applauncherd_sighup_effects_m(self) :
        self._test_check_applauncherd_sighup_effects(LAUNCHABLE_APPS,'m')

    def test_check_applauncherd_sighup_effects_d(self) :
        self._test_check_applauncherd_sighup_effects(LAUNCHABLE_APPS_QML,'d')

    def test_check_applauncherd_sighup_effects_e(self) :
        self._test_check_applauncherd_sighup_effects(LAUNCHABLE_APPS, 'e')

    def test_check_applauncherd_sighup_effects_q(self) :
        self._test_check_applauncherd_sighup_effects(LAUNCHABLE_APPS, 'q')

    def _test_check_applauncherd_sighup_effects(self, launchable_apps, btype) :
        """
        Test that applaucher-d is reinitilized afret sighup has been resived.
        This means
            - applaucherd is not killed just reinitilized 
            - killing old boosters and creating new one
            - not other child processes should be killed
        """
        daemonPid = wait_for_app('applauncherd', 10)

        appList = []

        try:
            for app in launchable_apps:
                p = run_app_as_user_with_invoker(app, booster = btype)
                pid = wait_for_app(app, timeout = 10, sleep = 1)
                if pid == None:
                    self.fail("%s was not launched using applauncherd")
                appList.append((pid, app))

            boosterPid = wait_for_app("booster-%s" %(btype))

            # send SIGHUP signal to applaucherd:
            kill_process(apppid=daemonPid, signum=1)
            time.sleep(5)

            self.assertNotEqual(wait_for_app("booster-%s" %(btype)), boosterPid, "Booster should be restarted by applaucherd with SIGHUP.")
            self.assertEqual(wait_for_app('applauncherd', 10), daemonPid, "applaucherd shouldn't be restarted after reciving SIGHUP.")

            for pid in appList :
                state = process_state(pid[0])
                self.assertNotEqual(state, None, 
                                    "Child process '%s' PID=%s has been killed after applaucherd recived SIGHUP!" %(pid[1], pid[0]))
        finally:
            for pid in appList:
                kill_process(apppid = pid[0])


# main
if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    check_prerequisites()
    tests = sys.argv[1:]
    try:
        mysuite = unittest.TestSuite(map(launcher_tests, tests))
    except:
        mysuite = unittest.TestSuite(map(daemon_handling, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)
    if not result.wasSuccessful():
        sys.exit(1)
    sys.exit(0)
