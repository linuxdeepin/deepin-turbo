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
    PIXELCHANGED_BINARY= '/usr/bin/fala_pixelchanged' 
    TEST_SCRIPT_LOCATION = '/usr/share/applauncherd-M-testscripts'
    PIXELCHANGED_LOG = '/tmp/fala_pixelchanged.log'
    @start_time = 0
    @end_time = 0
    @pos = 0
    
    $path = string = `echo $PATH `

    # method called before any test case
    def setup

        @sut = TDriver.sut(:Id=> 'sut_qt_maemo')
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
            system("initctl stop xsession/applifed")
   

	end


        
    end

    # method called after any test case for cleanup purposes
    def teardown
        puts "exit from teardown"
        system("initctl start xsession/sysuid")
        system("initctl start xsession/applifed")	
    end

    def open_Apps(appName)
        #Remove the Log file if it exists
        if FileTest.exists?(PIXELCHANGED_LOG)
          system "rm #{PIXELCHANGED_LOG}"
        end
    	appOnTop = @sut.application()	
        while appOnTop.attribute('objectName') != 'meegotouchhome'
	    fullName = appOnTop.attribute('FullName')
	    puts "Now killing #{fullName} from the top"
	    system "pkill #{fullName}"
	    appOnTop = @sut.application()
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

	    puts @pos
	    sleep (2)
	    system "#{PIXELCHANGED_BINARY} -c #{@pos} -f #{PIXELCHANGED_LOG} -q"		
            sleep (4)
            system "pkill #{appName}"
       else
            #icon does not
            #raise error and exit
            raise "Application not found in Application grid"
            exit 1
       end
    end

    def read_file(appName)
        #Reading the log file to get the time
	
	lines = File.open(PIXELCHANGED_LOG).readlines().collect { |x| x.split(" ")[0].to_i }
        
	# First line tells when the button is released
	@start_time = lines[0]
	puts "Line1: #{lines[0]}"
	# Second one when the first pixel has changed its color
	@end_time = lines[1]
	puts "Line2: #{lines[1]}"
	
    end
 

    def measure_time
       #Measuring the Startup Time for applications
       app_t = @end_time - @start_time
       return app_t
    end

    def test_performance
      wL = []
      woL = []
      wLsum = 0
      woLsum = 0
     
      #Run Application with invoker
      for i in 1..COUNT
          print "Now Launching  #{APP_WITH_LAUNCHER} %d times\n" %i
          open_Apps(APP_WITH_LAUNCHER)
          sleep (5)
          read_file(APP_WITH_LAUNCHER)
          wL.push(measure_time)
      end


      #Run Application without invoker
      for i in 1..COUNT
          print "Now Launching #{APP_WITHOUT_LAUNCHER} %d times\n" %i
          open_Apps(APP_WITHOUT_LAUNCHER)
          sleep (5)
          read_file(APP_WITHOUT_LAUNCHER)
          woL.push(measure_time)
      end
      print "Startup time in milliseconds\n"
      print "With Launcher \t\t Without Launcher\n"

      #Printing the data
      for i in 0..COUNT-1
          print "%d \t\t\t %d\n" %[wL[i],woL[i]]
          wLsum = wLsum + wL[i]
          woLsum = woLsum + woL[i]
      end
      print "\nAverage Values \n"
      print "%d \t\t\t %d\n\n" %[wLsum/COUNT, woLsum/COUNT]

       
    end
end



