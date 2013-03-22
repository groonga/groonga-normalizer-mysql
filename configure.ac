# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

AC_PREREQ(2.59)
m4_define([groonga_normalizer_mysql_version], m4_include(version))
AC_INIT([groonga-normalizer-mysql],
        groonga_normalizer_mysql_version,
        [groonga-talk@lists.sourceforge.net])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([normalizers/mysql.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

AC_PROG_LIBTOOL

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -Wextra"
  CFLAGS="$CFLAGS -Wmissing-declarations -Wmissing-prototypes"
fi

PKG_CHECK_MODULES([GROONGA], [groonga >= 2.1.2])

_PKG_CONFIG(GROONGA_PLUGINS_DIR, [variable=pluginsdir],    [groonga])
_PKG_CONFIG(GROONGA,             [variable=groonga],       [groonga])
_PKG_CONFIG(GROONGA_HTTPD,       [variable=groonga_httpd], [groonga])

GROONGA_PLUGINS_DIR="${pkg_cv_GROONGA_PLUGINS_DIR}"
GROONGA="${pkg_cv_GROONGA}"
GROONGA_HTTPD="${pkg_cv_GROONGA_HTTPD}"

AC_SUBST(GROONGA_PLUGINS_DIR)
AC_SUBST(GROONGA)
AC_SUBST(GROONGA_HTTPD)

normalizers_pluginsdir="\${GROONGA_PLUGINS_DIR}/normalizers"
AC_SUBST(normalizers_pluginsdir)

# check Ruby for test
ac_cv_ruby_available="no"
AC_ARG_WITH([ruby19],
            AS_HELP_STRING([--with-ruby19=PATH],
                           [Ruby 1.9 interpreter path (default: auto)]),
            [RUBY="$withval"],
            [RUBY="auto"])

if test "x$RUBY" = "xno"; then
  RUBY=
else
  if test "x$RUBY" = "xauto"; then
    AC_PATH_PROGS(RUBY, [ruby1.9 ruby19 ruby1.9.1 ruby], ruby19-not-found)
    if test "$RUBY" != "ruby19-not-found"; then
      ruby_version="`$RUBY --version`"
      if echo "$ruby_version" | grep -q -- 'ruby 1\.9'; then
        ac_cv_ruby_available="yes"
      else
        AC_MSG_WARN([$RUBY isn't Ruby 1.9 ($ruby_version)])
      fi
    fi
  else
    ruby_not_found_warning_message="$RUBY is not found. Disable HTTP test."
    case "$RUBY" in
    /*)
      AC_CHECK_FILE([$RUBY],
                    [ac_cv_ruby_available="yes"],
                    [AC_MSG_WARN([$ruby_not_found_warning_message])
                     RUBY="$RUBY-not-found"])
      ;;
    *)
      ruby_not_found="$RUBY-not-found"
      AC_PATH_PROGS(RUBY, "$RUBY", "$ruby_not_found")
      if test "$RUBY" = "$ruby_not_found"; then
        AC_MSG_WARN([$ruby_not_found_warning_message])
      else
        ac_cv_ruby_available="yes"
      fi
      ;;
    esac
  fi
fi
AC_SUBST(RUBY)
AM_CONDITIONAL([WITH_TEST], [test "$ac_cv_ruby_available" = "yes"])

# For package
AC_ARG_WITH(rsync-path,
            [AS_HELP_STRING([--with-rsync-path=PATH],
            [specify rsync path to upload groonga packages.])],
            [RSYNC_PATH="$withval"],
            [RSYNC_PATH="packages@packages.groonga.org:public"])
AC_SUBST(RSYNC_PATH)

GPG_UID=m4_include(gpg_uid)
AC_SUBST(GPG_UID)

AC_CONFIG_FILES([
  Makefile
  groonga-normalizer-mysql.pc
  normalizers/Makefile
  packages/Makefile
  packages/apt/Makefile
  packages/rpm/Makefile
  packages/rpm/centos/Makefile
  packages/rpm/centos/groonga-normalizer-mysql.spec
  packages/rpm/fedora/Makefile
  packages/rpm/fedora/groonga-normalizer-mysql.spec
  packages/yum/Makefile
  packages/source/Makefile
  test/Makefile
  doc/Makefile
  doc/text/Makefile
])

AC_OUTPUT

echo "$PACKAGE_NAME $PACKAGE_VERSION configuration:"
echo "-----------------------"
echo "  Compiler:              ${CC}"
echo "  CFLAGS:                ${CFLAGS}"
echo "  CXXFLAGS:              ${CXXFLAGS}"
echo "  Libraries:             ${LIBS}"
echo
echo "groonga"
echo "  CFLAGS:                ${GROONGA_CFLAGS}"
echo "  Libraries:             ${GROONGA_LIBS}"
echo "  install directory:     ${normalizers_pluginsdir}"
