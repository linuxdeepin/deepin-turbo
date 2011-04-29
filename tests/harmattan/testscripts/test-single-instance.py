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

    #Testcases
    def minimize(self, pid):
        # get window id
        st, op = commands.getstatusoutput('fala_windowid %s' % pid)
        wid = op.splitlines()

        self.assert_(len(wid) > 0, "no windows found for pid %s" % pid)

        # minimize all window id's reported
        for w in wid:
            run_cmd_as_user('xsendevent iconify %s' % w)

        time.sleep(2)

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
        pid1 = get_pid(app)
        self.assert_(pid1 != None, "%s was not started")
        pid1 = pid1.splitlines()[0]

        self.minimize(pid1)

        # start for the second time
        p2 = run_cmd_as_user('%s %s bar' % (si_cmd, app))

        time.sleep(2)

        # check  that there's only one instance running
        pids = get_pid(app)
        self.assert_(pids != None, "%s was not started" % app)
        pids = pids.splitlines()
        self.assert_(len(pids) == 1, "multiple instances of %s running" % app)

        # check that the pid is the same as the pid with 1st launch
        self.assert_(int(pid1) == int(pids[0]), "pid was changed %s => %s" % (pid1, pids[0]))

        kill_process(app, signum = 15)

        # start for the third time and see that the pid has changed
        run_cmd_as_user('%s %s baz' % (si_cmd, app))

        time.sleep(2)

        pids = get_pid(app)
        self.assert_(pids != None, "%s was not started")
        pid2 = pids.splitlines()[0]

        self.assert_(int(pid1) != int(pid2), "pid was not changed")

        kill_process(app, signum = 15)

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

        pid1 = get_pid(app)
        self.assert_(pid1 != None, "%s was not started" % app)
        pid1 = pid1.splitlines()[0]

        self.minimize(pid1)

        for i in range(20):
            p = run_cmd_as_user("%s %s bar%d" % (si_cmd, app, i))
            rc = p.wait()
            self.assert_(rc == 0 or rc == 250,
                         "[%d] return code was %d, should have been 0" % (i, p.returncode))

        pid = get_pid(app)
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

        pid = get_pid(app)
        self.assert_(pid != None, "%s was not started" % app)
        pid = pid.splitlines()[0]

        kill_process(app, signum = 9)

        run_cmd_as_user('%s %s bar' % (si_cmd, app))

        time.sleep(2)

        pid2 = get_pid(app)
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

    def test_single_instance_window_raise_with_invoker(self):
        self.single_instance_window_raise('invoker --type=m --single-instance')

    def test_single_instance_and_non_single_instance_with_invoker(self):
        self.single_instance_and_non_single_instance('invoker --type=m --single-instance')

    def test_single_instance_stress_test_with_invoker(self):
        self.single_instance_stress_test('invoker --type=m --single-instance')

    def test_single_instance_abnormal_lock_release_with_invoker(self):
        self.single_instance_abnormal_lock_release('invoker --type=m --single-instance')


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
