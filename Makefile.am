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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =					\
	build					\
	normalizers				\
	test					\
	doc

docdir = $(datadir)/doc/$(PACKAGE)
dist_doc_DATA =					\
	README.md

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = groonga-normalizer-mysql.pc

EXTRA_DIST =					\
	CMakeLists.txt				\
	config.h.cmake

update-tables:
	cd normalizers && $(MAKE) update-tables

echo-groonga:
	@echo $(GROONGA)

echo-groonga-httpd:
	@echo $(GROONGA_HTTPD)

echo-ruby:
	@echo $(RUBY)

tag:
	cd $(top_srcdir) && \
	  git tag v$(VERSION) -a -m 'groonga-normalizer-mysql $(VERSION)!!!' && \
	  git push --tags

update-version:
	@if test -z "$(NEW_VERSION)"; then	\
	  echo "\$$(NEW_VERSION) is missing";	\
	  exit 1;				\
	fi
	@echo -n $(NEW_VERSION) > $(srcdir)/version

update-latest-release:
	@if test -z "$(NEW_RELEASE_DATE)"; then		\
	  echo "\$$(NEW_RELEASE_DATE) is missing";	\
	  exit 1;					\
	fi
	cd $(top_srcdir)/packages &&		\
	  $(RUBY) -S rake version:update	\
	    RELEASE_TIME=$(NEW_RELEASE_DATE)
