#!/bin/sh
# probe for nall (see http://brouits.free.fr)
# tell the RECENT email received on an IMAP mail account
# need: telnet
#
HOSTNAME=example.com
USERNAME=example
PASSWORD=example
{ echo X login $USERNAME $PASSWORD; sleep 1; echo X select inbox; sleep 1; echo X logout;} | telnet $HOSTNAME 143 2>/dev/null | grep RECENT | tail -1 | sed 's/.* \([0-9][0-9]*\) .*/\1 recent mail/'
