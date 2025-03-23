#!/usr/bin/env ruby
#
# Copyright (C) 2025  Sutou Kouhei <kou@clear-code.com>
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

require_relative "parser"

options = {
  use_secondary_level: false,
  use_tertiary_level: false,
  debug: false,
}
@suffix = nil

option_parser = OptionParser.new
option_parser.banner += " MARIADB_BUILD/strings/ctype-uca1400data.h"

option_parser.on("--[no-]use-secondary-level",
                 "Whether use the secondary level or not",
                 "(#{options[:use_secondary_level]})") do |bool|
  options[:use_secondary_level] = bool
end
option_parser.on("--[no-]use-tertiary-level",
                 "Whether use the tertiary level or not",
                 "(#{options[:use_tertiary_level]})") do |bool|
  options[:use_tertiary_level] = bool
end
option_parser.on("--suffix=SUFFIX", "Add SUFFIX to names") do |suffix|
  @suffix = suffix
end
option_parser.on("--[no-]debug",
                 "Enable debug output") do |boolean|
  options[:debug] = boolean
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

parser = UCA1400Parser.new(options)
File.open(uca_h_path) do |uca_h|
  parser.parse(uca_h)
end

normalization_table = parser.normalization_table

normalized_uca_h_path =
  uca_h_path.sub(/\A.*\/([^\/]+\/strings\/ctype-uca1400data\.h)\z/, "\\1")

puts(<<-HEADER)
/*
  Copyright(C) 2025  Sutou Kouhei <kou@clear-code.com>

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
*/

#pragma once

#include <stdint.h>
HEADER

def variable_name_prefix
  prefix = "unicode_1400"
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
