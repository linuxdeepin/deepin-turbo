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
import fnmatch

APP = 'fala_ft_hello'

def check_prerequisites():
    if os.getenv('DISPLAY') == None:
        error("DISPLAY is not set. Check the requirements.")
        
    if os.getenv('DBUS_SESSION_BUS_ADDRESS') == None:
        error("DBUS_SESSION_BUS_ADDRESS is not set.\n" +
              "You probably want to source /tmp/session_bus_address.user")


class CoreDumpTests(unittest.TestCase):
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

    def get_cores(self, app):
        cores = os.listdir('/home/user/MyDocs/core-dumps')

        debug('cores found:')
        for core in cores:
            debug(core)
        
        cores = fnmatch.filter(cores, app + '*.rcore.lzo')

        debug('%d cores match' % len(cores))
        
        return cores

    def test_core_dumps(self):
        self.assert_(get_pid('booster-m') != None, "applauncherd is not running!")
        
        run_app_as_user_with_invoker(APP, arg = '--no-wait')

        time.sleep(3)

        pid = wait_for_app(APP)
        self.assert_(pid != None, APP + " is not running!")
        pid = pid.splitlines()[0]

        # just in case :-)
        cores_before = self.get_cores(APP)

        # send SEGV
        kill_process(APP, signum = 11)

        # give some time to the rich-core-dumper to make its thing :-)
        time.sleep(10)

        cores_after = self.get_cores(APP)

        # we should have at least one matching core more now than before sending SIGV
        self.assert_(len(cores_after) - len(cores_before) >= 1, "core wasn't generated!")


if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    check_prerequisites()

    # start applauncherd if it's not started
    if get_pid('applauncherd') == None:
        remove_applauncherd_runtime_files()
        start_applauncherd()

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(CoreDumpTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    # kill applauncherd if it's left running and restart it
    kill_process('applauncherd')
    restart_applauncherd()

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
