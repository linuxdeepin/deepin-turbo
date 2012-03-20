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
This program tests the functionalities of invoker
Authors:   nimika.1.keshri@nokia.com
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

class InvokerTests(CustomTestCase):
    def setUp(self):
        debug("setUp")
        if daemons_running():
            stop_daemons()
            self.START_DAEMONS_AT_TEARDOWN = True
        else:
            self.START_DAEMONS_AT_TEARDOWN = False
        if get_pid('applauncherd') == None:
            start_applauncherd()

    def tearDown(self):
        debug("tearDown")
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
    def test_wait_term_m(self, sighup = True):
        self._test_wait_term('/usr/bin/fala_ft_hello', 'fala_ft_hello', 'm')
        if(sighup):
            self.sighup_applauncherd()
            self.test_wait_term_m(False)

    def test_wait_term_qml(self, sighup = True):
        self._test_wait_term('/usr/bin/fala_qml_helloworld', 'fala_qml_helloworld', 'd')
        if(sighup):
            self.sighup_applauncherd()
            self.test_wait_term_qml(False)

    def test_wait_term_e(self, sighup = True):
        self._test_wait_term('/usr/bin/fala_ft_hello', 'fala_ft_hello', 'e')
        if(sighup):
            self.sighup_applauncherd()
            self.test_wait_term_e(False)

    def _test_wait_term(self, app_path, app_name, btype, directlyKillInvoker=True):
        """
        When calling invoker with --wait-term and killing invoker,
        the launched application should die too.
        """

        kill_process(app_name)
        # Launch the app with invoker using --wait-term
        p = run_app_as_user_with_invoker(app_path,booster = btype, arg = '--wait-term' )

        # Retrieve their pids
        app_pid = wait_for_app(app_name)

        # Make sure that both apps started
        self.assert_(app_pid != None, "%s not launched by invoker?" % app_path)

        # Send SIGTERM to invoker, the launched app should die
        if directlyKillInvoker :
            debug(" Send SIGTERM to invoker, the launched app should die")
            kill_process('invoker', None, 15)
            self.waitForAsertEqual(lambda: p.poll(), 143, "invoker was not killed.", timeout = 3)
        else :
            debug(" Send SIGTERM to shell which runs invoker, the launched app should die")
            kill_process(None, p.pid, 15)
            self.waitForAsertEqual(lambda: p.poll(), -15, "invoker was not killed.", timeout = 3)

        debug("Invoker return code: %s" %(p.poll()))

        self.waitForAsertEqual(lambda: get_pid(app_name), None,
                               "Application '%s' was not killed!" % app_path,
                               timeout = 8)

    def test_launch_wo_applauncherd(self):
        """
        To Test that invoker can launch applications when the
        applauncherd is not running. Try launching applications with
        all boosters
        """

        stop_applauncherd()
        if get_pid(PREFERED_APP) != None:
            kill_process(PREFERED_APP)

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'm')
        pid1 = wait_for_app(PREFERED_APP)
        self.assert_(pid1 != None, "Application was executed")
        kill_process(PREFERED_APP)

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'e')
        pid1 = wait_for_app(PREFERED_APP)
        self.assert_(pid1 != None, "Application was executed")
        kill_process(PREFERED_APP)

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'q')
        pid1 = wait_for_app(PREFERED_APP)
        self.assert_(pid1 != None, "Application was executed")
        kill_process(PREFERED_APP)

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'd')
        pid1 = wait_for_app(PREFERED_APP)
        self.assert_(pid1 != None, "Application was executed")
        kill_process(PREFERED_APP)

        start_applauncherd()

    def test_signal_forwarding(self, sighup = True):
        """
        To test that invoker is killed by the same signal as the application
        """
        #Test for m-booster
        debug("Test for m-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_m.py")
        debug("The Invoker killed by : <%s>" %op.split ('\n')[-1])

        self.assert_(op.split('\n')[-1].find('Segmentation fault') != -1, "The invoker(m-booster) was not killed by the same signal")

        #Test for d-booster
        debug("Test for d-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_d.py")
        debug("The Invoker killed by : %s" % op.split('\n')[-1])

        self.assert_(op.split('\n')[-1].startswith('Terminated') == True, "The invoker(d-booster) was not killed by the same signal")

        #Test for e-booster
        debug("Test for e-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_e.py")
        debug("The Invoker killed by : %s" % op.split('\n')[-1])

        self.assert_(op.split('\n')[-1].startswith('Terminated') == True, "The invoker(e-booster) was not killed by the same signal")

        #This case is launching the application in user mode
        #Test for q-booster
        debug("Test for q-booster")
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_qt.py")
        debug("The Invoker killed by : %s" %op.split('\n')[-1])

        self.assert_(op.split('\n')[-1].startswith('Aborted') == True, "The invoker(q-booster) was not killed by the same signal")

        if(sighup):
            self.sighup_applauncherd()
            self.test_signal_forwarding(False)


    def test_invoker_delay(self, sighup = True):
        """
        Test the --delay parameter of the invoker.
        """

        invokerDelay = 10
        # launch an app with invoker --delay n
        debug("launching fala_ft_hello ...")
        p = Popen(['/usr/bin/invoker', '--delay', str(invokerDelay), '--type=m', '--test-mode', '--no-wait',
                   '/usr/bin/fala_ft_hello'],
                  shell=False,
                  stdout=DEV_NULL, stderr=DEV_NULL, preexec_fn=permit_sigpipe)

        pStartTime = time.time()
        appPid = wait_for_app('fala_ft_hello', timeout=7)
        try:
            falaStartTime = time.time()

            # wait a little
            debug("Waiting for invoker to terminate after %.1fs..." %(invokerDelay))
            # time out loop, if invoker runs longer then invokerDelay + 1 this means that something is wrong
            while time.time()<pStartTime + invokerDelay + 1 and p.poll()==None :
                time.sleep(0.5)

            endTime = time.time()
            # check that invoker has terminated before timeout
            if p.poll()==None :
                p.terminate()
                self.assert_(False, "invoker didn't terminate on time. "
                             "It was running for: %.1fs (%.1fs was expected)."
                             %(endTime - pStartTime, invokerDelay))
            debug("Invoker has terminated after %.1fs." %(endTime - pStartTime))
            self.assert_(endTime - pStartTime > invokerDelay - 1,
                         "invoker has terminated to soon: %.1fs (%.1fs was expected)" %(endTime - pStartTime, invokerDelay))
            self.assertEqual(get_pid('fala_ft_hello'), appPid, "Test application shouldn't be killed.")
        finally:
            kill_process('fala_ft_hello')
        debug("Ok.")

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_delay(False)

    def test_invoker_exit_status(self, sighup = True):
        """
        To test that invoker returns the same exit status as the application
        """
        #Run application without invoker and get the exit status
        debug("Run application without invoker and get the exit status")
        st, op = commands.getstatusoutput('/usr/bin/fala_status')
        app_st_wo_inv = os.WEXITSTATUS(st)
        debug("The exit status of app without invoker is : %d" %app_st_wo_inv)

        #Run application with invoker and get the exit status - booster-m case
        debug("Run application with invoker and get the exit status")
        st, op = commands.getstatusoutput('invoker --test-mode  --type=m --wait-term /usr/bin/fala_status')
        app_st_w_inv = os.WEXITSTATUS(st)
        debug("The exit status of app with invoker (booster-m) is : %d" %app_st_w_inv)

        #Run application with invoker and get the exit status - booster-e case
        debug("Run application with invoker and get the exit status")
        st, op = commands.getstatusoutput('invoker --type=e --wait-term /usr/bin/fala_status')
        app_st_we_inv = os.WEXITSTATUS(st)
        debug("The exit status of app with invoker (booster-e) is : %d" %app_st_we_inv)

        self.assert_(app_st_wo_inv == app_st_w_inv, "The invoker returns a wrong exit status for booster-m")
        self.assert_(app_st_wo_inv == app_st_we_inv, "The invoker returns a wrong exit status for booster-e")

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_exit_status(False)

    def test_invoker_search_prog(self, sighup = True):
        """
        Test that invoker can find programs from directories listed in
        PATH environment variable and that it doesn't find something
        that isn't there.
        """

        # invoker searches PATH for the executable
        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'm', arg = '--no-wait')
        try:
            self.waitForAsertEqual(lambda: p.poll(), 0, "Couldn't launch fala_ft_hello")
        finally:
            wait_for_app('fala_ft_hello')
            kill_process('fala_ft_hello')

        # launch with relative path

        p = run_cmd_as_user('cd /usr/share/; invoker --test-mode --type=m --no-wait ' +
                            "../bin/fala_ft_hello")
        try:
            self.waitForAsertEqual(lambda: p.poll(), 0, "Couldn't launch fala_ft_hello" +
                        " with relative path or invoker didn't terminate")
        finally:
            wait_for_app('fala_ft_hello')
            kill_process('fala_ft_hello')

        # and finally, try to launch something that doesn't exist
        p = run_cmd_as_user('invoker --test-mode --type=m --no-wait spam_cake')
        try :
            self.waitForAsertEqual(lambda: p.poll(), 1, "spam_cake has been found!")
        finally:
            kill_process('spam_cake')

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_search_prog(False)

    def test_invoker_gid_uid(self, sighup = True):
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
        app = "invoker --test-mode --type=m --no-wait /usr/bin/fala_status"
        st, op = commands.getstatusoutput('su user -c "%s"' %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        debug("Invoker %s \tInvoker %s" %(usr_id, grp_id))

        #get application's uid and gid by running the application without invoker in user mode
        debug("get application's uid and gid by running the application without invoker in user mode")
        app = "/usr/bin/fala_status"
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
        app = "invoker --test-mode --type=m --no-wait /usr/bin/fala_status"
        st, op = commands.getstatusoutput("%s" %app );
        usr_id = op.split('\n')[1]
        grp_id = op.split('\n')[2]
        debug("Invoker %s \tInvoker %s" %(usr_id, grp_id))

        #get id by running the application without invoker in root mode
        debug("get application's uid and gid  by running the application without invoker in root mode")
        app = "/usr/bin/fala_status"
        st, op = commands.getstatusoutput("%s" %app );
        usr_id2 = op.split('\n')[-2]
        grp_id2 = op.split('\n')[-1]
        debug("Application %s \tApplication %s" %(usr_id2, grp_id2))

        self.assert_(usr_id == usr_id1, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id1, "The correct GID is not passed by invoker")

        self.assert_(usr_id == usr_id2, "The correct UID is not passed by invoker")
        self.assert_(grp_id == grp_id2, "The correct GID is not passed by invoker")

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_gid_uid(False)


    def test_invoker_param_creds(self):
        p = run_cmd_as_user('invoker --creds')
        self.assert_(p.wait() == 0, "'invoker --creds' failed")

    def test_invoker_param_respawn_delay(self, sighup = True):
        p = run_cmd_as_user('invoker --respawn 10 --type=q --no-wait fala_ft_hello')
        try :
            startTime = time.time()
            wait_for_app('fala_ft_hello')
            pid = wait_for_app('booster-q', timeout = startTime - time.time() + 12)
            endTime = time.time()
            self.assert_(endTime-startTime>8, "'booster-q' was respawned too soon: %.1fs" %(endTime-startTime))
            self.assert_(pid != None, "'booster-q' was not respawned on time.")

            p = run_cmd_as_user('invoker --respawn 256 --type=q --no-wait fala_ft_hello')
            self.assert_(p.wait() != 0, "invoker didn't die with too big respawn delay")
        finally:
            kill_process('fala_ft_hello')

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_param_respawn_delay(False)

    def test_invoker_bogus_apptype(self):
        p = run_cmd_as_user('invoker --type=foobar fala_ft_hello')
        self.assert_(p.wait() != 0, "invoker didn't die with bogus apptype")

        p = run_cmd_as_user('invoker fala_ft_hello')
        self.assert_(p.wait() != 0, "invoker didn't die with empty apptype")

    def test_invoker_signal_forward(self, sighup = True):
        """
        Test that UNIX signals are forwarded from
        invoker to the invoked process
        """
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_inv_sf.py")
        debug("The Invoker killed by : <%s>" %op)

        self.assert_(op.find('Segmentation fault')!=-1, "The invoker(m-booster) was not killed by the same signal. Result:\n%s" %(op))

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_signal_forward(False)

    def test_wrong_type(self):
        """
        Test that invoker help is printed if application is
        invoked using a wrong type
        """
        cmd = '/usr/bin/invoker --type=f /usr/bin/fala_wl'
        debug("Launch application with wrong type %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(op.split("\n")[0] == 'invoker: error: Unknown application type: f ',"No Error displayed")
        self.assert_(op.split("\n")[2] == 'Usage: invoker [options] [--type=TYPE] [file] [args]',"No usage displayed")

    def test_unknown_parameter(self):
        """
        Test that help is printed if unknown parameter
        is passed to invoker
        """
        cmd = '/usr/bin/invoker --test-mode --type=m --x /usr/bin/fala_wl'
        debug("Launch application with wrong option %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(op.split("\n")[2] == 'Usage: invoker [options] [--type=TYPE] [file] [args]',"No usage displayed")

    def test_non_existing_binary_launch(self):
        """
        Test that invoker gives error when it tries to launch
        a binary that does not exist
        """
        cmd = '/usr/bin/invoker --test-mode --type=m /usr/bin/fala_foo'
        debug("Launch a non existing application %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(st != 0,"The application was launched")

    def test_invoker_symlink(self):
        """
        Test that the symlink of invoker is unable to launch applications
        """
        os.system("ln -s /usr/bin/invoker /usr/bin/invoker_link")
        cmd = "/usr/bin/invoker_link --test-mode --type=m /usr/bin/fala_wl"
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        os.system("rm /usr/bin/invoker_link")
        self.assert_(st != 0,"The application was launched")

    def test_invoker_usage_printing(self):
        """
        Test that applauncherd invoker --help usage printing ok
        """

        p = run_cmd_as_user('invoker --help')
        self.assert_(p.wait() == 0, "'invoker --help' failed")

    def test_invoker_help_printing(self):
        """
        Test that applauncherd invoker usage printing ok
        """

        p = run_cmd_as_user('invoker')
        self.assert_(p.wait() == 1, "'invoker' failed")

    def test_app_link(self, sighup = True):
        """
        Test that symlink of an application can be launched.
        """
        os.system("ln -s /usr/bin/fala_wl /usr/bin/fala_link")
        cmd = "/usr/bin/invoker --test-mode --type=m /usr/bin/fala_link"
        os.system('su - user -c "%s"&' %cmd)
        pid = wait_for_app("fala_link")
        os.system("rm /usr/bin/fala_link")
        kill_process("fala_link")
        self.assert_(pid != None ,"The application was not launched")

        if(sighup):
            self.sighup_applauncherd()
            self.test_app_link(False)

    def test_D_and_G_option(self, sighup = True):
        """
        Test that -D and -G options work and applications are launcherd
        -G = INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL
        -D = INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP
        """
        p = run_app_as_user_with_invoker("/usr/bin/fala_wl", booster='m', arg = '-G')
        wait_for_app('fala_wl')
        returnCode = p.poll()
        kill_process("fala_wl")
        p.wait()
        self.assert_(returnCode == None ,"The application was not launched")

        p = run_app_as_user_with_invoker("/usr/bin/fala_wl", booster='m', arg = '-D')
        wait_for_app('fala_wl')
        returnCode = p.poll()
        kill_process("fala_wl")
        p.wait()
        self.assert_(returnCode == None ,"The application was not launched")

        if(sighup):
            self.sighup_applauncherd()
            self.test_D_and_G_option(False)

    def test_app_directory(self, sighup = True):
        """
        Test that invoker is unable to launch a application which is a directory
        """
        #Test for a directory
        os.system("mkdir /usr/bin/fala_dir")
        st, op = commands.getstatusoutput("/usr/bin/invoker --test-mode --type=m /usr/bin/fala_dir")
        os.system("rm -rf /usr/bin/fala_dir")
        self.assert_(st != 0 ,"The application was not launched")

        if(sighup):
            self.sighup_applauncherd()
            self.test_app_directory(False)

    def test_unsetPATH_launch(self):
        """
        Unset the PATH env variable and try to launch an application with
        relative path. The launch should fail
        """
        st, op = commands.getstatusoutput("(unset PATH;/usr/bin/invoker --test-mode --type=m fala_wl)")
        self.assert_(st != 0, "The application was launched")
        self.assert_(op == "invoker: died: could not get PATH environment variable", "The application was launched")

    def test_invoker_wait_term(self, sighup = True):
        """
        start application with --wait-term parameter.
        Check that application is started and invoker is waiting termination before exit
        """

        kill_process('fala_wait')

        # launch an app with invoker --wait-term
        debug("launching fala_wait ...")
        p = Popen(['/usr/bin/invoker', '--type=m', '--test-mode', '--wait-term',
                   '/usr/bin/fala_wait'],
                  shell=False,
                  stdout=DEV_NULL, stderr=DEV_NULL, preexec_fn=permit_sigpipe)

        # wait a little
        debug("waiting ...")
        wait_for_app('fala_wait', sleep = 0.3)

        # fala_wait is quits after 6 seconds
        endTime = time.time() + 6
        success = True
        while time.time()<endTime-1 :
            if p.poll() != None:
                debug("invoker has end working before it was expected")
                success = False
                break
            time.sleep(0.5)

        if success :
            debug("waiting for invoker to terminate ...")
        self.waitForAsert(lambda : p.poll() != None,
                          "invoker didn't terminate")
        debug("...invoker has been terminated.")
        self.assertEqual(get_pid('fala_wait'), None, "invoker died before fala_wait application.")
        self.assert_(success, "invoker has ended execution before expected time")

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_wait_term(False)

    def test_relative_path_search(self, sighup = True):
        """
        Test that invoker searches the application through relative path
        """
        os.system("(cd /usr;export PATH=bin;/usr/bin/invoker --test-mode --type=m fala_wl&)")
        pid = wait_for_app("fala_wl")
        kill_process("fala_wl")
        self.assert_(pid != None ,"The application was not launched")

        if(sighup):
            self.sighup_applauncherd()
            self.test_relative_path_search(False)

    def test_oom_adj_minus_two(self, sighup = True):
        """
        Test that oom.adj is -2 for launched application process when using
        invokers -o param
        """

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'm',
                                         arg = '-o')
        pid = wait_for_app(PREFERED_APP)
        st, op = commands.getstatusoutput('cat /proc/%s/oom_adj' % pid)
        kill_process(PREFERED_APP)
        self.assert_(op == '-2', "oom.adj of the launched process is not -2."
                     "cat return code: %s, oom_adj contains: '%s'" %(st, op))

        if(sighup):
            self.sighup_applauncherd()
            self.test_oom_adj_minus_two(False)


    def test_invoker_applauncherd_dies(self):
        """
        Test that invoker kills application and exits if applauncherd dies
        """
        if get_pid(PREFERED_APP) != None:
            kill_process(PREFERED_APP)  #just to be sure application is not running prior the test

        if get_pid('applauncherd') == None:
            start_applauncherd() #just to be sure applauncherd is running

        p = run_app_as_user_with_invoker(PREFERED_APP, booster = 'm')

        pid = wait_for_app(PREFERED_APP)
        self.assert_(pid != None, "The application was not launched")

        stop_applauncherd()

        self.assert_(wait_for_process_end('applauncherd'), "applauncherd was not stoped.")
        self.waitForAsert(lambda:p.poll() != None, "Invoker didn't terminate.")
        self.assert_(wait_for_process_end(PREFERED_APP), "invoker didn't kill application after applauncherd termination.")

    def test_invoker_dbus_security(self, sighup = True):
        """
        Test that invoker doesn't allow not-privileged application
        to re-use restricted d-bus connection opened by m-booster
        """
	#run without launcher
	cmd = '/usr/bin/fala_dbus'
	st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
	errorName = op.split('\n')[0]
	self.assert_(errorName.endswith('Error.AccessDenied'),"fala_dbus was able to connect to restricted d-bus service\n output: %s" %op)

	#run with launcher (m-booster). invoker not in test mode
	cmd = 'invoker --type=m /usr/bin/fala_dbus'
	st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
	errorName = op.split('\n')[0]
	self.assert_(errorName.endswith('Error.AccessDenied'),"fala_dbus was able to use m-booster to connect to restricted d-bus service \n output: %s" %op)

	#run with launcher (m-booster). invoker in test mode
	cmd = 'invoker --test-mode --type=m /usr/bin/fala_dbus'
	st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
	errorName = op.split('\n')[0]
	self.assert_(errorName.endswith('Error.NoReply'),"fala_dbus was not able to use m-booster in test-mode to connect to restricted d-bus service \n output: %s" %op)


        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_dbus_security(False)


    def test_invoker_test_mode(self, sighup = True):
        """
        Test that invoker test mode is disabled if
        test mode control file doesn't exist
        """
        #remove test mode control file
	os.system("rm -f /root/.itm")		
	#run with launcher (m-booster). Try to start invoker in test mode 
	cmd = 'invoker --test-mode --type=m /usr/bin/fala_dbus'
	st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        #restore test mode control file
        os.system("touch /root/.itm")		
	output = op.split('\n')
	self.assert_(len(output) > 2, "No error reported by invoker running test mode without test mode control file. \n output: %s " %op)
	error = output[2];
        self.assert_(error.endswith("Invoker test mode is not enabled."), "Incorrect error reported by invoker running test mode without test mode control file. \n output: %s " %op)

        if(sighup):
            self.sighup_applauncherd()
            self.test_invoker_dbus_security(False)

# main
if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    tests = sys.argv[1:]
    mysuite = unittest.TestSuite(map(InvokerTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)
    if not result.wasSuccessful():
        sys.exit(1)
    sys.exit(0)
