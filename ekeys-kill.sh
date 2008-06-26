#!/bin/sh

# Kill ekeys by its PID number.
if [ -f ~/.expresskeys/expresskeys.pid ]; then
  echo "Will try to kill ekeys."
  /bin/kill -15 `/bin/cat ~/.expresskeys/expresskeys.pid | /bin/head -n 1`
fi

