#!/usr/bin/env ruby
#
# Copyright (C) 2018  Kouhei Sutou <kou@clear-code.com>
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

require "optparse"

$LOAD_PATH.unshift(File.dirname(__FILE__))
require "parser"

@weight_level = 1
@suffix = nil

option_parser = OptionParser.new
option_parser.banner += " MYSQL_SOURCE/strings/uca900_data.h"

option_parser.on("--weight-level=N", Integer,
                 "Use N level weights",
                 "(#{@weight_level})") do |level|
  @weight_level = level
end
option_parser.on("--tailoring-locale=LOCALE",
                 "Use LOCALE tailoring",
                 "(#{@tailoring_locale})") do |locale|
  @tailoring_locale = locale
end
option_parser.on("--tailoring-path=PATH",
                 "Parse PATH to extract tailoring expression",
                 "(#{@tailoring_path})") do |path|
  @tailoring_path = path
end
option_parser.on("--suffix=SUFFIX", "Add SUFFIX to names") do |suffix|
  @suffix = suffix
end

begin
  option_parser.parse!(ARGV)
rescue OptionParser::ParseError
  $stderr.puts($!)
  exit(false)
end

if ARGV.size != 1
  puts(option_parser)
  exit(false)
end

uca_h_path = ARGV[0]

parser = UCA900Parser.new
if @tailoring_path
  File.open(@tailoring_path) do |tailoring_file|
    parser.parse_tailoring(tailoring_file, @tailoring_locale)
  end
end
File.open(uca_h_path) do |uca_h|
  parser.parse(uca_h)
end

normalization_table = parser.normalization_table(level: @weight_level)

normalized_uca_h_path =
  uca_h_path.sub(/\A.*\/([^\/]+\/strings\/uca900_data\.h)\z/, "\\1")

puts(<<-HEADER)
/*
  Copyright(C) 2018  Kouhei Sutou <kou@clear-code.com>

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
  #{normalized_uca_h_path}.
  The following is the header of the file:

    Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2.0,
    as published by the Free Software Foundation.

    This program is also distributed with certain software (including
    but not limited to OpenSSL) that is licensed under separate terms,
    as designated in a particular file or component or in included license
    documentation.  The authors of MySQL hereby grant you an additional
    permission to link the program and your derivative works with the
    separately licensed software that they have included with MySQL.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License, version 2.0, for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

    This header file contains weight tables and weight length table of
    UCA8.0.0, as well as some constant values and table of decomposition.
*/

#pragma once

#include <stdint.h>
HEADER

def variable_name_prefix
  prefix = "unicode_900"
  prefix << @suffix if @suffix
  prefix
end

def page_name(page)
  "#{variable_name_prefix}_page_%02x" % page
end

normalization_table.each do |page, characters|
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

static uint32_t *#{variable_name_prefix}_table[] = {
PAGES_HEADER

pages = []
normalization_table.each do |page, characters|
  pages[page] = page_name(page)
end
lines = pages.each_slice(2).collect do |pages_group|
  formatted_pages = pages_group.collect do |page|
    "%19s" % (page || "NULL")
  end
  "  " + formatted_pages.join(", ")
end
puts(lines.join(",\n"))

puts(<<-PAGES_FOOTER)
};
PAGES_FOOTER
