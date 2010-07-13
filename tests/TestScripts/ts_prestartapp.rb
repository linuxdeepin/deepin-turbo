#!/usr/bin/ruby1.8
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

require 'matti'
require 'date'
require 'test/unit'
include MattiVerify

# When run by testrunner in scratchbox, the PATH environment variable
# is missing some vital entries ...
if ENV.has_key?('_SBOX_DIR')
    ENV['PATH'] += ":" + ENV['_SBOX_DIR'] + "/tools/bin"
end

class TC_PRESTARTLAUNCHTESTS < Test::Unit::TestCase

    # method called before any test case
    def setup
        system "mcetool --set-tklock-mode=unlocked"
        @sut = MATTI.sut(:Id=>ARGV[0] || 'sut_qt_maemo')    
    end
    
    # method called after any test case for cleanup purposes
    def teardown
    end

    def test_launch_prestarted_app
        #Test that a prestarted application can be launched
	@appname = 'fala_testapp'
	if system("pgrep #{@appname}") == true
	    system("kill -9 `pgrep #{@appname}`")
	end
	sleep 2    
	verify_equal(false,2,"Application is Prestarted"){
		system "pgrep #{@appname}"}
	sleep 2

	string = `export DISPLAY=:0; source /tmp/session_bus_address.user;dbus-send --dest=com.nokia.#{@appname} --type="method_call" /org/maemo/m com.nokia.MApplicationIf.ping`
	sleep 1

	verify_equal(true,2,"Application is not Prestarted"){
		system "pgrep #{@appname}"}
	pid = string = `pgrep #{@appname}`
	sleep 1

	string = `export DISPLAY=:0; source /tmp/session_bus_address.user;dbus-send --dest=com.nokia.#{@appname} --type="method_call" /org/maemo/m com.nokia.MApplicationIf.launch`
	@app = @sut.application( :name => 'fala_testapp' ) 
	@app.MButton( :name => 'CloseButton' ).tap
	newid = string = `pgrep #{@appname}`
	verify_true(30,"The application is not prestarted"){pid == newid}
	sleep 1
	system "kill -9 `pgrep #{@appname}`"
    end
end
