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
4. Given application supports launcher with -launcher commandline argument.
5. waitforwindow application should be installed.

Usage:    test-perf-mbooster <launcherable application>

Example:  test-perf-mbooster /usr/bin/testapp

Authors: Nimika Keshri nimika.1.keshri@nokia.com 
"""
import os
import subprocess
import commands
import time
import sys
import unittest

TESTAPP = '/usr/bin/fala_testapp'
LOG_FILE = '/tmp/testapp.log'
DEV_NULL = file("/dev/null","w")

_start_time = 0
_end_time = 0

def debug(*msg):
    sys.stderr.write('[DEBUG %s] %s\n' % (time.time(), ' '.join([str(s) for s in msg]),))

def error(*msg):
    sys.stderr.write('ERROR %s\n' % (' '.join([str(s) for s in msg]),))
    sys.exit(1)

def basename(filepath):
    """
    return base name of a file
    """
    return os.path.basename(filepath) 
def is_executable_file(filename):
    return os.path.isfile(filename) and os.access(filename, os.X_OK)
    
def check_prerequisites():
    if os.getenv('DISPLAY') == None:
        error("DISPLAY is not set. Check the requirements.")
        
    if os.getenv('DBUS_SESSION_BUS_ADDRESS') == None:
        error("DBUS_SESSION_BUS_ADDRESS is not set.\n" +
              "You probably want to source /tmp/session_bus_address.user")

    if not is_executable_file(TESTAPP):
        error("'%s' is not an executable file\n" % (TESTAPP,) +
              "(should be an application that supports launcher)")


class launcher_perf_tests (unittest.TestCase):
    
    def setUp(self):
        print "Setup Executed"

    def tearDown(self):
        print "Teardown Executed"

    #Other functions
    def start_timer(self):
        global _start_time 
        _start_time = time.time()

    def run_without_launcher(self, appname):                                                       
        """starts the testapp without the launcher"""               
        os.system ('mcetool --set-tklock-mode=unlocked')
        if os.path.exists(LOG_FILE) and os.path.isfile(LOG_FILE):               
            os.system('rm %s' %LOG_FILE)                                                        
        self.start_timer()
        p = subprocess.Popen(appname,
                             shell=False,
                             stdout=DEV_NULL, stderr=DEV_NULL)
        debug("app", TESTAPP, "started without launcher")                                   
        time.sleep(5)
        self.read_log()
        app_time = self.app_start_time()
        self.kill_process(appname)
        return app_time

    def run_without_launcher_without_duihome(self, appname):                                                       
        """starts the testapp without launcher and without duihome"""               
        os.system ('mcetool --set-tklock-mode=unlocked')
        if os.path.exists(LOG_FILE) and os.path.isfile(LOG_FILE):               
            os.system('rm %s' %LOG_FILE)                                                        
        os.system('pkill -STOP duihome')
        os.system('pkill -STOP meegotouchhome')
        self.start_timer()
        p = subprocess.Popen(TESTAPP,
                             shell=False,
                             stdout=DEV_NULL, stderr=DEV_NULL)
        debug("app", TESTAPP, "started without launcher")                                   
        time.sleep(5)
        os.system('pkill -CONT duihome')
        os.system('pkill -CONT meegotouchhome')
        self.read_log()
        app_time = self.app_start_time()
        self.kill_process(appname)
        return app_time

    def run_with_launcher(self, appname):                                                       
        """starts the testapp with launcher and with duihome"""               
        os.system ('mcetool --set-tklock-mode=unlocked')
        if os.path.exists(LOG_FILE) and os.path.isfile(LOG_FILE):               
            os.system('rm %s' %LOG_FILE)                                                        

        self.start_timer()
        os.system('invoker --type=m %s' %TESTAPP)
        debug("app", TESTAPP, "started with launcher")                                   
        time.sleep(5)
        self.read_log()
        app_time = self.app_start_time()
        self.kill_process(appname)
        return app_time

    def run_with_wrt_launcher(self, appname):                                                       
        """starts the testapp with wrt-launcher and with duihome"""               
        os.system ('mcetool --set-tklock-mode=unlocked')
        if os.path.exists(LOG_FILE) and os.path.isfile(LOG_FILE):               
            os.system('rm %s' %LOG_FILE)                                                        

        self.start_timer()
        os.system('invoker --type=wrt %s' %TESTAPP)
        debug("app", TESTAPP, "started with wrt-launcher")                                   
        time.sleep(5)
        self.read_log()
        app_time = self.app_start_time()
        self.kill_process(appname)
        return app_time

    def run_with_launcher_without_duihome(self, appname):                                                       
        """starts the testapp with launcher but without duihome"""               
        os.system ('mcetool --set-tklock-mode=unlocked')
        if os.path.exists(LOG_FILE) and os.path.isfile(LOG_FILE):               
            os.system('rm %s' %LOG_FILE)                                                        
        os.system('pkill -STOP duihome')
        os.system('pkill -STOP meegotouchhome')
        self.start_timer()
        os.system('invoker --type=m %s' %TESTAPP)
        debug("app", TESTAPP, "started with launcher")                                   
        time.sleep(5)
        os.system('pkill -CONT duihome')
        os.system('pkill -CONT meegotouchhome')
        self.read_log()
        app_time = self.app_start_time()
        self.kill_process(appname)
        return app_time

    def read_log(self):
        """Reads the log file to get the startup time"""
        global _end_time
        fh = open(LOG_FILE, "r")
        lines = fh.readlines()
        lastline = lines[len(lines)-2]
        _end_time = lastline.split()[0]
        return _end_time

    def app_start_time(self):
        """Calculates the startup time for the testapp"""
        global _app_start_time
        _app_start_time = float(_end_time) - float(_start_time)
        return _app_start_time

    def kill_process(self, appname):
        """Kills the testapp"""
        commands.getoutput("pkill -9 %s" % (basename(appname)[:15],))


    def perftest_with_launcher(self, appname):
        debug("run app with launcher without duihome")
        with_l_no_d = self.run_with_launcher_without_duihome(appname)
        time.sleep(5)

        debug("run app with launcher with duihome")
        with_l_with_d = self.run_with_launcher(appname)
        time.sleep(5)

        return with_l_with_d, with_l_no_d

    def perftest_with_wrt_launcher(self, appname):
        debug("run app with wrt-launcher with duihome")
        wrt_l_with_dld = self.run_with_wrt_launcher(appname)
        time.sleep(5)

        return wrt_l_with_dld

    def perftest_without_launcher(self, appname):
        """Runs all the 4 scenarios with and without launcher"""
        debug("run app without launcher with duihome")
        no_l_with_d = self.run_without_launcher(appname)
        time.sleep(5)

        debug("run app without launcher without duihome")
        no_l_no_d = self.run_without_launcher_without_duihome(appname)
        time.sleep(5)

        return no_l_with_d, no_l_no_d


    def print_test_report(self, with_without_times, fileobj):
        """
        with_without_times is a list of pairs:
           (with_launcher_startup_time,
            without_launcher_startup_time)
        """
        def writeline(*msg):
            fileobj.write("%s\n" % ' '.join([str(s) for s in msg]))
        def fmtfloat(f):
            return "%.2f" % (f,)
        def filterstats(data, field):
            return tuple([d[field] for d in data])

        if with_without_times == []: return

        writeline("")
        rowformat = "%12s %12s %12s %12s %12s"
        writeline('Startup times [s]:')

        writeline(rowformat % ('launcher-Yes', 'launcher-Yes', 'launcher-No', 'launcher-No', 'wrtlauncher-Yes'))
        writeline(rowformat % ('duihome-Yes', 'duihome-No', 'duihome-Yes', 'duihome-No', 'duihome-Yes'))

        t1,t2,t3,t4,t5 = [], [], [], [], []
        for no_l_with_d, no_l_no_d, with_l_with_d, with_l_no_d, wrt_l_with_d in with_without_times:
            t1.append(no_l_with_d)
            t2.append(no_l_no_d)
            t3.append(with_l_with_d)
            t4.append(with_l_no_d)
            t5.append(wrt_l_with_d)
            writeline(rowformat % (fmtfloat(no_l_with_d), fmtfloat(no_l_no_d),
                                   fmtfloat(with_l_with_d), fmtfloat(with_l_no_d), fmtfloat(wrt_l_with_d)))

        writeline('Average times:')
        writeline(rowformat % (fmtfloat(sum(t1)/len(t1)), fmtfloat(sum(t2)/len(t2)),
                               fmtfloat(sum(t3)/len(t3)), fmtfloat(sum(t4)/len(t4)), fmtfloat(sum(t5)/len(t5))))
        return fmtfloat(sum(t1)/len(t1))

    def test_001(self):
        """Performance test to measure the startup time for application
        launched using launcher and comparing the results with application launched
        without launcher"""

        times = []

        times1, times2, times3 = [], [], []

        for i in xrange(3):
            times1.append(self.perftest_with_launcher(TESTAPP))

        for i in xrange(3):
            times2.append(self.perftest_without_launcher(TESTAPP))

        for i in xrange(3):
            times3.append(self.perftest_with_wrt_launcher(TESTAPP))

        times = [[t1[0], t1[1], times2[i][0], times2[i][1], times3[i]] for i, t1 in enumerate(times1)]
        avg_with_launcher = self.print_test_report(times, sys.stdout)
        self.assert_(float(avg_with_launcher) < float(0.75), "application launched with launcher takes more than 0.75 sec")


# main
if __name__ == '__main__':
    check_prerequisites()
    tests = sys.argv[1:]
    mysuite = unittest.TestSuite(map(launcher_perf_tests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)
    if not result.wasSuccessful():
        sys.exit(1)
    sys.exit(0)
