#!/bin/sh

INCLUDES=""
#for dir in /usr/share/aclocal \
#		   /usr/local/share/aclocal \
#		   /opt/gnome/share/aclocal
#do if test -d "$dir"; then INCLUDES="$INCLUDES -I $dir"; fi; done

set -x

glib-gettextize --copy --force

aclocal -I m4/ $ACLOCAL_FLAGS $INCLUDES

autoheader

automake --gnu --add-missing --copy

autoconf

if [ "$1" = "-conf" ]; then
	shift
	echo "++ ./configure --enable-debug $@"
	./configure --enable-debug "$@"
fi
