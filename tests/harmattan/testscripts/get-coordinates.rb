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
require 'optparse'

include TDriverVerify

WINDOWID_BINARY = '/usr/bin/fala_wid'
PIXELCHANGED_BINARY= '/usr/bin/fala_pixelchanged' 
MATTI_LOCATION='/usr/lib/qt4/plugins/testability/libtestability.so'
TEMPORARY_MATTI_LOCATION='/root/libtestability.so'

options = {}
 
optparse = OptionParser.new do|opts|
   # Set a banner, displayed at the top
   # of the help screen.
   opts.banner = "Usage: get-coordinates.rb [options] "
 
   options[:application] = nil
   opts.on( '-a', '--application APP', 'Application name in application grid' ) do|app|
     options[:application] = app
   end

   options[:grid] = false
   opts.on( '-g', '--grid_only', 'Only make application grid visible, not find any coordinates ' ) do
     options[:grid] = true
   end

   # This displays the help screen, all programs are
   # assumed to have this option.
   opts.on( '-h', '--help', 'Display this screen' ) do
     puts opts
     exit 0
   end

end
 
optparse.parse!
 

if options[:application] == nil && !(options[:grid])
  puts "Application name not defined!" 
  exit 1
end

pos = 0
appName=options[:application]
sut = TDriver.sut(:Id=> 'sut_qt_maemo')    

# Check if TDriver can attach to meegotouch instance.
# If not, this might be caused because MtHome is started
# before TDriver server or the Matti library is temporary
# renamed.

@meegoHome = nil
begin
  @meegoHome = sut.application(:name => 'meegotouchhome') 
  puts "Meegotouchhome found"
  
rescue MobyBase::TestObjectNotFoundError
  puts "Meegotouchhome not found"
  matti_removed = false
  if File.exists?(TEMPORARY_MATTI_LOCATION) && !(File.exists?(MATTI_LOCATION))
      puts "matti removed. putting back.."
    matti_removed = true
    system "mv #{TEMPORARY_MATTI_LOCATION} #{MATTI_LOCATION}"
  end
  puts "restarting meegotouchhome"
  system("initctl restart xsession/mthome")
  sleep(4)
  if matti_removed
    puts "removing matti again.."
    system "mv #{MATTI_LOCATION} #{TEMPORARY_MATTI_LOCATION}"
  end
end

# Bring the meegotouchhome to the top of the screen
meegotouchpid=`pgrep meegotouchhome`  
if !(result=$?.success?)
  #raise error and exit
  puts "meegotouchpid: #{meegotouchpid}"
  raise "Meegotouch home pid not found"
  exit 1
end 
meegotouchpid = meegotouchpid.split("\n")[-1]
puts "meegotouchpid: #{meegotouchpid}"

windowid=`#{WINDOWID_BINARY} #{meegotouchpid}`    
if !(result=$?.success?)
  #raise error and exit
  puts "windowid: #{windowid}"
  raise "Meegotouch home window id not found"
  exit 1
end
windowid = windowid.split("\n")[-1]
puts "windowid: #{windowid}"

system "#{PIXELCHANGED_BINARY} -r #{windowid}"
sleep (2)
system "mcetool --blank-screen > /dev/null"
sleep (2)
system "mcetool --set-tklock-mode=unlocked > /dev/null"
system "mcetool --unblank-screen > /dev/null"

# Application grid should be now visible
if options[:grid]
  exit 0;
end

@meegoHome = sut.application(:name => 'meegotouchhome')


sleep(2)
if @meegoHome.test_object_exists?(:text => appName)
  icon = @meegoHome.SwipeLauncherButton(:text => appName)

  while icon.attribute('visibleOnScreen') == 'false' || icon.attribute('y').to_i > 400
    @meegoHome.SwipePannableViewport( :name => 'SwipePage' ).MWidget( :name => 'glass' ).gesture(:Up, 1, 300)
    sleep(0.2)
    icon.refresh
  end
  xpos = @meegoHome.SwipeLauncherButton(:text => appName).attribute('x')
  xpos = xpos.to_i + 59
  ypos = @meegoHome.SwipeLauncherButton(:text => appName).attribute('y')
  ypos = ypos.to_i + 58
  @pos = "#{xpos}x#{ypos}"
  
  puts @pos
  exit 0
else
  #icon does not
  #raise error and exit
  raise "Application not found in Application grid"
  exit 1
end
  

