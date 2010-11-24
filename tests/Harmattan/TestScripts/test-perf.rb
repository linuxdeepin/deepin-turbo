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

#require 'tdriver'
require 'date'
require 'test/unit'
require 'optparse'

class TC_PerformanceTests < Test::Unit::TestCase
  COUNT = 3
  PIXELCHANGED_BINARY= '/usr/bin/fala_pixelchanged' 
  TEST_SCRIPT_LOCATION = '/usr/share/applauncherd-testscripts'
  GET_COORDINATES_SCRIPT="#{TEST_SCRIPT_LOCATION}/get-coordinates.rb"
  PIXELCHANGED_LOG = '/tmp/fala_pixelchanged.log'
  FALA_GETTIME_BINARY = '/usr/bin/fala_gettime_ms'
  @start_time = 0
  @end_time = 0
  @pos = 0
  @options = {}  
 

  $path = string = `echo $PATH `
  
  # method called before any test case
  def setup

     optparse = OptionParser.new do|opts|
      options = {}  
      # Set a banner, displayed at the top
      # of the help screen.
      opts.banner = "Usage: get-coordinates.rb [options] "
      
      options[:application] = nil
      opts.on( '-a', '--application APP', 'Application name in application grid' ) do|app|
        options[:application] = app
      end

      options[:command] = nil
      opts.on( '-c', '--command_line COMMAND', 'Start application from witc COMMAND from command line instead of grid.' ) do|command|
        options[:command] = command
      end

      options[:limit] = nil
      opts.on( '-l', '--limit MILLISECONDS', 'Time limit in milliseconds. Slower startup will make test to fail.' ) do|milliseconds|
        options[:limit] = milliseconds.to_i
      end
      
      opts.on( '-h', '--help', 'Display this screen' ) do
        puts opts
        exit 0
      end
      @options=options
    end


    optparse.parse!
 

    if @options[:application] == nil &&  @options[:command] == nil
      puts "Application not defined!" 
      exit 1
    end

    if @options[:command] != nil
      puts "#{@options[:command]}" 
    end

    if $path.include?("scratchbox")
      puts "Inside SB, Do Nothing to unlock"
    else
      system "mcetool --set-tklock-mode=unlocked"
    end        
    #restart duihome so that qttasserver notices it
    #NOTE: Remove the cludge after duihome -> meegotouchhome renaming is complete
    if not system("/sbin/initctl restart xsession/duihome")
      system("/sbin/initctl restart xsession/mthome")
    end

    system("initctl stop xsession/sysuid")
    system("initctl stop xsession/applifed")
    system("initctl stop xsession/search")
    system("mv /usr/lib/qt4/plugins/testability/libtestability.so /tmp/.")
#    system("pkill call-history")
    sleep(4)

  end
  

        
  # method called after any test case for cleanup purposes
  def teardown
    puts "exit from teardown"
    system("initctl start xsession/sysuid")
    system("initctl start xsession/applifed")	
    system("initctl start xsession/search")
    system("mv /tmp/libtestability.so /usr/lib/qt4/plugins/testability/libtestability.so")
  end
  
  def open_Apps(appName)
    #Remove the Log file if it exists
    if FileTest.exists?(PIXELCHANGED_LOG)
      system "rm #{PIXELCHANGED_LOG}"
    end
    sleep(2)

    if @options[:command] != nil
      puts "#{GET_COORDINATES_SCRIPT} -g"
      system "#{GET_COORDINATES_SCRIPT} -g"
#      system "ls -l -s -R /usr/share/applications"
      start_command ="`#{PIXELCHANGED_BINARY} -q >> #{PIXELCHANGED_LOG} &`; #{FALA_GETTIME_BINARY} \"Started from command line\" >>  #{PIXELCHANGED_LOG}; #{@options[:command]} &"
      puts "start command: #{start_command}"
      system "#{start_command}"
      sleep (4)
      puts "pkill \"#{@options[:command]}\""
      system "pkill \"#{@options[:command]}\""

    else
      @pos = `#{GET_COORDINATES_SCRIPT} -a #{@options[:application]}`
    
      puts @pos
      sleep (2)
      system "#{PIXELCHANGED_BINARY} -c #{@pos} -f #{PIXELCHANGED_LOG} -q"		
      sleep (4)
      system "pkill #{appName}"
    end

  end
  
  def read_file(appName)
    #Reading the log file to get the time
    
    lines = File.open(PIXELCHANGED_LOG).readlines().collect { |x| x.split(" ")[0].to_i }
    
    # First line tells when the button is released
    @start_time = lines[0]
    puts "Clicked: #{lines[0]}"
    # Second one when the first pixel has changed its color
    @end_time = lines[1]
    puts "Pixel changed: #{lines[1]}"
    
  end
  
  
  def measure_time
    #Measuring the Startup Time for applications
    app_t = @end_time - @start_time
    return app_t
  end
  
  def test_performance
    wL = []
    wLsum = 0
    
    #Run Application with invoker
    for i in 1..COUNT
      print "Now Launching  #{@options[:application]} %d times\n" %i
      open_Apps(@options[:application])
      sleep (5)
      read_file(@options[:application])
      wL.push(measure_time)
    end
    
    
    print "\n\nStartup time in milliseconds\n"
    print "Application: #{@options[:application]} \n"
    if @options[:limit] != nil
      print "Time limit: #{@options[:limit]} \n"
    end 
    
    #Printing the data
    for i in 0..COUNT-1
      print "%d \n" %[wL[i]]
      wLsum = wLsum + wL[i]
    end
    print "\nAverage: \n"
    print "%d\n" %[wLsum/COUNT]

    if @options[:limit] != nil
      assert((wLsum/COUNT) < @options[:limit], "Application: #{@options[:application]} avarage startup was slower than #{@options[:limit]} ms")
    end
    
  end
end



