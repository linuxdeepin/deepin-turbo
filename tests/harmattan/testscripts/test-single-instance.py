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
These test cases test the single instance functionality with
and without applauncherd/invoker.

Requirements:
1. DISPLAY environment variable must be set correctly.
2. DBus session bus must be running.
3. DBus session bus address must be stored in /tmp/session_bus_address.user.
4. Given application supports launcher with .launcher binary in /usr/bin/.
5. launcher application should be installed.
6. single-instance app should be installed

Authors:  ext-oskari.timperi@nokia.com
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

using_scratchbox = False

def check_prerequisites():
    if os.getenv('DISPLAY') == None:
        error("DISPLAY is not set. Check the requirements.")
        
    if os.getenv('DBUS_SESSION_BUS_ADDRESS') == None:
        error("DBUS_SESSION_BUS_ADDRESS is not set.\n" +
              "You probably want to source /tmp/session_bus_address.user")

class SingleInstanceTests(unittest.TestCase):

    def setUp(self):
        if daemons_running():
            stop_daemons()
            self.START_DAEMONS_AT_TEARDOWN = True
        else:
            self.START_DAEMONS_AT_TEARDOWN = False

        if wait_for_app('applauncherd') == None:
            start_applauncherd()
        #setup here
        debug("Executing SetUp")

    def tearDown(self):
        #teardown here
        debug("Executing TearDown")
        if wait_for_app('applauncherd') == None:
            start_applauncherd()
        wait_for_single_applauncherd()

        if self.START_DAEMONS_AT_TEARDOWN:
            start_daemons()

    def sighup_applauncherd(self): 
        same_pid, booster_status = send_sighup_to_applauncherd()
        self.assert_(same_pid, "Applauncherd has new pid after SIGHUP")
        self.assert_(booster_status, "Atleast one of the boosters is not restarted")

    #Testcases
    def minimize(self, app = None, pid = None):
        # get window id
        if pid:
            st, op = commands.getstatusoutput('fala_windowid %s' % pid)
        else:
            st, op = commands.getstatusoutput("xwininfo -root -tree| grep 854\
                    |awk '/%s/ {print $1}'" %(basename(app)))
        wid = op.splitlines()

        self.assert_(len(wid) > 0, "no windows found ")

        # minimize all window id's reported
        for w in wid:
            run_cmd_as_user('xsendevent iconify %s' % w)

        time.sleep(2)
        return op 

    def single_instance_window_raise(self, si_cmd):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Minimize it with xsendevent
        # 3. Start another multi-instance application with single-instance binary
        # 4. Check that there in only one app and its application pid is the same than with 1. launch
        # 5. Check that window is raised with correct pid (from log files written by test application)
        # 6. Close the application
        # 7. Start the multi-instance application with single-instance binary
        # 8. Check that the pid has changed

        app = '/usr/bin/fala_multi-instance'

        kill_process(app)

        # start for the first time
        p1 = run_cmd_as_user('%s %s foo' % (si_cmd, app))

        time.sleep(2)

        # get pid
        pid1 = wait_for_app(app)
        self.assert_(pid1 != None, "%s was not started")
        pid1 = pid1.splitlines()[0]

        self.minimize(pid = pid1)

        # start for the second time
        p2 = run_cmd_as_user('%s %s bar' % (si_cmd, app))

        time.sleep(2)

        # check  that there's only one instance running
        pids = wait_for_app(app)
        self.assert_(pids != None, "%s was not started" % app)
        pids = pids.splitlines()
        self.assert_(len(pids) == 1, "multiple instances of %s running" % app)

        # check that the pid is the same as the pid with 1st launch
        self.assert_(int(pid1) == int(pids[0]), "pid was changed %s => %s" % (pid1, pids[0]))

        kill_process(app, signum = 15)

        # start for the third time and see that the pid has changed
        run_cmd_as_user('%s %s baz' % (si_cmd, app))

        time.sleep(2)

        pids = wait_for_app(app)
        self.assert_(pids != None, "%s was not started")
        pid2 = pids.splitlines()[0]

        self.assert_(int(pid1) != int(pid2), "pid was not changed")

        kill_process(app, signum = 15)


    def get_pid_full(self, app):
        p = subprocess.Popen(['pgrep', '-f', app], shell = False,
                             stdout = subprocess.PIPE, stderr = DEV_NULL)

        op = p.communicate()[0]

        debug("The New Pid of %s is %s:" %(app, op.strip()))
        if p.wait() == 0:
            return op.strip()
        
        return None

    def wait_for_app(self, app = None, timeout = 10, sleep = 1):
        """
        Waits for an application to start. Checks periodically if
        the app is running for a maximum wait set in timeout.

        Returns the pid of the application if it was running before
        the timeout finished, otherwise None is returned.
        """

        pid = None
        start = time.time()

        while pid == None and time.time() < start + timeout:
            pid = self.get_pid_full(app)

            if pid != None:
                break

            debug("waiting %s secs for %s" % (sleep, app))

            time.sleep(sleep)

        return pid

    def single_instance_window_raise_with_script(self, si_cmd):
        # For Bug#250404
        # 1. Start the multi-instance application from script 
        #    -check the pid of started app
        # 2. Minimize it with xsendevent
        # 3. Start another multi-instance application with single-instance binary
        # 4. Check that there in only one app and its application pid is the same than with 1. launch
        # 5. Check that window is raised with correct pid (from log files written by test application)
        # 6. Close the application

        app = '/usr/bin/fala_focus'
        p = self.get_pid_full('/usr/bin/python /usr/bin/fala_focus')
        if p != None:
            kill_process(apppid=p, signum = 15)

        # start for the first time
        p1 = run_cmd_as_user('%s %s' % (si_cmd, app))

        time.sleep(4)

        # get pid
        pid1 = self.wait_for_app('/usr/bin/python /usr/bin/fala_focus')
        self.assert_(pid1 != None, "%s was not started")

        wid = self.minimize(app)
        st, op = commands.getstatusoutput("xprop -id %s | awk '/window state/ {print $3}'" %wid)
        self.assert_(op == 'Iconic', "%s was not minimized" % app)

        # start for the second time
        p2 = run_cmd_as_user('%s %s' % (si_cmd, app))

        time.sleep(4)

        # check  that there's only one instance running
        pids= self.wait_for_app('/usr/bin/python /usr/bin/fala_focus')
        st, op = commands.getstatusoutput("xprop -id %s | awk '/window state/ {print $3}'" %wid)
        self.assert_(op == 'Normal', "%s was not raised" % app)

        self.assert_(pids != None, "%s was not started" % app)

        # check that the pid is the same as the pid with 1st launch
        self.assert_(int(pid1) == int(pids), "pid was changed %s => %s" % (pid1, pids))

        kill_process(apppid=pids, signum = 15)

    def single_instance_and_non_single_instance(self, si_cmd):
        # 1. Start the multi-instance application without single-instance binary
        # 2. Start another multi-instance application with single-instance binary
        # 3. Check that both  application pids exist

        app = '/usr/bin/fala_multi-instance'

        kill_process(app)

        run_cmd_as_user(app + " foo")
        time.sleep(2)

        run_cmd_as_user("%s %s bar" % (si_cmd, app))
        time.sleep(2)

        pids = get_pid(app)
        self.assert_(pids != None, "nothing was started")
        pids = pids.splitlines()

        self.assert_(len(pids) == 2, "application count incorrect (%d)" % len(pids))

        kill_process(app, signum = 15)

    def single_instance_stress_test(self, si_cmd):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Minimize it with xsendevent
        # 3. Start the multi-instance app with single-instance binary 20 times very fast
        #    - check the return value of each launch is zero
        # 4. Check that there in only one application pid and it has the same than with 1. launch
        # 5. Check that window is raised with correct pid (from log files written by test application)

        app = '/usr/bin/fala_multi-instance'

        kill_process(app)

        try:
            os.remove('/tmp/fala_multi-instance.log')
        except:
            pass

        run_cmd_as_user("%s %s foo" % (si_cmd, app))

        time.sleep(2)

        pid1 = wait_for_app(app)
        self.assert_(pid1 != None, "%s was not started" % app)
        pid1 = pid1.splitlines()[0]

        self.minimize(pid = pid1)

        for i in range(20):
            p = run_cmd_as_user("%s %s bar%d" % (si_cmd, app, i))
            rc = p.wait()
            self.assert_(rc == 0 or rc == 250,
                         "[%d] return code was %d, should have been 0" % (i, p.returncode))

        pid = wait_for_app(app)
        self.assert_(pid != None, "%s was not started" % app)
            
        pid = pid.splitlines()
        self.assert_(len(pid) == 1, "%d instances running, should be 1" % len(pid))

        self.assert_(pid[0] == pid1, "pid is not the same as the first pid")

        kill_process(app, signum = 15)

        with open('/tmp/fala_multi-instance.log') as f:
            for line in f:
                if line.find('Maximized'):
                    # time pid event
                    # 1277997459568 4180 Maximized
                    pid = line.split()[1]

                    self.assert_(pid == pid1, "wrong app was raised")

                    break
        
    def single_instance_abnormal_lock_release(self, si_cmd):
        # 1. Start the multi-instance application with single-instance binary
        #    -check the pid of started app
        # 2. Kill the application with -9
        # 3. Start the multi-instance application with single-instance binary
        # 4. Check that application can be started and pid has changed

        app = '/usr/bin/fala_multi-instance'

        kill_process(app)

        run_cmd_as_user('%s %s foo' % (si_cmd, app))

        time.sleep(2)

        pid = wait_for_app(app)
        self.assert_(pid != None, "%s was not started" % app)
        pid = pid.splitlines()[0]

        kill_process(app, signum = 9)

        run_cmd_as_user('%s %s bar' % (si_cmd, app))

        time.sleep(2)

        pid2 = wait_for_app(app)
        self.assert_(pid2 != None, "%s was not started" % app)
        pid2 = pid2.splitlines()[0]

        kill_process(app, signum = 15)

        self.assert_(pid != pid2, "pid was not changed")

    def test_single_instance_window_raise_without_invoker(self):
        self.single_instance_window_raise('single-instance')

    def test_single_instance_and_non_single_instance_without_invoker(self):
        self.single_instance_and_non_single_instance('single-instance')

    def test_single_instance_stress_test_without_invoker(self):
        self.single_instance_stress_test('single-instance')

    def test_single_instance_abnormal_lock_release_without_invoker(self):
        self.single_instance_abnormal_lock_release('single-instance')

    def test_single_instance_window_raise_with_invoker(self, sighup = True):
        self.single_instance_window_raise('invoker --type=m --single-instance')
        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_window_raise_with_invoker(False)

    def test_single_instance_and_non_single_instance_with_invoker(self, sighup = True):
        self.single_instance_and_non_single_instance('invoker --type=m --single-instance')
        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_and_non_single_instance_with_invoker(False)

    def test_single_instance_stress_test_with_invoker(self, sighup = True):
        self.single_instance_stress_test('invoker --type=m --single-instance')
        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_stress_test_with_invoker(False)

    def test_single_instance_abnormal_lock_release_with_invoker(self, sighup = True):
        self.single_instance_abnormal_lock_release('invoker --type=m --single-instance')
        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_abnormal_lock_release_with_invoker(False)

    def test_single_instance_window_raise_with_script(self, sighup = True):
        self.single_instance_window_raise_with_script('invoker --single-instance --type=e')
        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_window_raise_with_script(False)

    def test_single_instance_windowless_w_invoker(self, sighup = True):
        """
        To test that starting twice windowless application with invoker with single-instance
        will report error in syslog
        """
        app = '/usr/bin/fala_windowless'
        #remove already running fala_windowless if any
        kill_process(app)
        #check applauncherd is started
        pid_ad = wait_for_single_applauncherd()
        self.assert_(pid_ad != None, "Applauncherd was not started")
        #run fala_windowless first time
        run_app_as_user_with_invoker(app, booster = 'm', arg = "--single-instance")
        pid1 = wait_for_app(app)
        self.assert_(pid1 != None, "%s was not started." % app)
        #count previous error messages related to single instance application start attempts
        st, op = commands.getstatusoutput('grep -c "]: Booster: Can\'t activate existing instance of the application!" /var/log/syslog ')
        debug("The errors count in syslog is: %s" %op)
        #run fala_windowless second time	
        run_app_as_user_with_invoker(app, booster = 'm', arg = "--single-instance")
        time.sleep(5)
        pid2 = get_pid(app)
        #count error messages in sislog once again
        st1, op1 = commands.getstatusoutput('grep -c "]: Booster: Can\'t activate existing instance of the application!" /var/log/syslog ')
        debug("The errors count in syslog is: %s" %op1)
        #cleanup
        pid_list = pid2.split()
        for pid in pid_list:
            kill_process(apppid=pid)
        #check only one instance has been started
        self.assert_(len(pid_list) == 1, "Second instance of single-instance app has been started.");
        #check +1 error message found
        self.assert_(int(op) + 1 == int(op1), "No errror has been logged to syslog for windowless single instance app.")

        if(sighup):
            self.sighup_applauncherd()
            self.test_single_instance_windowless_w_invoker(False)

if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    check_prerequisites()

    restart_applauncherd()

    time.sleep(5)

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(SingleInstanceTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
