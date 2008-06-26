#!/bin/sh

# Re-read expresskeys config file.

if [ -f ~/.expresskeys/expresskeys.pid ]; then

   echo "Re-reading expresskeys config file."

   /bin/kill -USR1 `/bin/cat ~/.expresskeys/expresskeys.pid | /bin/head -n 1`

else

   echo "expresskeys doesn't seem to be running..."

fi

