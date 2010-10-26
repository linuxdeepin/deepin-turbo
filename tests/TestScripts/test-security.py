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

import unittest
from utils import *

class SecurityTests(unittest.TestCase):
    def filter_creds(self, creds):
        """
        Filter out some unnecessary cruft from the test point of view
        """

        def f(x):
            return (x[:3] != "SRC" and
                    x[:3] != "AID" and
                    x != "applauncherd-testapps::applauncherd-testapps")

        return filter(f, creds)

    def user_creds(self, suppl = False):
        """
        Returns the user id, group id and optionally supplementary
        groups as credential tokens.
        """

        groups = []

        if suppl:
            groups = get_groups_for_user()

            def f(x):
                return 'GRP::' + x

            groups = map(f, groups)

        return ['UID::user', 'GID::users'] + groups
        
    def test_001(self):
        """
        Test that the fala_ft_creds* applications have the correct
        credentials set (check aegis file included in the debian package)
        """
        creds1 = launch_and_get_creds('/usr/bin/fala_ft_creds1')
        creds2 = launch_and_get_creds('/usr/bin/fala_ft_creds2')

        self.assert_(creds1 != None, "couldn't get credentials")
        self.assert_(creds2 != None, "couldn't get credentials")

        creds1 = self.filter_creds(creds1)
        creds2 = self.filter_creds(creds2)

        debug("fala_ft_creds1 has %s" % ', '.join(creds1))
        debug("fala_ft_creds2 has %s" % ', '.join(creds2))

        # When an application has a manifest, the users supplementary
        # groups are not by default included in the credential list,
        # only UID and GID.

        # required caps for fala_ft_creds1
        cap1 = ['tcb', 'drm', 'CAP::setuid', 'CAP::setgid',
                'CAP::setfcap'] + self.user_creds()

        # required caps for fala_ft_creds2
        cap2 = ['Cellular'] + self.user_creds()

        cap1.sort()
        cap2.sort()

        creds1.sort()
        creds2.sort()

        self.assert_(cap1 == creds1, "fala_ft_creds1 has incorrect credentials")
        self.assert_(cap2 == creds2, "fala_ft_creds2 has incorrect credentials")

    def test_002_no_aegis_Bug170905(self):
        """
        Check that an application that doesn't have aegis file doesn't
        get any funny credentials.
        """

        creds = launch_and_get_creds('/usr/bin/fala_ft_hello')
        debug("fala_ft_hello has %s" % ', '.join(creds))

        self.assert_(creds != None, "error retrieving credentials")

        # Credentials should be dropped, but uid/gid + groups retained
        req_creds = self.user_creds(True)

        creds.sort()
        req_creds.sort()

        print "APP HAS:  " + ', '.join(creds)
        print "REQUIRED: " + ', '.join(req_creds)

        self.assert_(creds == req_creds,
                     "fala_ft_hello has incorrect credentials")

    def test_003_invoker_creds(self):
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
        pid = get_pid(Testapp.replace('.launch', ''))
        self.assert_((st == 0), "Application was not launched using launcher")
        self.assert_(not (pid == None), "Application was not launched using launcher: actual pid%s" %pid)
        print pid
        #self.kill_process(Testapp.replace('.launch', ''))       
        kill_process(apppid=pid)  
        pid = get_pid(Testapp.replace('.launch', '')) 
        self.assert_((pid == None), "Application still running")        
        
        #launching the testapp with fake invoker
        st = os.system('%s --type=m %s'%(FAKE_INVOKER_BINARY, Testapp)) 
        pid = get_pid(Testapp.replace('.launch', ''))
        self.assert_(not (st == 0), "Application was launched using fake launcher")
        self.assert_((pid == None), "Application was launched using fake launcher")

    def test_004(self):
        """
        Compare the credentials of invoker and the launched application
        when applauncherd is running and when it's not. Verify that
        the credentials are different.

        See NB#183566, NB#187583
        """

        def do_it():
            """
            A little helper to keep the logic flowing. Does the actual
            work of getting credentials for invoker and app
            """

            # launch an application, leave invoker running
            print "launching application"
            invoker = Popen(['invoker', '--type=m', '--wait-term',
                             'fala_ft_hello.launch'], shell = False,
                            stdout = DEV_NULL, stderr = DEV_NULL)

            # get pid of invoker
            invoker_pid = invoker.pid

            print "invoker pid = %s" % invoker_pid

            # get credentials
            invoker_creds = get_creds(pid = invoker_pid)
            app_creds = get_creds(path = 'fala_ft_hello')

            self.assert_(invoker_creds != None,
                         "error retrieving creds for invoker")
            self.assert_(app_creds != None,
                         "error retrieving creds for fala_ft_hello")

            invoker_creds.sort()
            app_creds.sort()

            print "invoker creds = %s" % invoker_creds
            print "app creds = %s" % app_creds

            kill_process('fala_ft_hello')

            return (invoker_creds, app_creds)

        # creds when applauncherd is running
        creds1 = do_it()

        # stop applauncherd
        Popen(['initctl', 'stop', 'xsession/applauncherd']).wait()
        time.sleep(2)

        # remove sockets
        try:
            for f in glob.glob('/tmp/boost*'):
                os.remove(f)
        except Exception as e:
            print e

        # creds when applauncherd *is not* running
        creds2 = do_it()

        # start applauncherd
        Popen(['initctl', 'start', 'xsession/applauncherd']).wait()
        time.sleep(2)

        # app shouldn't have the same credentials as invoker, when
        # applauncherd *is* running
        self.assert_(creds1[0] != creds1[1],
                     'app creds are the same as invoker creds')

        # and the same when applauncherd is not running
        # note that the invoker doesn't show up in e.g. "ps ax"
        # because of its execing so we'll just use the creds from the
        # first step as they should be the same
        self.assert_(creds1[0] != creds2[1],
                     'app creds are the same as invoker creds when ' +
                     'applauncherd is not running')


    def test_005(self):
        """
        Launch an application as user and root both when applauncherd
        is running and when it isn't. Compare the credentials between
        the two cases and verify that they are the same.

        See NB#183566, NB#187583
        """

        # stop applauncherd
        Popen(['initctl', 'stop', 'xsession/applauncherd']).wait()
        time.sleep(2)

        # remove sockets
        try:
            for f in glob.glob('/tmp/boost*'):
                os.remove(f)
        except Exception as e:
            print e

        def do_it():
            """
            A helper function to launch application and get credentials
            as user and root.
            """

            handle = Popen(['su', '-', 'user', '-c',
                            '/usr/bin/fala_ft_hello'],
                           stdout = DEV_NULL, stderr = DEV_NULL)

            # give the application some time to launch up
            time.sleep(2)
            
            user = get_creds('fala_ft_hello')
            kill_process('fala_ft_hello')

            root = launch_and_get_creds('/usr/bin/fala_ft_hello').sort()

            return (user, root)

        # get creds for a launched application when applauncherd
        # is not running
        creds1 = do_it()

        # start applauncherd
        Popen(['initctl', 'start', 'xsession/applauncherd']).wait()
        time.sleep(2)

        # get creds for the same application when applauncherd
        # is running
        creds2 = do_it()

        # creds should be the same, regardless of applauncherd status
        self.assert_(creds1[0] == creds2[0], 'creds for user-case different')
        self.assert_(creds1[1] == creds2[1], 'creds for root-case different')

        # creds should be different, when run as user and when run as root,
        # regarless of applauncherd status
        self.assert_(creds1[0] != creds1[1],
                     'creds are same when applauncherd is not running')
        self.assert_(creds2[0] != creds2[1],
                     'creds are same when applauncherd is running')


# main
if __name__ == '__main__':
    # When run with testrunner, for some reason the PATH doesn't include
    # the tools/bin directory
    if os.getenv('_SBOX_DIR') != None:
        os.environ['PATH'] = os.getenv('PATH') + ":" + os.getenv('_SBOX_DIR') + '/tools/bin'
        using_scratchbox = True

    #check_prerequisites()
    #start_launcher_daemon()

    tests = sys.argv[1:]

    mysuite = unittest.TestSuite(map(SecurityTests, tests))
    result = unittest.TextTestRunner(verbosity=2).run(mysuite)

    if not result.wasSuccessful():
        sys.exit(1)

    sys.exit(0)
