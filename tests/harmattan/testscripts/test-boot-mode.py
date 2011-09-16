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
        if get_pid('applifed') != None:
            os.system("initctl stop xsession/applifed")

        self.start_applauncherd_in_boot_mode()

        for b in ['booster-m', 'booster-q', 'booster-d']:
            get_pid(b)

    def tearDown(self):
        debug("tearDown")

        if get_pid('fala_multi-instance') != None:
            kill_process('fala_multi-instance')
        
        if get_pid('applauncherd') != None:
            kill_process('applauncherd')
        if get_pid('applifed') == None:
            os.system("initctl start xsession/applifed")
            wait_for_app('camera-ui')
        start_applauncherd()

    def start_applauncherd_in_boot_mode(self):
        remove_applauncherd_runtime_files()

        run_cmd_as_user('/usr/bin/applauncherd --boot-mode')

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
        wait_for_app('booster-q')
        wait_for_app('booster-m')


        # get booster pids in normal mode
        pids2 = self.get_booster_pids()

        # terminate applauncherd
        kill_process('applauncherd', signum=15)

        self.assert_(pids[0] != pids2[0], "pid of booster-m didn't change")
        self.assert_(pids[1] != pids2[1], "pid of booster-m didn't change")

    def launch_apps(self, n = 6):
        # check that launching works and the apps are there
        for i in range(n):
            run_cmd_as_user('/usr/bin/invoker -n -r 2 --type=m fala_multi-instance %d' % i)
            time.sleep(4)

        # give the applications time to really start
        time.sleep(2 * n + 5)

        pids = get_pid('fala_multi-instance')
        pids = pids.split()

        # check that windows are there
        st, op = commands.getstatusoutput("xwininfo -root -tree| grep 854x480+0+0 |awk '/fala_multi-instance/ {print $1}'")
        wids = op.splitlines()
        
        debug("The windows is %s " %wids)
        
        # terminate apps
        kill_process('fala_multi-instance', signum=15)

        return [len(pids), len(wids)]

    def test_boot_mode_and_normal_mode(self):
        #self.start_applauncherd_in_boot_mode()

        #time.sleep(5)

        # launch apps in boot mode
        res_boot = self.launch_apps(6)
        debug("Res at boot : %s" %res_boot)

        # switch to normal mode and give boosters some time to start
        kill_process('applauncherd', signum=10)
        wait_for_app('booster-m')
        wait_for_app('booster-q')

        # launch apps in normal mode
        res_norm = self.launch_apps(6)
        debug("Res at normal : %s" %res_norm)

        # and finally, terminate applauncherd
        kill_process('applauncherd', signum=15)
        wait_for_app('booster-m')
        wait_for_app('booster-q')

        # assert that the boot mode results are correct
        self.assert_(res_boot[0] == 6 and res_boot[1] == 6,
                     "%d apps, %d windows. Expected %d apps, %d windows (boot mode)" % (res_boot[0],
                                                                                        res_boot[1],
                                                                                        6, 6))

        # assert that the normal mode results are correct
        self.assert_(res_norm[0] == 6 and res_norm[1] == 12,
                     "%d apps, %d windows. Expected %d apps, %d windows (normal mode)" % (res_norm[0],
                                                                                          res_norm[1],
                                                                                          6, 12))

    def test_SIGUSR2(self):
        """
        send SIGUSR2 for applauncherd when in boot mode. This should turn it into boot mode.
        """
        pid = wait_for_single_applauncherd()
        st, op = commands.getstatusoutput("kill -SIGUSR2 %s" %pid)
        time.sleep(3)
        st1, op1 = commands.getstatusoutput("grep '%s]: Daemon: Already in boot mode' /var/log/syslog " %pid)
        debug("The log msg is %s" %op1)
        self.assert_(st1 == 0, "Seems that SIGUSR2 was not send")

    def test_SIGUSR1(self):
        """
        send SIGUSR1 for applauncherd when it is in boot mode. 
        This should turn it into normal mode. Try sending signal twice.
        Another time you would get message in syslog that it is already in normal mode
        """
        #Send SIGUSR1 for the first time
        #Get pids for daemon and boosters
        daemon_pid = wait_for_single_applauncherd()
        pid_q = wait_for_app("booster-q")
        pid_d = wait_for_app("booster-d")
        pid_m = wait_for_app("booster-m")
        pid_e = wait_for_app("booster-e")

        #Send SIGUSR1 to daemon
        st, op = commands.getstatusoutput("kill -SIGUSR1 %s" %daemon_pid)
        time.sleep(3)
        st1, op1 = commands.getstatusoutput("grep '%s]: Daemon: Exited boot mode.' /var/log/syslog " %daemon_pid)
        debug("The log msg is %s" %op1)
        self.assert_(st1 == 0, "Seems that SIGUSR1 was not send")

        #Get pids for boosters
        pid_q_1 = wait_for_app("booster-q")
        pid_d_1 = wait_for_app("booster-d")
        pid_m_1 = wait_for_app("booster-m")
        pid_e_1 = wait_for_app("booster-e")

        self.assert_(pid_q != pid_q_1, "Applauncherd not changed to normal mode")
        self.assert_(pid_d != pid_d_1, "Applauncherd not changed to normal mode")
        self.assert_(pid_m != pid_m_1, "Applauncherd not changed to normal mode")
        self.assert_(pid_e != pid_e_1, "Applauncherd not changed to normal mode")

        #Send SIGUSR1 for the second time
        st, op = commands.getstatusoutput("kill -SIGUSR1 %s" %daemon_pid)
        time.sleep(3)
        st1, op1 = commands.getstatusoutput("grep '%s]: Daemon: Already in normal mode.' /var/log/syslog " %daemon_pid)
        debug("The log msg is %s" %op1)
        self.assert_(st1 == 0, "Seems that SIGUSR1 was not send")


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

