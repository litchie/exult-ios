#!/bin/sh

DIE=0

# Check for availability
(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo "**Error**: You must have 'autoconf' installed to compile Exult."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo "**Error**: You must have 'automake' installed to compile Exult."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.4.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
}
# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo "**Error**: Missing 'aclocal'.  The version of 'automake'"
  echo "installed doesn't appear recent enough."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.4.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

aclocalincludes=""
if test -d "/usr/local/share/aclocal"; then
  if test "/usr/local/share/aclocal" != `aclocal --print-ac-dir`; then
    aclocalincludes="$aclocalincludes -I /usr/local/share/aclocal"
  fi
fi

#if test -d "$HOME/share/aclocal"; then
#  if test "$HOME/share/aclocal" != `aclocal --print-ac-dir`; then
#    aclocalincludes="$aclocalincludes -I $HOME/share/aclocal"
#  fi
#fi

# Clean up the generated crud
rm -f configure config.log config.guess config.sub config.cache
rm -f libtool ltmain.sh missing mkinstalldirs install-sh
rm -f autoconfig.h.in
rm -f config.status aclocal.m4
rm -f `find . -name 'Makefile.in'`
rm -f `find . -name 'Makefile'`

# touch the configure.in file to force rebuilding configure
touch configure.in

# Regenerate everything
aclocal $aclocalincludes
libtoolize --force --copy
autoheader
automake --add-missing --copy --gnu
autoconf 

echo "You are now ready to run ./configure"
