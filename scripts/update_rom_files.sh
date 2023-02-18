#!/bin/bash -e

if [ -z $2 ]; then
	echo "Usage: $(basename $0) <file list> <rom directory> [<rom directory>] ..."
	exit 1
fi

filelist="$1"

TMPFILE=$(mktemp build/hashes.XXXXXX)
if [[ ! -e $TMPFILE ]]; then
	echo "Can't create tempfile!"
	exit 1
fi

find ${@:2} -type f \( -iname \*.gb -o -iname \*.gbc -o -iname \*.nes -o -iname \*.fds -o -iname \*.nsf -o -iname \*.ggcodes -o -iname \*.gw -o -iname \*.sms -o -iname \*.gg -o -iname \*.sg -o -iname \*.md -o -iname \*.gen -o -iname \*.bin -o -iname \*.col -o -iname \*.pce -o -iname \*.pceplus -o -iname \*.rom -o -iname \*.dsk -o -iname \*.mx1 -o -iname \*.mx2 -o -iname \*.mcf -o -iname \*.bin -o -iname \*.a78 -o -iname \*.png -o -iname \*.jpg -o -iname \*.bmp \) | sort > "${TMPFILE}" 2> /dev/null
#find ${@:2} -type f | sort > "${TMPFILE}" 2> /dev/null

if ! diff -q ${TMPFILE} ${filelist} > /dev/null 2> /dev/null; then
    echo "Updating file list ${filelist}"
	cp -f "${TMPFILE}" "${filelist}"
fi

rm -f "${TMPFILE}"
