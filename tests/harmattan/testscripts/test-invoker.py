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

class InvokerTests(unittest.TestCase):
    def setUp(self):
        debug("setUp")
        if get_pid('applauncherd') == None:
            start_applauncherd()

    def tearDown(self):
        debug("tearDown")
        if get_pid('applauncherd') == None:
            start_applauncherd()

        wait_for_single_applauncherd()

    #Testcases
    def test_invoker_signal_forward(self):
        """
        Test that UNIX signals are forwarded from 
        invoker to the invoked process
        """
        st, op = commands.getstatusoutput("/usr/share/applauncherd-testscripts/signal-forward/fala_inv_sf.py")
        time.sleep(3)
        debug("The Invoker killed by : <%s>" %op)
    
        self.assert_(op == 'Segmentation fault', "The invoker(m-booster) was not killed by the same signal")
        time.sleep(2)

    def test_wrong_type(self):
        """
        Test that invoker help is printed if application is
        invoked using a wrong type
        """
        cmd = '/usr/bin/invoker --type=f /usr/bin/fala_wl'
        debug("Launch application with wrong type %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(op.split("\n")[0] == 'invoker: error: Unknown application type: f ',"No Error displayed")
        self.assert_(op.split("\n")[2] == 'Usage: invoker [options] [--type=TYPE] [file] [args]',"No usgae displayed")

    def test_unknown_parameter(self):
        """
        Test that help is printed if unknown parameter 
        is passed to invoker
        """
        cmd = '/usr/bin/invoker --type=m --x /usr/bin/fala_wl'
        debug("Launch application with wrong option %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(op.split("\n")[2] == 'Usage: invoker [options] [--type=TYPE] [file] [args]',"No usage displayed")

    def test_non_existing_binary_launch(self):
        """
        Test that invoker gives error when it tries to launch 
        a binary that does not exist
        """
        cmd = '/usr/bin/invoker --type=m /usr/bin/fala_foo'
        debug("Launch a non existing application %s" %cmd)
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        self.assert_(st != 0,"The application was launched")

    def test_invoker_symlink(self):
        """
        Test that the symlink of invoker is unable to launch applications
        """
        os.system("ln -s /usr/bin/invoker /usr/bin/invoker_link")
        cmd = "/usr/bin/invoker_link --type=m /usr/bin/fala_wl"
        st, op = commands.getstatusoutput('su - user -c "%s"' %cmd)
        os.system("rm /usr/bin/invoker_link")
        self.assert_(st != 0,"The application was launched")

    def test_app_link(self):
        """
        Test that symlink of an application can be launched.
        """
        os.system("ln -s /usr/bin/fala_wl /usr/bin/fala_link")
        cmd = "/usr/bin/invoker --type=m /usr/bin/fala_link"
        os.system('su - user -c "%s"&' %cmd)
        pid = get_pid("fala_link")
        os.system("rm /usr/bin/fala_link")
        kill_process("fala_link")
        self.assert_(pid != None ,"The application was not launched")

    def test_D_and_G_option(self):
        """
        Test that -D and -G options work and applications are launcherd
        -G = INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL
        -D = INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP
        """
        p = run_app_as_user_with_invoker("/usr/bin/fala_wl", booster='m', arg = '-G')
        time.sleep(3)
        pid = p.pid
        kill_process("fala_wl")
        self.assert_(pid != None ,"The application was not launched")

        p = run_app_as_user_with_invoker("/usr/bin/fala_wl", booster='m', arg = '-D')
        time.sleep(3)
        pid = p.pid
        kill_process("fala_wl")
        self.assert_(pid != None ,"The application was not launched")

    def test_app_directory(self):
        """
        Test that invoker is unable to launch a application which is a directory 
        """
        #Test for a directory
        os.system("mkdir /usr/bin/fala_dir")
        st, op = commands.getstatusoutput("/usr/bin/invoker --type=m /usr/bin/fala_dir") 
        time.sleep(3)
        os.system("rm -rf /usr/bin/fala_dir")
        self.assert_(st != 0 ,"The application was not launched")

    def test_unsetPATH_launch(self):
        """
        Unset the PATH env variable and try to launch an application with
        relative path. The launch should fail
        """
        st, op = commands.getstatusoutput("(unset PATH;/usr/bin/invoker --type=m fala_wl)")
        self.assert_(st != 0, "The application was launched")
        self.assert_(op == "invoker: died: could not get PATH environment variable", "The application was launched")

    def test_invoker_wait_term(self):
        """
        start application with --wait-term parameter.
        Check that application is started and invoker is waiting termination before exit
        """

        # launch an app with invoker --wait-term
        debug("launching fala_wait ...")
        p = Popen(['/usr/bin/invoker', '--type=m', '--wait-term',
                   '/usr/bin/fala_wait'],
                  shell=False, 
                  stdout=DEV_NULL, stderr=DEV_NULL)

        # wait a little
        debug("waiting ...")
        time.sleep(5)

        success = True

        if p.poll() == None:
            debug("NOT DEAD")
        else:
            debug("DEAD")
            success = False

        debug("waiting for invoker to terminate ...")
        p.wait()

        debug("terminating fala_wait ...")

        self.assert_(success, "invoker terminated before delay elapsed")

    def test_relative_path_search(self):
        """
        Test that invoker searches the application through relative path
        """
        os.system("(cd /usr;export PATH=bin;/usr/bin/invoker --type=m fala_wl&)")
        pid = get_pid("fala_wl")
        kill_process("fala_wl")
        self.assert_(pid != None ,"The application was not launched")

        

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
