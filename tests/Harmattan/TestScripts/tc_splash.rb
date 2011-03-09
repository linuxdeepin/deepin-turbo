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


require 'tdriver'
require 'date'
require 'test/unit'
include TDriverVerify

LandscapeImg = '/usr/share/fala_images/landscape.jpg'
PortraitImg = '/usr/share/fala_images/portrait.jpg'
TestApp = '/usr/bin/fala_wl'
No_Splash_App = '/usr/bin/fala_ft_hello'


class TC_Splash < Test::Unit::TestCase

    def setup
        print_debug("In Setup")
        apps = ['fala_wl', 'fala_ft_hello']
        for app in apps
            if system("pgrep #{app}") == true
                system("kill -9 `pgrep #{app}`")
            end
        end
    end

    def teardown
        apps = ['fala_wl', 'fala_ft_hello']
        for app in apps
            if system("pgrep #{app}") == true
                system("kill -9 `pgrep #{app}`")
            end
        end
        print_debug("Exiting teardown")
    end

    def print_debug(msg)
        message = "[INFO]  #{msg}\n"
        puts message
    end

    def get_compositor_wid
        x = `xprop -root | awk '/_NET_SUPPORTING_WM_CHECK/ {print $5}'`
        wid = x.split(/\n/)[0]
        print_debug("The Wid value is :#{wid}")
        return wid
    end

    def get_pid(app)
        pid = `pgrep #{app}`.split(/\s/).collect { |x| x.strip() }.delete_if { |x| x.empty? }
        print_debug("The Pid of #{app} is #{pid}")
        pid = nil if pid.empty?
        return pid
    end
    
    def test_splash
        """
        To Test that splash parameters are set when application is launched 
        using --splash
        """
        system "invoker --splash #{PortraitImg} --type=m #{TestApp} &"
        sleep(2)

        p = get_pid(TestApp)
        w = get_compositor_wid
        
        prop1 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $3}'`
        print_debug("The Prop 1 is #{prop1}")

        prop2 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $4}'`
        print_debug("The Prop 2 is #{prop2}")

        prop3 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $5}'`
        print_debug("The Prop 3 is #{prop3}")

        prop4 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $6}'`
        print_debug("The Prop 4 is #{prop4}")

        verify_equal(p[0], 2, "Wrong Pid"){prop1.split('"')[1]}
        verify_equal("Fala_wl", 2, "Wrong Application Name"){prop2.split('"')[1]}
        verify_equal(PortraitImg, 2, "Wrong splash image"){prop3.split('"')[1]}
        verify_equal("", 2, "Wrong splash image"){prop4.split('"')[1]}
    end

    def test_splash_landscape
        """
        To Test that splash-landscape parameters are set when application is launched 
        using --splash-landscape
        """
        system "invoker --splash #{PortraitImg} --splash-landscape #{LandscapeImg} --type=m #{TestApp} &"
        sleep(2)

        p = get_pid(TestApp)
        w = get_compositor_wid

        prop1 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $3}'`
        print_debug("The Prop 1 is #{prop1}")

        prop2 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $4}'`
        print_debug("The Prop 2 is #{prop2}")

        prop3 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $5}'`
        print_debug("The Prop 3 is #{prop3}")

        prop4 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $6}'`
        print_debug("The Prop 4 is #{prop4}")

        verify_equal(p[0], 2, "Wrong Pid"){prop1.split('"')[1]}
        verify_equal("Fala_wl", 2, "Wrong Application Name"){prop2.split('"')[1]}
        verify_equal(PortraitImg, 2, "Wrong splash image"){prop3.split('"')[1]}
        verify_equal(LandscapeImg, 2, "Wrong splash image"){prop4.split('"')[1]}
    end

    def test_no_splash
        """
        To Test that splash parameters remains the same even if application is  
        launched without using splash
        """
        system "invoker --splash #{PortraitImg} --splash-landscape #{LandscapeImg} --type=m #{TestApp} &"
        sleep(2)
        p = get_pid(TestApp)
        system "kill -9 #{p}"
        sleep(2)
        system "invoker --type=m #{No_Splash_App} &" 
        sleep(2)
        pid = get_pid(No_Splash_App)
        w = get_compositor_wid

        prop1 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $3}'`
        print_debug("The Prop 1 is #{prop1}")

        prop2 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $4}'`
        print_debug("The Prop 2 is #{prop2}")

        prop3 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $5}'`
        print_debug("The Prop 3 is #{prop3}")

        prop4 = `xprop -id #{w} | awk '/_MEEGO_SPLASH_SCREEN/ {print $6}'`
        print_debug("The Prop 4 is #{prop4}")

        verify_equal(p[0], 2, "Wrong Pid"){prop1.split('"')[1]}
        verify_equal("Fala_wl", 2, "Wrong Application Name"){prop2.split('"')[1]}
        verify_equal(PortraitImg, 2, "Wrong splash image"){prop3.split('"')[1]}
        verify_equal(LandscapeImg, 2, "Wrong splash image"){prop4.split('"')[1]}
        system ("kill -9 #{pid}")
    end


end
