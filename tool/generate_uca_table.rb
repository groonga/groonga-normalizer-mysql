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
  puts("Usage: #{$0} MYSQL_SOURCE/strings/ctype-uca.c")
  exit(false)
end

ctype_uca_c_path = ARGV[0]

parser = CTypeUCAParser.new
File.open(ctype_uca_c_path) do |ctype_uca_c|
  parser.parse(ctype_uca_c)
end

target_pages = {}
parser.weight_based_characters.each do |weight, characters|
  next if characters.size == 1
  representative_character = characters.first
  representative_code_point = representative_character[:code_point]
  rest_characters = characters[1..-1]
  rest_characters.each do |character|
    code_point = character[:code_point]
    page = code_point >> 8
    low_code = code_point & 0xff
    target_pages[page] ||= [nil] * 256
    target_pages[page][low_code] = representative_code_point
  end
end

sorted_target_pages = target_pages.sort_by do |page, code_points|
  page
end

normalized_ctype_uca_c_path =
  ctype_uca_c_path.sub(/\A.*\/([^\/]+\/strings\/ctype-uca\.c)\z/, "\\1")
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
  #{normalized_ctype_uca_c_path}.
  The following is the header of the file:

    Copyright (c) 2004, 2011, Oracle and/or its affiliates. All rights reserved.

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

    UCA (Unicode Collation Algorithm) support.
    Written by Alexander Barkov <bar@mysql.com>
*/

#ifndef MYSQL_UCA_H
#define MYSQL_UCA_H

#include <stdint.h>
HEADER

def page_name(page)
  "unicode_ci_page_%02x" % page
end

sorted_target_pages.each do |page, characters|
  puts(<<-PAGE_HEADER)

static uint32_t #{page_name(page)}[] = {
PAGE_HEADER
  lines = characters.each_with_index.each_slice(8).collect do |characters_group|
    formatted_code_points = characters_group.collect do |normalized, low_code|
      normalized ||= (page << 8) + low_code
      "0x%05x" % normalized
    end
    "  " + formatted_code_points.join(", ")
  end
  puts(lines.join(",\n"))
  puts(<<-PAGE_FOOTER)
};
PAGE_FOOTER
end

puts(<<-PAGES_HEADER)

static uint32_t *unicode_ci_table[256] = {
PAGES_HEADER

pages = ["NULL"] * 256
sorted_target_pages.each do |page, characters|
  pages[page] = page_name(page)
end
lines = pages.each_slice(2).collect do |pages_group|
  formatted_pages = pages_group.collect do |page|
    "%19s" % page
  end
  "  " + formatted_pages.join(", ")
end
puts(lines.join(",\n"))

puts(<<-PAGES_FOOTER)
};
PAGES_FOOTER

puts(<<-FOOTER)

#endif
FOOTER
