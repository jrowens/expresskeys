#!/bin/sh

# Terminate expresskeys by its PID number.

if [ -f ~/.expresskeys/expresskeys.pid ]; then

   echo "Will try to terminate expresskeys."

   /bin/kill -TERM `/bin/cat ~/.expresskeys/expresskeys.pid | /bin/head -n 1`

   /bin/sleep 1

   if ! [ -f ~/.expresskeys/expresskeys.pid ]; then
      echo "Success!"
   else
      echo "Failure!"
   fi

else

   echo "expresskeys doesn't seem to be running..."

fi

