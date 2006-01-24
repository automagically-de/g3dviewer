#!/bin/sh

INCLUDES=""
#for dir in /usr/share/aclocal \
#		   /usr/local/share/aclocal \
#		   /opt/gnome/share/aclocal
#do if test -d "$dir"; then INCLUDES="$INCLUDES -I $dir"; fi; done

echo "++ aclocal -I m4/ $ACLOCAL_FLAGS $INCLUDES"
aclocal -I m4/ $ACLOCAL_FLAGS $INCLUDES

echo "++ autoheader"
autoheader

echo "++ automake --gnu --add-missing --copy"
automake --gnu --add-missing --copy

echo "++ autoconf"
autoconf

if [ "$1" = "-conf" ]; then
	shift
	echo "++ ./configure --enable-debug $@"
	./configure --enable-debug "$@"
fi
