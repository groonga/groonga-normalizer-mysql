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

require "English"

if ARGV.size != 1
  puts("Usage: #{$0} MYSQL_SOURCE/strings/ctype-utf8.c")
  exit(false)
end

def code_point_to_utf8(code_point)
  [code_point].pack("U")
end

def utf8_to_code_point(utf8)
  utf8.unpack("U")
end

current_plane = nil
planes = {}
ARGF.each_line do |line|
  case line
  when / plane(\d{2})\[\]=/
    current_plane = $1.to_i(16)
    planes[current_plane] = []
  when /^\s*
         \{0x([\da-z]+),0x([\da-z]+),0x([\da-z]+)\},
         \s*
         \{0x([\da-z]+),0x([\da-z]+),0x([\da-z]+)\},?$/ix
    next if current_plane.nil?
    parsed_characters = $LAST_MATCH_INFO.captures.collect do |value|
      code_point_to_utf8(value.to_i(16))
    end
    upper1, lower1, sort1, upper2, lower2, sort2 = parsed_characters
    characters = planes[current_plane]
    characters << {:upper => upper1, :lower => lower1, :sort => sort1}
    characters << {:upper => upper2, :lower => lower2, :sort => sort2}
  when /^\};$/
    current_plane = nil
  end
end

planes.each do |plane, characters|
  characters.each_with_index do |character, i|
    character[:base] = code_point_to_utf8((plane << 8) + i)
  end
end

sorted_planes = planes.sort_by do |plane, characters|
  plane
end

n_differences = 0
n_expanded_sort_characters = 0
sorted_planes.each do |plane, characters|
  characters.each do |character|
    base = character[:base]
    upper = character[:upper]
    lower = character[:lower]
    sort = character[:sort]
    next if base == sort
    n_differences += 1
    utf8s = [base, upper, lower, sort]
    formatted_code_points = utf8s.collect do |utf8|
      "%#07x" % utf8_to_code_point(utf8)
    end
    if sort.bytesize > base.bytesize
      n_expanded_sort_characters += 1
    end
    p [utf8s, formatted_code_points]
  end
end

puts "Number of differences: #{n_differences}"
puts "Number of expanded sort characters: #{n_expanded_sort_characters}"
