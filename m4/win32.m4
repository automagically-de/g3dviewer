
AC_DEFUN([AC_IS_WIN32], [
	AC_MSG_CHECKING([for Win32])
	case "$host" in
		*-mingw*)
			os_win32=yes
			;;
		*)
			os_win32=no
			;;
	esac
	AC_MSG_RESULT([$os_win32])
	if test "$os_win32" = "yes"; then
		if test x$enable_static = xyes -o x$enable_static = x; then
			AC_MSG_WARN(
				[Disabling static library build, must build as DLL on Windows.])
		fi
		if test x$enable_shared = xno; then
			AC_MSG_WARN(
				[Enabling shared library build, must build as DLL on Windows.])
		fi
		enable_static=no
		enable_shared=yes
	fi
	AM_CONDITIONAL(OS_WIN32, test "$os_win32" = yes)
])

