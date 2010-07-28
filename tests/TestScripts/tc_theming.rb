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

"""
This script tests that system theming works for launched applications.

Testing that changing the theme is done as follows:

  * launch application
  * take a screenshot (1)
  * change theme
  * take a screenshot (2)
  * change theme back to original theme
  * take a screenshot (3)
  * make sure that 1 and 2 differ and that 1 and 3 are the same

Testing that application specific theming is done using an application
which has application specific theming and application specific svg
graphics:

  * start application without launcher
  * take screenshot of themed widgets (1)
  * start application with launcher
  * take screenshot of themed widgets (2)
  * verify that (1) and (2) are equal
"""

require 'matti'
require 'date'
require 'test/unit'
include MattiVerify

SCREENSHOT_DIR = '/home/user/MyDocs'

if ENV.has_key?('_SBOX_DIR')
    ENV['PATH'] += ":" + ENV['_SBOX_DIR'] + "/tools/bin"
    SCREENSHOT_DIR = '/tmp'
end

class TC_Theming < Test::Unit::TestCase
    def change_theme(theme_name)
        puts "Changing theme from #{get_theme()} to #{theme_name}"

        cmd = "gconftool-2 -s /meegotouch/theme/name -t string " + theme_name
        
        verify_equal(true, 5, "Could not change theme") {
            system(cmd)
        }

        sleep(15)
    end

    def get_theme()
        `gconftool-2 -g /meegotouch/theme/name`.strip()
    end

    def setup
        system('mcetool --set-tklock-mode=unlocked')
        @sut = MATTI.sut(:Id => ARGV[0] || 'sut_qt_maemo') 
    end

    def test_global_theming
        app_name = 'fala_ft_hello'

        system("pkill #{app_name}")

        system(app_name)
        sleep(5)

        #pid = `pgrep -n #{app_name}`

        app = @sut.application(:name => app_name)
        widget = app.MWidget(:name => 'centralWidget')

        original_theme = get_theme()
        alternative_theme = 'plankton'

        if original_theme == alternative_theme
            alternative_theme = 'blanco'
        end

        widget.capture_screen('PNG', SCREENSHOT_DIR + '/testshot1.png', true)

        change_theme(alternative_theme)

        widget.capture_screen('PNG', SCREENSHOT_DIR + '/testshot2.png', true)

        change_theme(original_theme)

        widget.capture_screen('PNG', SCREENSHOT_DIR + '/testshot3.png', true)

        system("pkill #{app_name}")

        # diff returns an error when files are different
        verify_equal(false, 1, "Shots 1 & 2 should differ!") {
            system("diff #{SCREENSHOT_DIR}/testshot1.png #{SCREENSHOT_DIR}/testshot2.png")
        }

        # diff returns success when files are the same
        verify_equal(true, 1, "Shots 1 & 3 should be the same!") {
            system("diff #{SCREENSHOT_DIR}/testshot1.png #{SCREENSHOT_DIR}/testshot3.png")
        }

        File.delete(SCREENSHOT_DIR + "/testshot1.png",
                    SCREENSHOT_DIR + "/testshot2.png",
                    SCREENSHOT_DIR + "/testshot3.png")
    end

    def test_application_specific_theming

        app_with_launcher = 'fala_ft_themetest'
        app_without_launcher = "#{app_with_launcher}.launch"

        # start app and take screenshots of the themed widgets
        system("pkill #{app_without_launcher}")
        system("pkill #{app_with_launcher}")
        system("#{app_without_launcher} &")
        sleep(5)

        app = @sut.application(:name => app_without_launcher)
        widget = app.MLabel(:name => 'themedlabel')
        widget.capture_screen('PNG', '/tmp/testshot1.png', true)
        widget = app.MLabel(:name => 'themedlabelwithgraphics')
        widget.capture_screen('PNG', '/tmp/testshot2.png', true)
        close_button = app.MButton( :name => 'CloseButton' )
        close_button.tap

        # once again using the launcher
        system("#{app_with_launcher} &")
        sleep(5)

        app = @sut.application(:name => app_with_launcher)
        widget = app.MLabel(:name => 'themedlabel')
        widget.capture_screen('PNG', '/tmp/testshot3.png', true)
        widget = app.MLabel(:name => 'themedlabelwithgraphics')
        widget.capture_screen('PNG', '/tmp/testshot4.png', true)
        close_button = app.MButton( :name => 'CloseButton' )
        close_button.tap

        
        verify_equal(true, 1, "Application specific theming failed with launcher!") {
            # diff returns success when files are the same
            system("diff /tmp/testshot1.png /tmp/testshot3.png")
        }

        verify_equal(true, 1, "Application specific svg files not found with launcher!") {
            # diff returns success when files are the same
            system("diff /tmp/testshot2.png /tmp/testshot4.png")
        }

        File.delete("/tmp/testshot1.png", "/tmp/testshot2.png",
                    "/tmp/testshot3.png", "/tmp/testshot4.png")
    end
end
