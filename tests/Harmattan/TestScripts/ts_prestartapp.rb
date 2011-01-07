#!/usr/bin/ruby
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
#
#
#  * Description: Testcases for the prestart functionality 
#   
#  * Objectives: Test that a prestarted application can be 
#    launched as well    

require 'tdriver'
require 'date'
require 'test/unit'
include TDriverVerify

# When run by testrunner in scratchbox, the PATH environment variable
# is missing some vital entries ...
if ENV.has_key?('_SBOX_DIR')
    ENV['PATH'] += ":" + ENV['_SBOX_DIR'] + "/tools/bin"
end

class TC_PRESTARTLAUNCHTESTS < Test::Unit::TestCase

    # method called before any test case
    def setup
        system "mcetool --set-tklock-mode=unlocked"
        system "initctl stop xsession/applifed"
        @sut = TDriver.sut(:Id=> 'sut_qt_maemo')    
    end
    
    # method called after any test case for cleanup purposes
    def teardown
        system "initctl start xsession/applifed"
    end

    def get_pids(app)
        pids = `pgrep #{app}`.split(/\s/).collect { |x| x.strip() }.delete_if { |x| x.empty? }
        pids = nil if pids.empty?
        return pids
    end


    def test_launch_prestarted_app
        #Test that a prestarted application can be launched
	@appname = 'fala_wl.launch'
	if system("pgrep #{@appname}") == true
	    system("kill -9 `pgrep #{@appname}`")
	end
	verify_equal(false,2,"Application is Prestarted"){
		system "pgrep #{@appname}"}

        #Prestart application using invoker
        system("su - user -c '/usr/bin/invoker --type=m --no-wait #{@appname} -prestart'&") 
        sleep(2)
     
        #verify that the application is prestarted and it has a valid pid
        pid = get_pids(@appname)
        verify_true(2,"The Application was not prestarted"){pid != nil}

        @wid = `xwininfo -root -tree| awk '/Applauncherd testapp/ {print $1}'` 
        @wid = @wid.split(/\n/)[0]

        #verify that Window state of applications is blank as the application is not launhced 
        @switcher =  `xprop -id #{@wid} | awk '/window state/{print $1}'`
        verify_equal("",2,"Application was Launched"){@switcher}

        #Now Launch the prestarted application 
        system("su - user -c '/usr/bin/invoker --type=m --no-wait #{@appname}'")
        sleep(2)

        #verify that Window state of applications is Normal as the application is launhced 
        @switcher =  `xprop -id #{@wid} | awk '/window state/{print $3}'`

        verify_equal("Normal", 2,\
               "The Application window was not launched") {@switcher.split(/\n/)[0]}
        
        #Close application using xsendevent
        system("/usr/bin/xsendevent close #{@wid}")

        #verify that the application is goes to lazyshutdown and has the same pid
        npid = get_pids(@appname)
        verify_true(2,"The Application was killed"){pid != nil}
        verify_true(2,"The Application was Restarted"){npid == pid}

        #verify that Window state of applications is blank as the application is not launhced 
        @switcher =  `xprop -id #{@wid} | awk '/window state/{print $1}'`
        verify_equal("",2,"Application was Launched"){@switcher}

        #verify that prestarted state of the application is 1 
        @switcher =  `xprop -id #{@wid} | awk '/_MEEGOTOUCH_PRESTARTED/{print $3}'`
        verify_equal("1",2,"Application was Launched"){@switcher.split(/\n/)[0]}
    end

end
