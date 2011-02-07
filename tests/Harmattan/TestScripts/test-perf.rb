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
  MATTI_LOCATION='/usr/lib/qt4/plugins/testability/libtestability.so'
  TEMPORARY_MATTI_LOCATION='/root/libtestability.so'
   

  @start_time = 0
  @end_time = 0
  @app_from_cache = 0
  @win_from_cache = 0
  @pos = 0
  @options = {}  
 

  $path = string = `echo $PATH `
  def print_debug(msg)
      message = "[INFO]  #{msg}\n"
      puts message
  end

  
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

      options[:binary] = nil
      opts.on( '-b', '--binary BINARY', 'Name of the application binary which is used when killing the application' ) do|binary|
        options[:binary] = binary
      end

      options[:command] = nil
      opts.on( '-c', '--command_line COMMAND', 'Start application from witc COMMAND from command line instead of grid.' ) do|command|
        options[:command] = command
      end

      options[:startup] = nil
      opts.on( '-s', '--startup MILLISECONDS', 'Time limit in milliseconds. Slower startup will make test to fail.' ) do|milliseconds|
        options[:startup] = milliseconds.to_i
      print_debug("The Startup limit is : #{options[:startup]}")
      end

      options[:appCache] = nil
      opts.on( '-t', '--appCache MILLISECONDS', 'Time limit in milliseconds for application from cache.' ) do|milliseconds|
        options[:appCache] = milliseconds.to_i
      print_debug("The limit for MApplication from cache is : #{options[:appCache]}")
      end

      options[:winCache] = nil
      opts.on( '-w', '--winCache MILLISECONDS', 'Time limit in milliseconds for window from cache.' ) do|milliseconds|
        options[:winCache] = milliseconds.to_i
      print_debug("The limit for MApplicationWindow from cache is : #{options[:winCache]}")
      end

      options[:logFile] = nil
      opts.on( '-f', '--logFile LOG_FILE', 'Log file which stores the timestamps' ) do|logFile|
        options[:logFile] = logFile
      end
      
      options[:pre_step] = nil
      opts.on( '-p', '--pre_step PRE_STEP', 'Command to be executed everytime before starting the application' ) do|pre_step|
        options[:pre_step] = pre_step
      end

      opts.on( '-h', '--help', 'Display this screen' ) do
        puts opts
        exit 0
      end
      @options=options
    end


    optparse.parse!
 

    if @options[:application] == nil &&  @options[:command] == nil
      print_debug ("Application not defined!")
      exit 1
    end

    if @options[:binary] == nil
      print_debug ("Binary of the application not defined!")
      exit 1
    end

    if @options[:logFile] != nil
      print_debug ("The logFile is: #{@options[:logFile]}")
    end

    if @options[:command] != nil
      print_debug ("The command to launch is #{@options[:command]}")
    end

    if @options[:pre_step] != nil
      print_debug ("The Pre-steps is :#{@options[:pre_step]}" )
    end

    if $path.include?("scratchbox")
      print_debug ("Inside SB, Do Nothing to unlock")
    else
      print_debug("Unlocking device")
      system "mcetool --set-tklock-mode=unlocked"
      system "mcetool --set-inhibit-mode=stay-on"
    end        

    print_debug("restart mthome")
    system("initctl restart xsession/mthome")

    print_debug("move #{MATTI_LOCATION} to #{TEMPORARY_MATTI_LOCATION}")
    system "mv #{MATTI_LOCATION} #{TEMPORARY_MATTI_LOCATION}"

    print_debug("restart applauncherd")
    system("initctl restart xsession/applauncherd")
    sleep(10)
  end
  

        
  # method called after any test case for cleanup purposes
  def teardown
    print_debug ("exit from teardown")
    print_debug("move #{TEMPORARY_MATTI_LOCATION} to #{MATTI_LOCATION}")
    system "mv #{TEMPORARY_MATTI_LOCATION} #{MATTI_LOCATION}"

    if @options[:application] != nil     
      print_debug("restart mthome")
      system("initctl restart xsession/mthome")
      sleep(10)
    end
  end
  
  def open_Apps(appName)
    
    #Remove the Log file if it exists
    if FileTest.exists?(PIXELCHANGED_LOG)
      print_debug("remove #{PIXELCHANGED_LOG}")
      system "rm #{PIXELCHANGED_LOG}"
    end
    if FileTest.exists?(@options[:logFile])
      print_debug("remove #{@options[:logFile]}")
      system "rm #{@options[:logFile]}"
    end
    # Kill the binary if alive
    print_debug("Kill #{@options[:binary]}")
    system "pkill #{@options[:binary]}"
    sleep(2)

    if @options[:command] != nil
      # execute the optional command if available
      if @options[:pre_step] != nil 
        print_debug ("pre_step: #{@options[:pre_step]}")
        system "#{@options[:pre_step]}"
      end


      # Check the avarage system load is under 0.3
      print_debug("Check the avarage system load is under 0.3")
      system "/usr/bin/waitloadavg.rb -l 0.3 -p 1.0 -t 100 -d"

      start_command ="`#{PIXELCHANGED_BINARY} -t 20x20 -t 840x466 -q >> #{PIXELCHANGED_LOG} &`; #{FALA_GETTIME_BINARY} \"Started from command line\" >>  #{PIXELCHANGED_LOG}; #{@options[:command]} &"
      print_debug ("start command: #{start_command}")
      system "#{start_command}"
      sleep (4)
      print_debug ("pkill #{@options[:binary]}")
      system "pkill \"#{@options[:binary]}\""

    else
     # execute the optional command if available
      if @options[:pre_step] != nil 
        print_debug ("pre_step: #{@options[:pre_step]}")
        system "#{@options[:pre_step]}"
      end
        
      @pos = `#{GET_COORDINATES_SCRIPT} -a #{@options[:application]}`
      @pos = @pos.split("\n")[-1]
      print_debug ("Co-ordinates: #{@pos}")
 
      print_debug("Check the avarage system load is under 0.3")
      system "/usr/bin/waitloadavg.rb -l 0.3 -p 1.0 -t 50 -d"

      cmd = "#{PIXELCHANGED_BINARY} -c #{@pos} -t 20x20 -t 840x466 -f #{PIXELCHANGED_LOG} -q"
      print_debug("pixel changed command is : #{cmd}")
      system cmd
      sleep (4)
      # Raise meegotouchhome to the top.
      #Workaround for keeping the window stack in shape.
      print_debug("#{GET_COORDINATES_SCRIPT} -g")
      system "#{GET_COORDINATES_SCRIPT} -g"

      print_debug("pkill :#{@options[:binary]}")
      system "pkill #{@options[:binary]}"
    end

  end
  
  def read_file(appName)
    #Reading the log file to get the time
    lines = File.open(PIXELCHANGED_LOG).readlines().collect { |x| x.split(" ")[0].to_i }
    lines_app = File.open(@options[:logFile]).readlines().collect { |x| x.split(" ")[0].to_i }
    #app_from_cache value
    @app_from_cache = lines_app[1] - lines_app[0]   
    print_debug ("App from cache #{@app_from_cache}")

    @win_from_cache = lines_app[2] - lines_app[1]   
    print_debug ("Window from cache #{@win_from_cache}")

    # First line tells when the button is released
    @start_time = lines[0]
    print_debug ("Started at : #{lines[0]}")
    # Second one when the first pixel has changed its color
    @end_time = lines[1]
    print_debug ("Pixel changed: #{lines[1]}")
    return @app_from_cache, @win_from_cache
  end
  
  
  def measure_time
    #Measuring the Startup Time for applications
    app_t = @end_time - @start_time
    print_debug ("Startup time : #{app_t}")
    return app_t
  end
  
  def test_performance
    wL = []
    list = []
    wLsum = 0
    app_cache_sum = 0
    win_cache_sum = 0 
    #Run Application with invoker
    for i in 1..COUNT
      print_debug ("Now Launching  #{@options[:application]} #{i} times" )
      print_debug("Kill #{PIXELCHANGED_BINARY} if any before launching ")
      system("pkill #{PIXELCHANGED_BINARY}")
      open_Apps(@options[:application])
      sleep (5)
      list.push(read_file(@options[:application]))
      wL.push(measure_time)
    end
    
    
    print_debug ("Startup time in milliseconds\n")
    print_debug ("Application: #{@options[:application]} \n")
    if @options[:startup] != nil
      print_debug ("Time startup: #{@options[:startup]} ")
    end 
    
    #Printing the data
    for i in 0..COUNT-1
      print "%d \n" %[wL[i]]
      wLsum = wLsum + wL[i]
      app_cache_sum = app_cache_sum + list[i][0]
      win_cache_sum = win_cache_sum + list[i][1]
    end
    print_debug ("Average Startup : #{wLsum/COUNT}")
    print_debug ("MAppliacation from cache: #{app_cache_sum/COUNT}")
    print_debug ("MApplicationWindow from cache #{win_cache_sum/COUNT}")

    if @options[:startup] != nil
      print_debug("Check that startup time is less than #{@options[:startup]} ms")
      assert((wLsum/COUNT) < @options[:startup], "Application: #{@options[:application]} avarage startup was slower than #{@options[:startup]} ms")
    end

    if @options[:appCache] != nil
      print_debug("Check that MApplication from cache takes less than #{@options[:appCache]} ms")
      assert((app_cache_sum/COUNT) < @options[:appCache], "Application: #{@options[:application]} avarage app-cache was slower than #{@options[:appCache]} ms")
    end

    if @options[:winCache] != nil
      print_debug("Check that MApplicationWindow from cache takes less than #{@options[:winCache]} ms")
      assert((win_cache_sum/COUNT) < @options[:winCache], "Application: #{@options[:application]} avarage window-cache was slower than #{@options[:winCache]} ms")
    end
    
  end
end



