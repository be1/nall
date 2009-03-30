#!/bin/sh
# probe for nall (see http://brouits.free.fr)
# tell the RECENT or UNSEEN email received on an IMAP mail account
# need: telnet
#
HOSTNAME=imap.example.com
USERNAME=example
PASSWORD=secret

imap () {
 echo "X login $USERNAME $PASSWORD"; sleep 1;
 echo "X select inbox"; sleep 1;
 echo "X logout"; sleep 1;
}
#escape the begining * by a shell comment because of telnet output evaluation
INBOX=`imap|telnet $HOSTNAME 143|sed 's/^\*/#/' 2>/dev/null`
RECENT=`echo $INBOX | sed 's/.*\([0-9][0-9]*\) [Rr][Ee][Cc][Ee][Nn][Tt].*/\1/'`
UNSEEN=`echo $INBOX | sed 's/.*[Uu][Nn][Ss][Ee][Ee][Nn] \([0-9][0-9]*\).*/\1/'`

#test login fail
if echo $INBOX | grep -i "LOGIN failed" >/dev/null 2>&1
then
 echo login failed
 exit 1
elif [ "X$RECENT" = "X" ]
then
 echo no server response
 exit 1
elif echo "$UNSEEN" | grep -v "^[0-9][0-9]*$"
then
 UNSEEN=0
fi
if [ $RECENT -eq 0 -a $UNSEEN -eq 0 ]
then
 echo no mail
elif [ $RECENT -gt 0 ]
then
 echo $RECENT recent mail
else
 echo $UNSEEN unseen mail
fi
