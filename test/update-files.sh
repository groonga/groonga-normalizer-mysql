#!/bin/sh
#
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

list_paths()
{
    variable_name=$1
    echo "$variable_name = \\"
    LC_ALL=C sort | \
    sed \
      -e 's,^,\t,' \
      -e 's,$, \\,'
    echo "\t\$(NULL)"
    echo
}

find . -type f -name '*.test' | \
    sed -e 's,\./,,' | \
    sort | \
    list_paths "test_files"

find . -type f -name '*.expected' | \
    sed -e 's,\./,,' | \
    sort | \
    list_paths "expected_files"

find . -type f -name '*.grn' | \
    sed -e 's,\./,,' | \
    sort | \
    list_paths "fixture_files"
