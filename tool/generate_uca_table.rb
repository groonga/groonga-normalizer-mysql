#!/usr/bin/env ruby
#
# Copyright (C) 2013-2018  Kouhei Sutou <kou@clear-code.com>
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

@version = nil
@suffix = ""
@options = {
  split_small_kana: false,
  split_kana_with_voiced_sound_mark: false,
  split_kana_with_semi_voiced_sound_mark: false,
}

option_parser = OptionParser.new
option_parser.banner += " MYSQL_SOURCE/strings/ctype-uca.c"

option_parser.on("--version=VERSION", "Use VERSION as UCA version") do |version|
  @version = version
end

option_parser.on("--suffix=SUFFIX", "Add SUFFIX to names") do |suffix|
  @suffix = suffix
end

option_parser.on("--[no-]split-small-kana",
                 "Split small hiragana (katakana) and " +
                   "large hiragana (katakana)",
                 "(#{@options[:split_small_kana]})") do |boolean|
  @options[:split_small_kana] = boolean
end

option_parser.on("--[no-]split-kana-with-voiced-sound-mark",
                 "Split hiragana (katakana) with voiced sound mark",
                 "(#{@options[:split_kana_with_voiced_sound_mark]})") do |boolean|
  @options[:split_kana_with_voiced_sound_mark] = boolean
end

option_parser.on("--[no-]split-kana-with-semi-voiced-sound-mark",
                 "Split hiragana (katakana) with semi-voiced sound mark",
                 "(#{@options[:split_kana_with_semi_voiced_sound_mark]})") do |boolean|
  @options[:split_kana_with_semi_voiced_sound_mark] = boolean
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

ctype_uca_c_path = ARGV[0]

parser = CTypeUCAParser.new(@version)
File.open(ctype_uca_c_path) do |ctype_uca_c|
  parser.parse(ctype_uca_c)
end

normalization_table = parser.normalization_table(@options)

normalized_ctype_uca_c_path =
  ctype_uca_c_path.sub(/\A.*\/([^\/]+\/strings\/ctype-uca\.c)\z/, "\\1")

puts(<<-HEADER)
/*
  Copyright(C) 2013-2015  Kouhei Sutou <kou@clear-code.com>

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

    Copyright (c) 2004, 2014, Oracle and/or its affiliates. All rights reserved.

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

#pragma once

#include <stdint.h>
HEADER

def variable_name_prefix
  prefix = "unicode"
  if @version
    prefix << "_#{@version}"
  end
  prefix << "_ci#{@suffix}"
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
