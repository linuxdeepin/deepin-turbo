#!/bin/sh
TIME=`/usr/bin/fala_gettime`
echo $TIME > /tmp/testapp.log
exec /usr/bin/fala_wol
