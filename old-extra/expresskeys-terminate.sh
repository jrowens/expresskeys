#!/bin/sh

# Terminate expresskeys by its PID number.

if [ -f ~/.expresskeys/expresskeys.pid ]; then

   echo "Will try to terminate expresskeys."

   kill -TERM `cat ~/.expresskeys/expresskeys.pid | head -n 1`

   sleep 1

   if ! [ -f ~/.expresskeys/expresskeys.pid ]; then
      echo "Success!"
   else
      echo "Failure!"
   fi

else

   echo "expresskeys doesn't seem to be running as a daemon..."

fi

