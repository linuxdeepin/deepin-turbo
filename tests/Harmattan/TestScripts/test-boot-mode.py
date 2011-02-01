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
These test cases test the boot mode functionality.

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

class BootModeTests(unittest.TestCase):
    def setUp(self):
        debug("setUp")

        if get_pid('applauncherd') != None:
            stop_applauncherd()

        time.sleep(2)

        if get_pid('applauncherd') != None:
            kill_process('applauncherd')

        self.start_applauncherd_in_boot_mode()

        for b in ['booster-m', 'booster-q', 'booster-d']:
            get_pid(b)

    def tearDown(self):
        debug("tearDown")

        kill_process('fala_multi-instance')
        
        if get_pid('applauncherd'):
            kill_process('applauncherd')

        time.sleep(5)

    def start_applauncherd_in_boot_mode(self):
        remove_applauncherd_runtime_files()

        run_app_as_user('/usr/bin/applauncherd --boot-mode')

        time.sleep(5)

        # assert that applauncherd is running
        pid = get_pid('applauncherd')
        self.assert_(pid != None, "applauncherd wasn't started in boot mode")

    def get_booster_pids(self):
        pid_m = get_pid('booster-m')
        pid_q = get_pid('booster-q')
        
        self.assert_(pid_m != None, "booster-m not running")
        self.assert_(pid_q != None, "booster-q not running")
        
        return [pid_m.strip(), pid_q.strip()]

    def test_change_to_normal_mode(self):
        # start applauncherd in boot mode
        # check booster pids
        # send sigusr1 to applauncherd
        # check that booster pids have changed

        #self.start_applauncherd_in_boot_mode()

        # get booster pids in boot mode
        pids = self.get_booster_pids()

        # send SIGUSR1
        kill_process('applauncherd', signum=10)

        time.sleep(10)

        # get booster pids in normal mode
        pids2 = self.get_booster_pids()

        # terminate applauncherd
        kill_process('applauncherd', signum=15)

        self.assert_(pids[0] != pids2[0], "pid of booster-m didn't change")
        self.assert_(pids[1] != pids2[1], "pid of booster-m didn't change")

    def launch_apps(self, n = 6):
        # check that launching works and the apps are there
        for i in range(n):
            run_app_as_user('/usr/bin/invoker -n -r 2 --type=m fala_multi-instance %d' % i)

        # give the applications time to really start
        time.sleep(2 * n + 5)

        pids = get_pid('fala_multi-instance')
        pids = pids.split()

        # check that windows are there
        st, op = commands.getstatusoutput('xwininfo -root -tree')
        op = op.splitlines()

        wids = []
        for line in op:
            if line.find('fala_multi-instance') != -1 and line.find('1x1+0+0') == -1:
                print line
                wids.append(line.split()[0])
        
        # terminate apps
        kill_process('fala_multi-instance', signum=15)

        return [len(pids), len(wids)]

    def test_boot_mode_and_normal_mode(self):
        #self.start_applauncherd_in_boot_mode()

        #time.sleep(5)

        # launch apps in boot mode
        res_boot = self.launch_apps(6)

        # switch to normal mode and give boosters some time to start
        kill_process('applauncherd', signum=10)
        time.sleep(5)

        # launch apps in normal mode
        res_norm = self.launch_apps(6)

        # and finally, terminate applauncherd
        kill_process('applauncherd', signum=15)

        # assert that the boot mode results are correct
        self.assert_(res_boot[0] == 6 and res_boot[1] == 6,
                     "%d apps, %d windows. Expected %d apps, %d windows (boot mode)" % (res_boot[0],
                                                                                        res_boot[1],
                                                                                        6, 6))

        # assert that the normal mode results are correct
        self.assert_(res_norm[0] == 6 and res_norm[1] == 6,
                     "%d apps, %d windows. Expected %d apps, %d windows (normal mode)" % (res_norm[0],
                                                                                          res_norm[1],
                                                                                          6, 6))

if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    check_prerequisites()

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(BootModeTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    # kill applauncherd if it's left running and restart it
    kill_process('applauncherd')
    restart_applauncherd()

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
