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

    def test_001_daemon_list(self):
        """
        To List the daemons running using invoker after boot
        """
        st, op = commands.getstatusoutput("ps ax | grep invoker")
        count = len(op.split("\n"))
        for i in xrange(count-2):
            if op.split("\n")[i].find("prestart") == -1:
                debug("%s" %op.split("\n")[i])

if __name__ == '__main__':

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(DaemonTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
