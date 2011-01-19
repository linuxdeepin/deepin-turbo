#!/usr/bin/env python
import os
import time
import commands

#Launching application from the script
os.system("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_d.sh &")

time.sleep(3)
st, op = commands.getstatusoutput("pgrep fala_qml_helloworld")

#Killing application with a signal 15 (Terminated)
commands.getoutput("kill -15 %s" %op)
time.sleep(2)

