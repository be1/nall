#! /bin/sh
git log > ChangeLog
intltoolize
aclocal
autoheader
autoconf
automake --add-missing
