#!/bin/sh

# Re-read expresskeys config file.

if [ -f ~/.expresskeys/expresskeys.pid ]; then

   echo "Re-reading expresskeys config file."

   kill -USR1 `cat ~/.expresskeys/expresskeys.pid | head -n 1`

else

   echo "expresskeys doesn't seem to be running as a daemon..."

fi

