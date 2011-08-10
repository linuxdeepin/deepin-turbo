#!/usr/bin/env python
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

import subprocess
import sys

p = subprocess.Popen("(for p in $(pgrep applauncherd); do ls -l /proc/$p/fd | grep pipe ; done) | awk -F\: '{print $3}' | sort -u",
                     shell=True,
                     stdout=subprocess.PIPE)

result= p.stdout.read()
print type(result), result

if result[0]!='[':
    sys.exit(1)


p = subprocess.Popen("(for p in $(pgrep applauncherd); do ls -l /proc/$p/fd | grep pipe ; done) | awk -F\: '{print $3}' | sort -u | wc -l",
                     shell=True,
                     stdout=subprocess.PIPE)

result= int(p.stdout.read())

if result!=1:
    sys.exit(2)

p = subprocess.Popen("(for p in $(pgrep applauncherd); do ls -l /proc/$p/fd | grep pipe ; done) | awk -F\: '{print $3}' | wc -l",
                     shell=True,
                     stdout=subprocess.PIPE)

result= int(p.stdout.read())

if result!=4:
    sys.exit(4)
