#!/usr/bin/ruby
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
#  * Description: Performance Test for applauncherd 
#   
#  * Objectives: test the startup time for applications
#    

require 'tdriver'
require 'date'
require 'test/unit'
include TDriverVerify


class TC_PerformanceTests < Test::Unit::TestCase
    COUNT = 3
    APP_WITH_LAUNCHER = 'fala_wl' 
    APP_WITHOUT_LAUNCHER = 'fala_wol' 
    @start_time = 0
    @end_time = 0
    @pos = 0
    
    $path = string = `echo $PATH `

    # method called before any test case
    def setup
        if $path.include?("scratchbox")
            puts "Inside SB, Do Nothing to unlock"
        else
	    system "mcetool --set-tklock-mode=unlocked"

            # restart duihome so that qttasserver notices it
            # NOTE: Remove the cludge after duihome -> meegotouchhome renaming is complete
            if not system("/sbin/initctl restart xsession/duihome")
                system("/sbin/initctl restart xsession/mthome")
            end

            system("initctl stop xsession/sysuid")
            sleep (5)
        end
        @sut = TDriver.sut(:Id=> 'sut_qt_maemo')
    end

    # method called after any test case for cleanup purposes
    def teardown
        puts "exit from teardown"
        system("initctl start xsession/sysuid")
    end

    def open_Apps(appName)
        #Remove the Log file if it exists
        if FileTest.exists?("/tmp/app_xresponse.log")
          system "rm /tmp/app_xresponse.log"
        end
        
        #Open the Application from the application grid
        begin
            @meegoHome = @sut.application(:name => 'duihome')
        rescue MobyBase::TestObjectNotFoundError
            @meegoHome = @sut.application(:name => 'meegotouchhome')
        end

        sleep(2)
        if @meegoHome.test_object_exists?("LauncherButton", :text => appName)
            icon = @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName)
            while icon.attribute('visibleOnScreen') == 'false'
                @meegoHome.Launcher.MPannableViewport( :name => 'SwipePage' ).MWidget( :name => 'glass' ).gesture(:Up, 1, 300)
                sleep(0.2)
                icon.refresh
            end
	    xpos = @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName).attribute('x')
	    ypos = @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName).attribute('y')
	    @pos = "#{xpos}x#{ypos}"

            if appName == APP_WITH_LAUNCHER 
                @winId = `sh /usr/share/applauncherd-testscripts/fala_xres_wl #{@pos}`
                @winId = @winId.split("\n")[0]
            else
                system "sh /usr/share/applauncherd-testscripts/fala_xres_wol #{@pos}"
            end 
 
            @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName).tap
            sleep (2)
            @app = @sut.application(:name => appName)
            sleep (2)
            @app.MEscapeButtonPanel.MButton( :name => 'CloseButton' ).tap
            system "pkill xresponse"
       else
            #icon does not
            #raise error and exit
            raise "Application not found in Application grid"
            exit 1
       end
    end

    def read_file(appName)
        #Reading the log file to get the time
	file_name="/tmp/app_xresponse.log"
	search_str1 = "Button 1 pressed at #{@pos}"
	x = check_file('/tmp/app_xresponse.log', search_str1)
	@start_time = x.split(':')[0].split('ms')[0]
        
        if appName == APP_WITH_LAUNCHER 
            search_str2 = "Got damage event 864x480+0+0 from #{@winId}"
            y = check_file('/tmp/app_xresponse.log', search_str2)
            @end_time = y.split(':')[0].split('ms')[0]
        else
            search_wId = check_file('/tmp/app_xresponse.log',APP_WITHOUT_LAUNCHER ) 
            @winId = search_wId.split(" ")[6]
            search_str2 = "Got damage event 864x480+0+0 from #{@winId}"
            y = check_file('/tmp/app_xresponse.log', search_str2)
            @end_time = y.split(':')[0].split('ms')[0]
        end
    end
 
    def check_file( file, string )
	File.open( file ) do |io|
	io.each {|line| line.chomp! ; 
		return line if line.include? string}
	end
	nil
    end

    def measure_time
       #Measuring the Startup Time for applications
       app_t = @end_time.to_i - @start_time.to_i
       return app_t
    end

    def test_performance
      wL = []
      woL = []
      wLsum = 0.0
      woLsum = 0.0
     
      #Run Application with invoker
      for i in 1..COUNT
          print "Now Launching  APP_WITH_LAUNCHER %d times\n" %i
          open_Apps(APP_WITH_LAUNCHER)
          sleep (5)
          read_file(APP_WITH_LAUNCHER)
          wL.push(measure_time)
      end


      #Run Application without invoker
      for i in 1..COUNT
          print "Now Launching APP_WITHOUT_LAUNCHER %d times\n" %i
          open_Apps(APP_WITHOUT_LAUNCHER)
          sleep (5)
          read_file(APP_WITHOUT_LAUNCHER)
          woL.push(measure_time)
      end
      print "Startup time in milliseconds\n"
      print "With Launcher \t\t Without Launcher\n"

      #Printing the data
      for i in 0..COUNT-1
          print "%.2f \t\t\t %.2f\n" %[wL[i],woL[i]]
          wLsum = wLsum + wL[i]
          woLsum = woLsum + woL[i]
      end
      print "\nAverage Values \n"
      print "%.2f \t\t\t %.2f\n\n" %[wLsum/COUNT, woLsum/COUNT]

       
    end
end
