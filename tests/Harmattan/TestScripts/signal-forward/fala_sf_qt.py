#!/usr/bin/env python
import os
import time
import commands

#Launching application from the script
os.system("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_qt.sh &")

time.sleep(3)
st, op = commands.getstatusoutput("pgrep fala_ft_hello")

#Killing application with a signal 11 (Segmentation Fault)
commands.getoutput("kill -6 %s" %op)
time.sleep(2)

