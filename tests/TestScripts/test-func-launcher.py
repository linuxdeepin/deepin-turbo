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

import os
import subprocess
import commands
import time
import sys
import unittest
import re

from subprocess import Popen

LAUNCHER_BINARY='/usr/bin/applauncherd'
DEV_NULL = file("/dev/null","w")
LAUNCHABLE_APPS = ['/usr/bin/fala_ft_hello','/usr/bin/fala_ft_hello1', '/usr/bin/fala_ft_hello2']
PREFERED_APP = '/usr/bin/fala_ft_hello'

using_scratchbox = False

def debug(*msg):
    """
    Debug function
    """
    sys.stderr.write('[DEBUG %s] %s\n' % (time.ctime(), \
            ' '.join([str(s) for s in msg]),))

def error(*msg):
    """
    exit when error, give proper log 
    """
    sys.stderr.write('ERROR %s\n' % (' '.join([str(s) for s in msg]),))
    sys.exit(1)

def basename(filepath):
    """
    return base name of a file
    """
    return os.path.basename(filepath) 

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
        assert(len(basename(app)) <= 14, "For app: %s, base name !<= 14" % app)

class launcher_tests (unittest.TestCase):
    def setUp(self):
        #setup here
        print "Executing SetUp"

    def tearDown(self):
        #teardown here
        print "Executing TearDown"

    #Other functions
    def run_app_with_launcher(self, appname):
        p = subprocess.Popen(appname, 
                shell=False, 
                stdout=DEV_NULL, stderr=DEV_NULL)
        return p

    #get_pid = lambda appname: commands.getstatusoutput("pgrep %s" % appname)[-1]
    
    def get_pid(self, appname):
        temp = basename(appname)[:14]
        st, op = commands.getstatusoutput("pgrep %s" % temp)
        if st == 0:
            return op
        else:
            return None
    
    def kill_process(self, appname=None, apppid=None, signum=9):
        if apppid and appname: 
            return None
        else:
	    if apppid: 
		st, op = commands.getstatusoutput("kill -%s %s" % (str(signum), str(apppid)))
	    if appname: 
		temp = basename(appname)[:14]
		st, op = commands.getstatusoutput("pkill -%s %s" % (str(signum), temp))
	        os.wait()

    def process_state(self, processid):
        st, op = commands.getstatusoutput('cat /proc/%s/stat' %processid)
        if st == 0:
            return op
        else:
            debug(op)
            return None

    def get_creds(self, path):
        """
        Tries to launch an application and if successful, returns the
        credentials the application has as a list. 
        """

        # try launch the specified application
        handle = self.run_app_with_launcher(path)

        # sleep for a moment to allow applauncherd to start the process
        time.sleep(5)

        # with luck, the process should have correct name by now
        pid = self.get_pid(path)

        debug("%s has PID %s" % (basename(path), pid,))

        self.assert_(pid != None, "Couldn't launch %s" % basename(path))

        # get the status and output (needs creds-get from libcreds2-tools
        # package)
        st, op = commands.getstatusoutput("/usr/bin/creds-get -p %s" % pid)

        self.kill_process(path)

        # The first line from creds-get is rubbish (at the moment at least)
        return op.split("\n")[1:], pid

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

    def wait_for_app(self, app = None, timeout = 5, sleep = 0.5):
        """
        Waits for an application to start. Checks periodically if
        the app is running for a maximum wait set in timeout.
        
        Returns the pid of the application if it was running before
        the timeout finished, otherwise None is returned.
        """

        pid = None
        start = time.time()

        while pid == None and time.time() < start + timeout:
            pid = self.get_pid(app)
            
            if pid != None:
                break

            print "waiting %s secs for %s" % (sleep, app)

            time.sleep(sleep)

        return pid

    def test_003_zombie_state(self):
        """
        To test that no Zombie process exist after the application is killed
        """
        #launch application with launcher
        #check if the application is running
        #kill the application (pid = p.pid)
        #check if pgrep appname should be nothing
        #self.kill_process(LAUNCHER_BINARY)

        process_handle = self.run_app_with_launcher(PREFERED_APP)
        process_id = self.wait_for_app(PREFERED_APP, 5)
        print process_id
        self.kill_process(PREFERED_APP)
        time.sleep(4)

        process_handle = self.run_app_with_launcher(PREFERED_APP)
        process_id1 = self.wait_for_app(PREFERED_APP, 5)
        print process_id1
        self.kill_process(PREFERED_APP)
        time.sleep(4)

        process_id1 = self.get_pid(PREFERED_APP)
        print process_id1

        self.assert_(process_id != process_id1 , "New Process not launched")
        self.assert_(process_id1 == None , "Process still running")
    
    def test_004_launch_multiple_apps(self):
        """
        To test that more than one applications are launched by the launcher 
        """
        pidlist = []
        for app in LAUNCHABLE_APPS: 
            #launch application with launcher
            #check if the application is running
            #check if p.pid is same as pgrep appname
            #in a global dictionary, append the pid
            process_handle = self.run_app_with_launcher(app)
            time.sleep(5)
            process_id = self.get_pid(app)
            pidlist.append(process_id)
            self.assert_(not (process_id == None), "All Applications were not launched using launcher")
        for pid in pidlist:
            self.kill_process(apppid=pid)
       
    
    def test_005_one_instance(self):
        """
        To test that only one instance of a application exist 
        """
        #launch application
        #self.run_app_with_launcher(appname)
        #get pid of application
        #launch applicatoin again
        #check pgrep application
        #y = commands.getstatusoutput(pgrep appname)
        #len(y[-1].split(' ')) == 1
        process_handle = self.run_app_with_launcher(PREFERED_APP)
        process_id = self.get_pid(PREFERED_APP)
        debug("PID of first %s" % process_id)
        process_handle1 = self.run_app_with_launcher(PREFERED_APP)
        time.sleep(2)
        process_id = self.get_pid(PREFERED_APP)
        debug("PID of 2nd %s" % process_id)
        self.assert_( len(process_id.split(' ')) == 1, "Only one instance of app not running")
        self.kill_process(PREFERED_APP)


    def test_006_creds(self):
        """
        Test that the fala_ft_creds* applications have the correct
        credentials set (check aegis file included in the debian package)
        """
        op1, pid1 = self.get_creds('/usr/bin/fala_ft_creds1')
        op2, pid2 = self.get_creds('/usr/bin/fala_ft_creds2')

        # filter out some unnecessary tokens
        def filterfunc(x):
            pattern = "^(SRC|AID)::"
            return re.match(pattern, x) == None

        op1 = filter(filterfunc, op1)
        op2 = filter(filterfunc, op2)

        debug("fala_ft_creds1 has %s" % ', '.join(op1))
        debug("fala_ft_creds2 has %s" % ', '.join(op2))

        # required common caps
        caps = ['UID::user', 'GID::users',
                'applauncherd-testapps::applauncherd-testapps']

        # required caps for fala_ft_creds1
        cap1 = ['tcb', 'drm', 'Telephony', 'CAP::setuid', 'CAP::setgid',
                'CAP::setfcap'] + caps

        # required caps for fala_ft_creds2
        cap2 = ['Cellular'] + caps

        # check that all required creds are there
        for cap in cap1:
            self.assert_(cap in op1, "%s not set for fala_ft_creds1" % cap)

        for cap in cap2:
            self.assert_(cap in op2, "%s not set for fala_ft_creds2" % cap)

        # check that no other creds are set
        op1.sort()
        cap1.sort()

        self.assert_(op1 == cap1, "fala_ft_creds1 has non-requested creds!")

        op2.sort()
        cap2.sort()

        self.assert_(op2 == cap2, "fala_ft_creds2 has non-requested creds!")

    def test_007_no_aegis_Bug170905(self):
        """
        Check that an application that doesn't have aegis file doesn't
        get any funny credentials.
        """

        creds, pid = self.get_creds('/usr/bin/fala_ft_hello')
        debug("fala_ft_hello has %s" % ', '.join(creds))

        # Credentials should be dropped, but uid/gid retained
        req_creds = ['UID::user', 'GID::users']

        creds.sort()
        req_creds.sort()

        self.assert_(creds == req_creds, "fala_ft_hello has differnt creds set!")

    def test_008_invoker_creds(self):
        """
        Test that the launcher registered customized credentials 
        and invoker has proper credentials to access launcher
        """

        INVOKER_BINARY='/usr/bin/invoker'
        FAKE_INVOKER_BINARY='/usr/bin/faulty_inv'
        
        #test application used for testing invoker
        Testapp = '/usr/bin/fala_ft_hello.launch'

        #launching the testapp with actual invoker
        st = os.system('%s --type=m %s'%(INVOKER_BINARY, Testapp))
        pid = self.get_pid(Testapp.replace('.launch', ''))
        self.assert_((st == 0), "Application was not launched using launcher")
        self.assert_(not (pid == None), "Application was not launched using launcher: actual pid%s" %pid)
        print pid
        #self.kill_process(Testapp.replace('.launch', ''))       
        self.kill_process(apppid=pid)  
        pid = self.get_pid(Testapp.replace('.launch', '')) 
        self.assert_((pid == None), "Application still running")        
        
        #launching the testapp with fake invoker
        st = os.system('%s --type=m %s'%(FAKE_INVOKER_BINARY, Testapp)) 
        pid = self.get_pid(Testapp.replace('.launch', ''))
        self.assert_(not (st == 0), "Application was launched using fake launcher")
        self.assert_((pid == None), "Application was launched using fake launcher")
        
    def test_009_launch_multiple_apps_cont(self):
        """
        To test that more than one applications are launched by the launcher 
        """
        for app in LAUNCHABLE_APPS: 
            #launch application with launcher
            #check if the application is running
            #check if p.pid is same as pgrep appname
            #in a global dictionary, append the pid
            process_handle = self.run_app_with_launcher(app)
        time.sleep(8)
        process_id = self.get_pid('fala_ft_hello')
        pid_list = process_id.split()
        self.assert_(len(pid_list) == len(LAUNCHABLE_APPS), "All Applications were not launched using launcher")
        for pid in pid_list:
            self.kill_process(apppid=pid)

    def test_010(self):
        """
        NB#179266

        When calling invoker with --wait-term and killing invoker,
        the launched application should die too.
        """

        invoker = '/usr/bin/invoker'
        app_path = '/usr/bin/fala_ft_hello.launch'

        # Launch the app with invoker
        p = subprocess.Popen(('%s --type=m --wait-term %s' % (invoker, app_path)).split(),
                             shell = False,
                             stdout = DEV_NULL, stderr = DEV_NULL)

        # Retrieve their pids
        invoker_pid = self.wait_for_app('invoker')
        app_pid = self.wait_for_app('fala_ft_hello')

        # Make sure that both apps started
        self.assert_(invoker_pid != None, "invoker not executed?")
        self.assert_(app_pid != None, "%s not launched by invoker?" % app_path)

        # Send SIGTERM to invoker, the launched app should die
        self.kill_process(None, invoker_pid, 15)
        
        time.sleep(2)

        # This should be None
        app_pid2 = self.get_pid('fala_ft_hello')

        if (app_pid2 != None):
            self.kill_process(None, app_pid2)
            self.assert_(False, "%s was not killed" % app_path)


    def test_011(self):
        """
        Test that the --daemon parameter works for applauncherd
        """

        # function to remove some temporaries
        def rem():
            files = ['/tmp/applauncherd.lock', '/tmp/qtlnchr', '/tmp/mlnchr']

            for f in files:
                print "removing %s" % f

                try:
                    os.remove(f)
                except:
                    pass

        # stop applauncherd if it's running
        if not using_scratchbox:
            commands.getstatusoutput("initctl stop xsession/applauncherd")

        # and for the fun of it let's do it again
        commands.getstatusoutput("pkill applauncherd")

        rem()

        # start applauncherd daemonized
        p = subprocess.Popen(["/usr/bin/applauncherd.bin", "--daemon"],
                             shell=False, 
                             stdout=DEV_NULL, stderr=DEV_NULL)

        time.sleep(3)

        st, op = commands.getstatusoutput('pgrep -lf "applauncherd.bin --daemon"')
        print op

        # filter some cruft out from the output and see how many instances are running
        op = filter(lambda x: x.find("sh ") == -1, op.split("\n"))
        count = len(op)

        print "count = %d" % count

        self.assert_(count == 1, "applauncherd was not daemonized (or too many instances running ..)")

        # try to launch an app
        self.run_app_with_launcher('/usr/bin/fala_ft_hello')
        time.sleep(2)

        pid = self.wait_for_app('fala_ft_hello')

        if pid != None:
            self.kill_process(apppid = pid)
        else:
            self.assert_(False, "fala_ft_hello was not launched!")

        # only the daemonized applauncherd should be running now
        commands.getstatusoutput('pkill applauncherd')

        rem()

        # start applauncherd again
        if using_scratchbox:
            subprocess.Popen("/usr/bin/applauncherd",
                             shell=False, 
                             stdout=DEV_NULL, stderr=DEV_NULL)
        else:
            commands.getstatusoutput("initctl start xsession/applauncherd")


    def test_012(self):
        """
        Test the --delay parameter of the invoker.
        """

        # launch an app with invoker --delay n
        print "launching fala_ft_hello ..."
        p = Popen(['/usr/bin/invoker', '--delay', '10', '--type=m',
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
