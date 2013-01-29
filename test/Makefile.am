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

if WITH_TEST
TESTS = run-test.sh
endif
TESTS_ENVIRONMENT =								\
	NO_MAKE=yes								\
	BUILD_DIR="$(builddir)"							\
	RUBY="$(RUBY)"								\
	GROONGA="$(GROONGA)"							\
	GROONGA_SUGGEST_CREATE_DATASET="$(GROONGA_SUGGEST_CREATE_DATASET)"

update-files:
	cd $(srcdir) && ./update-files.sh > files.am

include files.am

EXTRA_DIST =					\
	run-test.sh				\
	suite

