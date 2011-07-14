#!/usr/bin/env python
import os
import time
import commands

#Launching application from the script
os.system("/usr/share/applauncherd-testscripts/signal-forward/fala_sf_m.sh &")

time.sleep(2)
st, op = commands.getstatusoutput("ps ax | grep fala_ft_hello")
if op.split("\n")[1].find("invoker") != -1:
    op = op.split("\n")[1].split(" ")[0]
else:
    print("The Application was not launched by invoker")
    exit

#Killing application with a signal 11 (Segmentation Fault)
commands.getoutput("kill -11 %s" %op)
time.sleep(2)

