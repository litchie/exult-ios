#!/bin/sh
# First of all clean up the generated crud
#rm -f configure config.log config.guess config.sub config.cache
#rm -f libtool ltmain.sh missing mkinstalldirs install-sh
#rm -f autoconfig.h.in
#rm -f config.status aclocal.m4
#rm -f `find . -name 'Makefile.in'`
#rm -f `find . -name 'Makefile'`

# touch the configure.in file to force rebuilding configure
touch configure.in

# Regenerate everything
aclocal -I /usr/local/share/aclocal
libtoolize --force --copy
autoheader
automake --add-missing --gnu
autoconf 

echo "You are now ready to run ./configure"
