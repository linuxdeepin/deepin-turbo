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
    COUNT = 5 
    @start_time = 0
    @end_time = 0
    
    $path = string = `echo $PATH `

    # method called before any test case
    def setup
        if $path.include?("scratchbox")
            puts "Inside SB, Do Nothing to unlock"
        else
	    system "mcetool --set-tklock-mode=unlocked"
            # restart duihome so that qttasserver notices it
            verify { 
                system("/sbin/initctl restart xsession/duihome")
            }
            sleep (5)
        end
        @sut = TDriver.sut(:Id=> 'sut_qt_maemo')
    end

    # method called after any test case for cleanup purposes
    def teardown
        puts "exit from teardown"
    end

    def open_Apps(appName)
        #Remove the Log file if it exists
        if FileTest.exists?("/tmp/testapp.log")
          system "rm /tmp/testapp.log"
        end

        count = 0

        #Open the Application from the application grid
        @meegoHome = @sut.application(:name => 'duihome')
        @meegoHome.MButton(:name => "ToggleLauncherButton").tap
        sleep(2)
        if @meegoHome.test_object_exists?("LauncherButton", :text => appName)
            icon = @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName)
            totalPages = @meegoHome.children(:type => 'LauncherPage').length
            while icon.attribute('visibleOnScreen') == 'false' and count < totalPages
                @meegoHome.PagedViewport(:name => 'LauncherPagedViewport').MWidget(:name => 'glass').gesture(:Left, 1, 800)
                sleep(0.2)
                count = count +1 
                icon.refresh
            end
            @meegoHome.LauncherButton(:name => "LauncherButton", :text => appName).tap
            sleep (2)
            @app = @sut.application(:name => appName)
            sleep (2)
            @app.MEscapeButtonPanel.MButton( :name => 'CloseButton' ).tap
       else
            #icon does not
            #raise error and exit
            raise "Application not found in Application grid"
            exit 1
       end
    end

    def read_file
       #Reading the log file to get the time
       file_name="/tmp/testapp.log"
       last_line = `tail -n 2 #{file_name}`
       @start_time = `head -n 1 #{file_name}`
       @end_time = last_line.split(" ")[0] 
    end

    def measure_time
       #Measuring the Startup Time for applications
       start_t = "%10.6f" % @start_time.to_f
       app_t = Float(@end_time) - Float(start_t)
       return app_t
    end

    def test_performance
      wL = []
      woL = []
      wLsum = 0.0
      woLsum = 0.0
     
      #Run Application with invoker
      for i in 1..COUNT
          open_Apps("fala_wl")
          print "Now Launching fala_wl %d times\n" %i
          sleep (5)
          read_file
          wL.push(measure_time)
      end

      #Run Application without invoker
      for i in 1..COUNT
          open_Apps("fala_wol")
          print "Now Launching fala_wol %d times\n" %i
          sleep (5)
          read_file
          woL.push(measure_time)
      end

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
