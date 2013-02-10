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

EXTRA_DIST =					\
	CMakeLists.txt

AM_CFLAGS =					\
	$(GROONGA_CFLAGS)

AM_LDFLAGS =					\
	-avoid-version				\
	-module					\
	-no-undefined

LIBS =						\
	$(GROONGA_LIBS)

normalizers_plugins_LTLIBRARIES =
normalizers_plugins_LTLIBRARIES += mysql.la

include mysql_sources.am

update-general-ci-table:
	@if test -z "$(MYSQL_SOURCE_DIR)"; then		\
	  echo "\$$(MYSQL_SOURCE_DIR) is missing";	\
	  exit 1;					\
	fi
	$(top_srcdir)/tool/generate_utf8_table.rb	\
	  $(MYSQL_SOURCE_DIR)/strings/ctype-utf8.c >	\
	  $(srcdir)/mysql_general_ci_table.h
