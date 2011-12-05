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

def has_GL_context(processId, tries=2):
    for i in range(tries):
        processMapsFile = open("/proc/" + processId + "/maps")
        processMapsData = processMapsFile.read()
        m = re.search(r"/dev/pvrsrvkm", processMapsData)
        processMapsFile.close()
        if(m != None):
            return True
        time.sleep(1)
    return False

class launcher_tests (CustomTestCase):
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
        wait_for_app('applauncherd')
        st_new, op = commands.getstatusoutput("initctl start xsession/applauncherd")
 
        self.assert_(st == 0, "Applauncherd do not start")
        self.assert_(st_new != 0, "Applauncherd even force start second time")

    def test_zombie_state_m(self):
        self._test_zombie_state(PREFERED_APP,'m')

    def test_zombie_state_qml(self):
        self._test_zombie_state(PREFERED_APP_QML,'d')

    def test_zombie_state_q(self):
        self._test_zombie_state(PREFERED_APP,'q')

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
        wait_for_process_end(prefered_app)

        process_handle = run_app_as_user_with_invoker(prefered_app,booster = btype)
        process_id1 = wait_for_app(prefered_app, 5)
        debug("The pid of %s id %s" %(prefered_app, process_id1))
        kill_process(prefered_app)
        wait_for_process_end(prefered_app)

        process_id2 = get_pid(prefered_app)
        debug("The pid of %s id %s" %(prefered_app, process_id1))

        self.assert_(process_id != process_id1 , "New Process not launched (zombie exists)")
        self.assert_(process_id2 == None , "Process still running")
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

        process_handle1 = run_app_as_user_with_invoker(prefered_app, booster=btype)

        # second call of invoker should return immediately
        for i in range(10) :
            process_handle1.poll()
            if process_handle1.returncode!=None :
                debug("Second call of invoker has returned code: %s (loop run: %s)" %(process_handle1.returncode, i))
                break
            time.sleep(1)

        process_id1 = wait_for_app(prefered_app)
        debug("PID of 2nd %s" % process_id1)

        st, pids = commands.getstatusoutput("pgrep -l %s" % prefered_app) #get all pids of the app
        kill_process(prefered_app)

        self.assert_( len(pids.split('\n')) == 1, "More then one instance of app is running: %s" %pids)
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

        startTime = time.time() + 30
        debug("Waiting for %s apps with prefix '%s' to start up in 30s..." %(len(launchable_apps), app_common_prefix))
        pid_list = None
        while time.time() < startTime+30 :
            process_id = get_pid(app_common_prefix)
            if process_id :
                pid_list = process_id.split('\n')
                if len(pid_list) == len(launchable_apps) :
                    break
            time.sleep(2)
        debug("Waiting process took %.1fs." %(time.time()-startTime))

        for pid in pid_list:
            kill_process(apppid=pid)

        self.assert_(len(pid_list) == len(launchable_apps), "Not All Applications were launched using launcher")

        if(sighup):
            self.sighup_applauncherd()
            self._test_launch_multiple_apps_cont(launchable_apps, app_common_prefix, btype, False)


    def test_fd_booster_m(self, sighup = True):
        """
        File descriptor test for booster-m
        """
        self._check_changed_fd_count("m", PREFERED_APP)

    def test_fd_booster_q(self, sighup = True):
        """
        File descriptor test for booster-q
        """
        self._check_changed_fd_count("q", PREFERED_APP)

    def test_fd_booster_d(self, sighup = True):
        """
        File descriptor test for booster-d
        """
        self._check_changed_fd_count("d", PREFERED_APP_QML)

    def test_fd_booster_e(self, sighup = True):
        """
        File descriptor test for booster-e
        """
        self._check_changed_fd_count("e", PREFERED_APP)

    def _check_changed_fd_count(self, btype, app_name, sighup = True):
        """
        To test that file descriptors are closed before calling application main
        """
        debug("kill %s if it already exists" % app_name)
        kill_process(app_name)
        wait_for_process_end(app_name)

        #get fd of booster before launching application
        debug("get fd of booster before launching application")
        pid = wait_for_app('booster-%s'%btype)
        init = get_fd_dict(pid)
        debug("\nThe initial file descriptors are : %s\n" %init)
    
        #launch application using booster
        debug("launch %s using booster" % app_name)
        st = os.system('invoker --type=%s --no-wait %s' % (btype, app_name))
        self.assert_(st == 0, "failed to start %s,%s" % (app_name,st))
    
        # wait for new booster and app to start
        pid1 = wait_for_app(app_name)
    
        #get fd of booster after launching the application
        debug("get fd of booster after launching the application")
        final = get_fd_dict(pid)
        debug("\nThe final file descriptors are : %s\n" %final)
        kill_process(app_name) 
        self.assert_(pid == pid1, "application did not start with same booster")
    
        mykeys = init.keys()
        count = 0
    
        for key in mykeys:
            try:
                if init[key] != final[key]:
                    count = count + 1
            except KeyError:
                print "some key in init is not in final" 

        debug("The number of changed file descriptors %d" %count)
        self.assert_(count != 0, "None of the file descriptors were changed")

        if(sighup):
            self.sighup_applauncherd()
            self._check_changed_fd_count(btype, app_name, False)

    def test_restart_booster(self, sighup = True):
        """
        Test that booster is restarted if it is killed 
        """
        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        epid, dpid, qpid, mpid = get_booster_pid(timeout=10)
        self.assert_(qpid != None, "No booster process running")
        self.assert_(mpid != None, "No booster process running")
        self.assert_(dpid != None, "No booster process running")
        self.assert_(epid != None, "No booster process running")

        #Kill the booster processes
        debug("Kill the booster processes")
        kill_process(apppid=qpid)
        kill_process(apppid=mpid)
        kill_process(apppid=dpid)
        kill_process(apppid=epid)
        
        #wait for the boosters to be restarted
        epid_new, dpid_new, qpid_new, mpid_new = get_booster_pid(timeout=10)

        #check that the new boosters are started
        debug("check that the new boosters are started")
        self.assert_(qpid_new != None, "No booster process running")
        self.assert_(qpid_new != qpid, "booster process was not killed")

        self.assert_(mpid_new != None, "No booster process running")
        self.assert_(mpid_new != mpid, "booster process was not killed")

        self.assert_(dpid_new != None, "No booster process running")
        self.assert_(dpid_new != dpid, "booster process was not killed")

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
        boosterTypes = ('e', 'd', 'q', 'm')
        
        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        for bType in boosterTypes :
            bpid = get_pid('booster-%s' %bType)
            self.assertNotEqual(bpid, None, "No booster process running")
            self.assertEqual(len(bpid.split("\n")), 1, "multiple instances of booster-%s running: %s" %(bType, bpid.split()))

        #stop applauncherd
        stop_applauncherd()
 
        #wait for the applauncherd to be closed
        wait_for_process_end('applauncherd')

        #check that the none of the booster is running
        debug("check that the none of the booster is running")
        
        for bType in boosterTypes :
            bpid = get_pid('booster-%s' %bType)
            self.assertEqual(bpid, None, "booster-%s still running" %bType)

        #Now start the applauncherd
        start_applauncherd()
        
        #wait for the boosters to be restarted
        boostersPids = get_booster_pid(timeout = 40)

        #get the pids of boosters and make sure they are running
        debug("get the pids of boosters and make sure they are running")
        for bType in boosterTypes :
            bpid = get_pid('booster-%s' %bType)
            self.assertNotEqual(bpid, None, "No booster process running")
            self.assertEqual(len(bpid.split("\n")), 1, "multiple instances of booster-%s running: %s" %(bType, bpid.split()))

        #Now kill applauncherd
        debug("Now kill applauncherd")
        kill_process('applauncherd')
        
        #wait for the boosters to be restarted
        newBoostersPids = get_booster_pid(timeout = 10)
        
        for i in range(len(boosterTypes)) :
            self.assertNotEqual(newBoostersPids[i], None, "No booster-%s process running" %boosterTypes[i])
            self.assertNotEqual(newBoostersPids[i], boostersPids[i], "booster-%s process was not restarted" %boosterTypes[i])

    def test_booster_pid_change(self, sighup = True):
        """
        Test that application pid changes to the booster 'x' when application 
        is started with invoker --type='x'
        """
        for i in xrange(3):
            for bType in ('m', 'd', 'e', 'q') :
                #Launching application with booster-m
                bpid = wait_for_app('booster-%s' %bType, timeout = 10)
                p = run_app_as_user_with_invoker(bType!='d' and PREFERED_APP or PREFERED_APP_QML, booster = bType, arg = '--no-wait')
                app_pid = wait_for_app(bType!='d' and 'fala_ft_hello' or 'fala_qml_helloworld', timeout = 10)
                bpid_new = wait_for_app('booster-%s' %bType, timeout = 10)
                self.assertNotEqual(app_pid, None, "Application is not running")
                self.assertEqual(app_pid, bpid, "Application is not assigned the booster-%s pid" %bType)
                self.assertNotEqual(bpid_new, None, "No booster process running")
                self.assertNotEqual(bpid_new, bpid, "booster-%s process did not receive the new pid" %bType)
                kill_process(apppid = app_pid)

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
            wait_for_process_end(app_name)

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
        for bType in ('m', 'd', 'q', 'e') :
            appName = bType != 'd' and 'fala_wl' or 'fala_qml_helloworld'
            p = run_cmd_as_user('invoker --type=%s --no-wait %s -faulty' %(bType, appName))
            pid = wait_for_app(appName, timeout = 40)
            self.assert_(pid, "Fail to launch '%s' application" %appName)
            try :
                #Check though the process list
                st, op = commands.getstatusoutput('cat /proc/%s/cmdline' %pid)
                self.assertEqual(op.split('\0')[0], appName, 'Application name in the process list is incorrect')

                #check through the window property
                winName = bType == 'd' and "Applauncherd QML testapp" or "Applauncherd testapp"
                windows = wait_for_windows(winName)
                self.assert_(windows, "Fail to find any window: %s!" %winName)
                self.assertEqual(len(windows), 1, "To many windows '%s' has been found: %s!" %(winName, windows))
    
                st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_COMMAND/{print $4}'" %(windows[0]))
                self.assertEqual(op1.split(",")[0], '"%s"'%appName, 'Application name in the window property is incorrect: "%s" vs ""%s""' %(op1.split(",")[0], appName))
            finally:
                kill_process(appname=appName)

        if(sighup):
            self.sighup_applauncherd()
            self.test_launched_app_name(False)


    def test_unix_signal_handlers(self, sighup = True):
        """
        Test unixSignalHandlers by killing booster-m,d,q,e with signal hup
        """

        self._test_sighup_booster('m')
        self._test_sighup_booster('d')
        self._test_sighup_booster('q')
        self._test_sighup_booster('e')

        if(sighup):
            self.sighup_applauncherd()
            self.test_unix_signal_handlers(False)

    def _test_sighup_booster(self, btype):
        pid = wait_for_app('booster-%s' % btype)
        st, op = commands.getstatusoutput('kill -hup %s' % pid)
        time.sleep(2)
        pid_new = wait_for_app('booster-%s' % btype)
        self.assert_(pid != pid_new, "booster-%s pid is not changed" % btype)

    def test_qttas_load_booster_d(self):
        self._test_qttas_load_booster(PREFERED_APP_QML, 'd')

    def test_qttas_load_booster_m(self):
        self._test_qttas_load_booster(PREFERED_APP, 'm')

    def test_qttas_load_booster_q(self):
        self._test_qttas_load_booster(PREFERED_APP, 'q')

    def _test_qttas_load_booster(self, testapp, btype, sighup = True):
        """
        To test invoker that qttestability plugin is loaded with -testability argument for booster-d
        """
        for i in range(2):
                debug("Running cycle %s" %i)
                p = run_app_as_user_with_invoker("%s -testability" %testapp, booster = btype)
                pid = wait_for_app(testapp)
                self.assert_(pid != None, "Can't start application %s" %testapp)

                for j in range(3) :
                    st_tas, op_tas = commands.getstatusoutput("grep -c libtestability.so /proc/%s/maps" %pid)
                    if st_tas == 0 :
                        break
                    time.sleep(1)
                debug("The value of status is %d" %st_tas)
                debug("The value of output is %s" %op_tas)

                for j in range(3) :
                    st_tas_plug, op_tas_plug = commands.getstatusoutput("grep -c libqttestability.so /proc/%s/maps" %pid)
                    if st_tas_plug == 0 :
                        break
                    time.sleep(1)

                debug("The value of status is %d" %st_tas_plug)
                debug("The value of output is %s" %op_tas_plug)

                kill_process(testapp)
                # wait_for_process_end(testapp)
                time.sleep(2)

                self.assert_(st_tas == 0,"libtestability.so not loaded")
                self.assert_(st_tas_plug == 0,"libqttestability.so not loaded")


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

                p = subprocess.Popen(cmd, shell = False, stdout = DEV_NULL, stderr = DEV_NULL, preexec_fn=permit_sigpipe)

                pid = wait_for_app(testapp)
                self.assert_(pid != None, "Can't start application %s" %testapp)

                st_tas, op_tas = commands.getstatusoutput("grep -c libtestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas)
                debug("The value of output is %s" %op_tas)

                st_tas_plug, op_tas_plug = commands.getstatusoutput("grep -c libqttestability.so /proc/%s/maps" %pid)
                debug("The value of status is %d" %st_tas_plug)
                debug("The value of output is %s" %op_tas_plug)

                kill_process(testapp)
                wait_for_process_end(testapp)

                self.assert_(st_tas == 0,"libtestability.so not loaded")
                self.assert_(st_tas_plug == 0,"libqttestability.so not loaded")

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
        logFileName = "/tmp/%s.log" % testapp
        if os.path.isfile(logFileName):
            os.system("rm %s" % logFileName)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s %s/%s' % (btype, path, testapp))
        pid = wait_for_app(testapp)
        self.assert_(pid != None, "The application was not launched")
        try :
            dirPathLine = self.waitForAssertLogFileContains(logFileName,
                                                            "applicationDirPath:",
                                                            "Application directory path not found in log file: %s" %logFileName,
                                                            timeout = 5)

            fullPathLine = self.waitForAssertLogFileContains(logFileName,
                                                            "applicationFilePath:",
                                                            "Application path not found in log file: %s" %logFileName,
                                                            timeout = 5)
            dirpath = dirPathLine.split(" ")[2]
            filepath = fullPathLine.split(" ")[2]
            self.assert_(dirpath == path, "Wrong dirPath: %s" % dirpath)
            self.assert_(filepath == "%s/%s" % (path, testapp), "Wrong filePath: %s" % filepath)

        finally :
            kill_process(apppid=pid)

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
        logFileName = "/tmp/%s.log" % testapp
        if os.path.isfile(logFileName):
            os.system("rm %s" % logFileName)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s /usr/bin/%s --log-args 0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t' % (btype, testapp))
        pid = wait_for_app(testapp)
        self.assert_(pid != None, "The application was not launched")

        try:
            original_argv = self.waitForAssertLogFileContains(logFileName, "argv:",
                                                              "Arguments are not found in log file: %s" %logFileName,
                                                              timeout = 5)
            cache_argv = self.waitForAssertLogFileContains(logFileName, "argv:",
                                                           "Arguments are not found in log file: %s" %logFileName,
                                                           timeout = 5, findCount=2)

            self.assert_(original_argv == cache_argv, "Wrong arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))
        finally:
            kill_process(apppid=pid)
        
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
        logFileName = "/tmp/%s.log" % testapp
        if os.path.isfile(logFileName):
            os.system("rm %s" % logFileName)
        if get_pid(testapp)!= None:
            kill_process(testapp)
        p = run_cmd_as_user('invoker --type=%s /usr/bin/%s --log-args 0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t u v w x y z' % (btype, testapp))
        pid = wait_for_app(testapp)
        self.assert_(pid != None, "The application was not launched")
        try:
            debug("get arguments from log file")
            original_argv = self.waitForAssertLogFileContains(logFileName, "argv:",
                                                              "Arguments are not found in log file: %s" %logFileName,
                                                              timeout = 5)
            cache_argv = self.waitForAssertLogFileContains(logFileName, "argv:",
                                                           "Arguments are not found in log file: %s" %logFileName,
                                                           timeout = 5, findCount=2)
            original_argv = original_argv.split(" ")[1:]
            cache_argv = cache_argv.split(" ")[1:]
            self.assert_(len(cache_argv) == ARGV_LIMIT, "Wrong number of arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))
            for i in range(ARGV_LIMIT):
                self.assert_(original_argv[i] == cache_argv[i], "Wrong arguments passed.\nOriginal: %s\nCached: %s" % (original_argv, cache_argv))
        finally:
            kill_process(apppid=pid)

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
        wait_for_process_end(app_wol)
        
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
        wait_for_process_end(app_wl)
        
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
        #Number of windows changed to 2 QDeclarativeView actually is no longer created in cache
        self._test_launched_app_wm_class_helper("d","fala_qml_helloworld","-window-not-from-cache","fala_qml_helloworld",2)


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

        pid = wait_for_app(test_application)
        self.assert_(pid != None, "Can't start application %s" %test_application)

        windowsIds = wait_for_windows(window_name, minCount = window_count)
        xProperties=[]
        for wid in windowsIds:
                st, op1 = commands.getstatusoutput("xprop -id %s | awk '/WM_CLASS/{print $3$4}'" %wid)
                xProperties.append(op1)

        kill_process(apppid=pid)

        #check that we catch exac number of windows
        numwind = len(windowsIds)
        self.assertEqual(window_count, numwind, 'Got wrong number of windows: %s insted of: %s\nids = %s' 
                         %(numwind, window_count, windowsIds))

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
        app_pid = wait_for_app(testapp)
        self.assert_(app_pid != None, "Process '%s' is not running"%testapp)
        self.assert_(app_pid == booster_pid, "Process '%s' is not a boosted app"%testapp)
        
        #check if app has GL context
        glcontext = has_GL_context(app_pid, 3)
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

        # wait for first use of booster
        wait_for_app(testAppName)

        debug("First run command result is: %s" %(p))

        # wait for NEW booster
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
            - applauncherd is not killed just reinitilized 
            - killing old boosters and creating new one
            - not other child processes should be killed
        """
        daemonPid = wait_for_single_applauncherd()

        appList = []

        try:
            for app in launchable_apps:
                p = run_app_as_user_with_invoker(app, booster = btype)
                pid = wait_for_app(app, timeout = 10, sleep = 1)
                if pid == None:
                    self.fail("%s was not launched using applauncherd" % app)
                appList.append((pid, app))

            boosterPid = wait_for_app("booster-%s" %(btype))

            # send SIGHUP signal to applaucherd:
            oldBoosters = get_pid('booster')
            kill_process(apppid=daemonPid, signum=1)
            wait_for_new_boosters(oldBoosters)

            self.assertNotEqual(wait_for_app("booster-%s" %(btype)), boosterPid, "Booster should be restarted by applauncherd with SIGHUP.")
            self.assertEqual(wait_for_single_applauncherd(), daemonPid, "applauncherd shouldn't be restarted after reciving SIGHUP.")

            for pid in appList :
                state = process_state(pid[0])
                self.assertNotEqual(state, None, 
                                    "Child process '%s' PID=%s has been killed after applauncherd recived SIGHUP!" %(pid[1], pid[0]))
        finally:
            for pid in appList:
                kill_process(apppid = pid[0])

    def _test_reexec_remove_test_package(self, packageName):
        st, op = commands.getstatusoutput('dpkg -r %s' %(packageName))
        if st!=0 :
            debug('Fail to remove "%s" package! Returned error code is: %s. '
                  'Check remove log:\n%s\n<<<<<<remove log.' %(packageName, st, op))
        else :
            debug('Uninstallation of "%s" was successful' %(packageName))

    def _wait_and_check_for_new_boosters(self, oldBoosterPids, oldApplauncherPid):
        oldBoosters = set(oldBoosterPids)
        newBoosterPids = None
        for i in range(3) :
            tmp = get_booster_pid()
            if len(oldBoosters & set(tmp))==0 :
                newBoosterPids = tmp
            time.sleep(1)

        num_of_same_pids = len(set(oldBoosterPids) & set(newBoosterPids))
        self.assertEqual(num_of_same_pids, 0, "Boosters pids did not change")

        newApplauncherPid = wait_for_single_applauncherd()
        self.assertEqual(oldApplauncherPid, newApplauncherPid,
                         "applauncherd has crashed. Its pid has changed from "
                         "%s to %s." %(oldApplauncherPid, newApplauncherPid))
        return (newApplauncherPid, newBoosterPids)

    def test_reexec_when_file_in_usrlib_modified(self) :
        """
        Checks that when a file is installed/modified/removed from /usr/lib
        trigger is activated and sighup is send to applauncherd.
        """

        packageFileName = '/usr/share/fala_images/applauncherd-usrlib-test_1.0_armel.deb'
        updatePackageFileName = '/usr/share/fala_images/applauncherd-usrlib-test_1.1_armel.deb'
        packageName = 'applauncherd-usrlib-test'

        applauncherPid = wait_for_single_applauncherd()
        boosterPids = get_booster_pid()

        #check that test package is NOT installed
        installed = isPackageInstalled(packageName)
        if (installed) :
            debug('Tested package "%s" is installed, removing it.' %(packageName))
            self._test_reexec_remove_test_package(packageName)
            wait_for_new_boosters(boosterPids)
            applauncherPid = wait_for_single_applauncherd()
            boosterPids = get_booster_pid()

        try:
            #real test starts here
            debug('Installing test package "%s"...' %(packageName))
            debug('This installs new file to /usr/lib/')
            st, op = commands.getstatusoutput('dpkg -i %s' %(packageFileName))
            self.assertEqual(st, 0, 'Installation of "%s" failed see:\n%s'  %(packageName, op))
            debug('...Installation was successful.')
            applauncherPid, boosterPids = self._wait_and_check_for_new_boosters(boosterPids, applauncherPid)

            debug('Upgrading test package to "%s"' % (updatePackageFileName))
            debug('This modifies the file existing in /usr/lib/ to a new verison')
            st, op = commands.getstatusoutput('dpkg -i %s' %(updatePackageFileName))
            self.assertEqual(st, 0, 'Upgrading of "%s" failed see:\n%s'  %(packageName, op))
            debug('...Upgrading was successful.')
            applauncherPid, boosterPids = self._wait_and_check_for_new_boosters(boosterPids, applauncherPid)

            debug('Downgrading test package to "%s"' % (packageFileName))
            debug('This modifies the file existing in /usr/lib/ to a old verison')
            st, op = commands.getstatusoutput('dpkg -i %s' %(packageFileName))
            self.assertEqual(st, 0, 'Downgrading of "%s" failed see:\n%s'  %(packageName, op))
            debug('...Downgrading was successful.')
            applauncherPid, boosterPids = self._wait_and_check_for_new_boosters(boosterPids, applauncherPid)

        except:
            debug('Restoring system to initial state: removing test package after test was asserted.')
            self._test_reexec_remove_test_package(packageName)
            raise

        # now remove the package
        debug('Uninstalling test package "%s"...' %(packageName))
        debug('This deletes the file from /usr/lib')
        self._test_reexec_remove_test_package(packageName)
        applauncherPid, boosterPids = self._wait_and_check_for_new_boosters(boosterPids, applauncherPid)

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
