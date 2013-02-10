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
  puts("Usage: #{$0} MYSQL_SOURCE/strings/ctype-uca.c")
  exit(false)
end

def code_point_to_utf8(code_point)
  [code_point].pack("U")
end

def utf8_to_code_point(utf8)
  utf8.unpack("U")
end

current_page = nil
pages = {}
in_length = false
lengths = []
ARGF.each_line do |line|
  case line
  when / page(\d{3})data\[\]=/
    current_page = $1.to_i(16)
    pages[current_page] = []
  when /^\s*0x(?:[\da-z]+)(?:,\s*0x(?:[\da-z]+))*,?$/i
    next if current_page.nil?
    weights = line.chomp.split(/,\s*/).collect do |component|
      Integer(component)
    end
    pages[current_page].concat(weights)
  when / uca_length\[256\]=/
    in_length = true
  when /^\d+(?:,\d+)*,?$/
    next unless in_length
    _lengths = line.chomp.split(/,/).collect {|length| Integer(length)}
    lengths.concat(_lengths)
  when /^\};$/
    current_page = nil
    in_length = false
  end
end

pages.each do |page, flatten_weights|
  weights = flatten_weights.each_slice(lengths[page])
  pages[page] = weights.with_index.collect do |weight, i|
    if weight.all?(&:zero?)
      weight = [0]
    else
      while weight.last.zero?
        weight.pop
      end
    end
    code_point = (page << 8) + i
    {
      :weight     => weight,
      :code_point => code_point,
      :utf8       => code_point_to_utf8(code_point),
    }
  end
end

sorted_pages = pages.sort_by do |page, characters|
  page
end

weight_based_characters = {}
sorted_pages.each do |page, characters|
  characters.each do |character|
    weight = character[:weight]
    weight_based_characters[weight] ||= []
    weight_based_characters[weight] << character
  end
end

n_idencials = 0
weight_based_characters.each do |weight, characters|
  next if characters.size == 1
  n_idencials += 1
  formatted_weight = weight.collect {|component| '%#07x' % component}.join(', ')
  puts "weight: #{formatted_weight}"
  characters.each do |character|
    utf8 = character[:utf8]
    code_point = character[:code_point]
    p ["U+%04x" % code_point, utf8]
  end
end

puts "Number of idencial weights #{n_idencials}"
