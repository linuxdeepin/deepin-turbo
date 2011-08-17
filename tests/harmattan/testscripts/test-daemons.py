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
These checks the daemons running using invoker just after boot
"""

import commands
import unittest
from utils import *


class DaemonTests(unittest.TestCase):
    def setUp(self):
        debug("setUp")

    def tearDown(self):
        debug("tearDown")

    def test_launcher_exist(self):
        """
        To test if the launcher exists and is executable or not
        """
        self.assert_(os.path.isfile(LAUNCHER_BINARY), "Launcher file does not exist")
        self.assert_(os.access(LAUNCHER_BINARY, os.X_OK), "Launcher exists, but is not executable")

    def test_daemon_list(self):
        """
        To List the daemons running using invoker after boot
        """
        st, op = commands.getstatusoutput("ps ax | grep invoker")
        count = len(op.split("\n"))
        for i in xrange(count-2):
            if op.split("\n")[i].find("prestart") == -1:
                debug("%s" %op.split("\n")[i])

    def test_daemon(self):
        """
        Test that the --daemon parameter works for applauncherd
        """

        stop_applauncherd()

        remove_applauncherd_runtime_files()

        p = run_cmd_as_user('/usr/bin/applauncherd.bin --daemon')

        time.sleep(5)

        st, op = commands.getstatusoutput('pgrep -lf "applauncherd.bin --daemon"')
        p_id = op.split(" ")[0]
        debug("The pid of applauncherd --daemon is %s" %op)

        # filter some cruft out from the output and see how many
        # instances are running
        op = filter(lambda x: x.find("sh ") == -1, op.split("\n"))
        count = len(op)

        debug("count = %d" % count)

        self.assert_(count == 1, "applauncherd was not daemonized (or too many instances running ..)")

        # try to launch an app
        run_cmd_as_user('/usr/bin/fala_ft_hello')
        time.sleep(2)

        pid = wait_for_app('fala_ft_hello')

        if pid != None:
            kill_process(apppid = pid)
        else:
            self.assert_(False, "fala_ft_hello was not launched!")

        # only the daemonized applauncherd should be running now
        kill_process(apppid = p_id)
        #commands.getstatusoutput('pkill applauncherd')

        remove_applauncherd_runtime_files()

        start_applauncherd()

    def test_daemon_second_instance(self):
        """
        Test that second instance of applauncherd cannot be started
        """
        daemon_pid = get_pid("applauncherd")
        if daemon_pid == None:
            start_applauncherd
            daemon_pid = get_pid("applauncherd")
        debug("start applauncherd again")
        st, op = commands.getstatusoutput("initctl start xsession/applauncherd")
        time.sleep(3)
        daemon_pid_new = get_pid("applauncherd")
        self.assert_(daemon_pid == daemon_pid_new, "New instance of applauncherd started")
        self.assert_(st != 0, "Second instance of applauncherd started")
        
    def test_writable_executable_mem(self):
        """
        Test that applauncherd does not have the writable and executable memory
        """

        pid = get_pid('applauncherd')
        st, op = commands.getstatusoutput("grep wx /proc/%s/smaps" %pid)
        debug("The value of status is %d" %st)
        debug("The value of output is %s" %op)
        self.assert_(st != 0, "applauncherd has writable and executable memory")

    def test_applauncherd_fd_close(self):
        self._test_applauncherd_fd()

    def test_applauncherd_fd_kill(self):
        self._test_applauncherd_fd(False)

    def _test_applauncherd_fd(self, close = True):
        """
        To test that file descriptors are closed before calling application main
        """
        #get fd of booster before launching application
        debug("get fd of applauncherd before launching application")
        pid = commands.getoutput("pgrep applauncherd")
        init_count = commands.getoutput('ls -l /proc/%s/fd/ | wc -l' % str(pid))
        debug("\nThe count of initial file descriptors is : %s\n" %init_count)
        time.sleep(3)
        
        #launch application using booster
        debug("launch fala_wl using booster")
        status = os.system('invoker --type=m /usr/bin/fala_wl &')

        #get fd of booster after launching the application
        debug("get fd of booster after launching the application")
        if status == 0:
            launch_count = commands.getoutput('ls -l /proc/%s/fd/ | wc -l' % str(pid))
            debug("\nThe count of file descriptors after launch : %s\n" %launch_count)
        time.sleep(3)
        
        #Close application
        if close:
            st, wid = commands.getstatusoutput(\
                    "xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'")
            os.system("/usr/bin/xsendevent close %s" %wid)
        else:
            pid_app = commands.getoutput('pgrep fala_wl')    
            kill_process(apppid=pid_app) 
        time.sleep(3)

        #get fd of booster after closing the application
        debug("get fd of booster after closing the application")
        close_count = commands.getoutput('ls -l /proc/%s/fd/ | wc -l' % str(pid))
        debug("\nThe count file descriptors after close: %s\n" %close_count)
        self.assert_(int(launch_count) == int(init_count) +1, "The file descriptors was not changed")
        self.assert_(close_count == init_count, "The file descriptors was changed")

    def test_nonlaunchable_apps(self):
        """
        Test that Booster gives warning while trying to launch non launchable applications
        Here fala_wid is a shell script and libebooster.so is a library
        """
        st, op = commands.getstatusoutput("/usr/bin/invoker --type=m /usr/bin/fala_wid")
        debug("The Warning is %s" %(op.split("\n")[0]))
        pos = op.split("\n")[0].find("Booster: Loading invoked application failed:")
        self.assert_(pos != -1, "The booster did not give warning")
        
        st, op = commands.getstatusoutput("/usr/bin/invoker --type=m /usr/lib/applauncherd/libebooster.so")
        debug("The Warning is %s" %(op.split("\n")[0]))
        pos = op.split("\n")[0].find("Booster: Loading symbol 'main' failed:")
        self.assert_(pos != -1, "The booster did not give warning")

    def test_daemon_debug(self):
        """
        Test the --debug option for the daemon
        """
        stop_applauncherd()
        remove_applauncherd_runtime_files()
        os.system('/usr/bin/applauncherd.bin --debug&')
        time.sleep(10)

        st, op = commands.getstatusoutput('pgrep -lf "applauncherd.bin --debug"')
        p_id = op.split(" ")[0]
        debug("The pid of applauncherd --debug is %s" %op)
        kill_process(apppid=p_id)
        start_applauncherd()
        self.assert_(st == 0, "Applauncherd was not started in debug mode")

    def test_daemon_help(self):
        """
        Test the --help parameter for the daemon
        """
        stop_applauncherd()
        remove_applauncherd_runtime_files()
        st, op = commands.getstatusoutput('/usr/bin/applauncherd.bin --help')
        start_applauncherd()
        self.assert_(st == 0, "Applauncherd did not print help")
        self.assert_(op.split("\n")[1] == 'Usage: applauncherd [options]', "Applauncherd did not print help")

    def test_daemon_no_display(self):
        """
        Test that daemon cannot be started if the DISPLAY env variable is not set
        """
        stop_applauncherd()
        remove_applauncherd_runtime_files()
        st, op = commands.getstatusoutput('(unset DISPLAY;/usr/bin/applauncherd.bin)')
        start_applauncherd()
        self.assert_(st != 0, "Applauncherd was started even when DISPLAY was not set")
        self.assert_(op == 'FATAL!!: DISPLAY environment variable not set.',\
                "Applauncherd was started even when DISPLAY was not set")






if __name__ == '__main__':

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(DaemonTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
