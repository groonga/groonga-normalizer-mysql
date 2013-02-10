#!/usr/bin/env ruby
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

$LOAD_PATH.unshift(File.dirname(__FILE__))
require "parser"

if ARGV.size != 1
  puts("Usage: #{$0} MYSQL_SOURCE/strings/ctype-utf8.c")
  exit(false)
end

ctype_utf8_c_path = ARGV[0]

parser = CTypeUTF8Parser.new
File.open(ctype_utf8_c_path) do |ctype_utf8_c|
  parser.parse(ctype_utf8_c)
end

target_planes = {}
parser.sorted_planes.each do |plane, characters|
  characters.each do |character|
    base = character[:base]
    upper = character[:upper]
    lower = character[:lower]
    sort = character[:sort]
    next if base == sort
    target_planes[plane] ||= [nil] * 256
    low_code = Unicode.from_utf8(base) & 0xff
    target_planes[plane][low_code] = Unicode.from_utf8(sort)
  end
end

normalized_ctype_utf8_c_path =
  ctype_utf8_c_path.sub(/\A.*\/([^\/]+\/strings\/ctype-utf8\.c)\z/, "\\1")
puts(<<-HEADER)
/*
  Copyright(C) 2013  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; version 2
  of the License.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301, USA

  This file uses normalization table defined in
  #{normalized_ctype_utf8_c_path}.
  The following is the header of the file:

    Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
    MA 02110-1301, USA

    UTF8 according RFC 2279
    Written by Alexander Barkov <bar@udm.net>
*/

#ifndef MYSQL_UTF8_H
#define MYSQL_UTF8_H

#include <stdint.h>
HEADER

def plane_name(plane)
  "general_ci_plane_%02x" % plane
end

target_planes.each do |plane, characters|
  puts(<<-PLANE_HEADER)

static uint32_t #{plane_name(plane)}[] = {
PLANE_HEADER
  lines = characters.each_with_index.each_slice(8).collect do |characters_group|
    formatted_code_points = characters_group.collect do |normalized, low_code|
      normalized ||= (plane << 8) + low_code
      "0x%05x" % normalized
    end
    "  " + formatted_code_points.join(", ")
  end
  puts(lines.join(",\n"))
  puts(<<-PLANE_FOOTER)
};
PLANE_FOOTER
end

puts(<<-PLANES_HEADER)

static uint32_t *general_ci_table[256] = {
PLANES_HEADER

planes = ["NULL"] * 256
target_planes.each do |plane, characters|
  planes[plane] = plane_name(plane)
end
lines = planes.each_slice(2).collect do |planes_group|
  formatted_planes = planes_group.collect do |plane|
    "%19s" % plane
  end
  "  " + formatted_planes.join(", ")
end
puts(lines.join(",\n"))

puts(<<-PLANES_FOOTER)
};
PLANES_FOOTER

puts(<<-FOOTER)

#endif
FOOTER
