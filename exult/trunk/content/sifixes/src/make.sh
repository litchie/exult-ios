#!/bin/bash

expack&>/dev/null
if [[ $? -eq 127 ]] ; then
	echo "expack was not found; please add it to your PATH environment variable"
	echo "and run this script again."
	exit 127
fi

ucc -o usecode $0&>/dev/null
if [[ $? -eq 127 ]] ; then
	echo "ucc was not found; please add it to your PATH environment variable"
	echo "and run this script again."
	exit 127
fi

for f in faces gumps shapes paperdol sprites fonts mainshp
do
	if [[ -e "graphics/$f.in" ]] ; then
		echo "Generating $f..."
		expack -i graphics/$f.in
		rm ../patch/$f\_*.h
	fi
done

echo "Compiling Usecode..."
ucc -o ../patch/usecode usecode.uc
if [[ $? -eq 0 ]] ; then
	echo "Usecode has been successfuly compiled!"
else
	echo "There were error(s) compiling usecode!"
fi
