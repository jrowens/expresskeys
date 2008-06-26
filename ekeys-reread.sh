#!/bin/sh

# Reread ekeys config file.

if [ -f ~/.expresskeys/expresskeys.pid ]; then

   echo "Rereading ekeys config file."

   /bin/kill -USR1 `/bin/cat ~/.expresskeys/expresskeys.pid | /bin/head -n 1`

else

   echo "ekeys doesn't seem to be running..."

fi

