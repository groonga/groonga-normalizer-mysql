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

require "English"

module Unicode
  module_function
  def to_utf8(code_point)
    [code_point].pack("U")
  end

  def from_utf8(utf8)
    utf8.unpack("U")[0]
  end
end

class Character < Struct.new(:weights,
                             :code_point)
  def utf8
    Unicode.to_utf8(code_point)
  end
end

module CharacterArray
  def find_representative_character(options={})
    representative_character = nil
    case first.utf8
    when "⺄", "⺇", "⺈", "⺊", "⺌", "⺗"
      representative_character = last
    when "⺜", "⺝", "⺧", "⺫", "⺬", "⺮", "⺶", "⺻", "⺼", "⺽"
      representative_character = self[1]
    when "⻆", "⻊", "⻏", "⻑", "⻕", "⻗", "⻝", "⻡", "⻣", "⻤"
      representative_character = last
    when "⻱", "⼀", "⼆", "⼈"
      representative_character = self[1]
    when "ぁ", "ぃ", "ぅ", "ぇ", "ぉ", "っ", "ゃ", "ゅ", "ょ", "ゎ"
      representative_character = self[1] unless options[:split_small_kana]
    else
      representative_character ||= find_greek_capital_character
    end
    representative_character ||= first
    representative_character
  end

  GREEK_CAPITAL_UNICODE_RANGE = Unicode.from_utf8("Α")..Unicode.from_utf8("Ω")
  def find_greek_capital_character
    find do |character|
      GREEK_CAPITAL_UNICODE_RANGE.cover?(character.code_point)
    end
  end
end

class CTypeUTF8Parser
  def initialize
    @pages = {}
  end

  def parse(input)
    parse_ctype_utf8(input)
    normalize_pages
  end

  def sorted_pages
    @pages.sort_by do |page, characters|
      page
    end
  end

  private
  def parse_ctype_utf8(input)
    current_page = nil
    input.each_line do |line|
      case line
      when / plane([\da-fA-F]{2})\[\]=/
        current_page = $1.to_i(16)
        @pages[current_page] = []
      when /^\s*
             \{0x([\da-z]+),0x([\da-z]+),0x([\da-z]+)\},
             \s*
             \{0x([\da-z]+),0x([\da-z]+),0x([\da-z]+)\},?$/ix
        next if current_page.nil?
        parsed_characters = $LAST_MATCH_INFO.captures.collect do |value|
          Unicode.to_utf8(value.to_i(16))
        end
        upper1, lower1, sort1, upper2, lower2, sort2 = parsed_characters
        characters = @pages[current_page]
        characters << {:upper => upper1, :lower => lower1, :sort => sort1}
        characters << {:upper => upper2, :lower => lower2, :sort => sort2}
      when /^\};$/
        current_page = nil
      end
    end
  end

  def normalize_pages
    @pages.each do |page, characters|
      characters.each_with_index do |character, i|
        character[:base] = Unicode.to_utf8((page << 8) + i)
      end
    end
  end
end

class UCAParser
  def initialize
    @pages = {}
  end

  def weight_based_characters(level)
    weight_based_characters = {}
    sorted_pages.each do |page, characters|
      characters.each do |character|
        weights = character.weights
        target_weights = weights.collect do |weight|
          weight[0, level]
        end
        weight_based_characters[target_weights] ||= []
        weight_based_characters[target_weights] << character
      end
    end
    weight_based_characters
  end

  def sorted_pages
    @pages.sort_by do |page, characters|
      page
    end
  end
end

class CTypeUCAParser < UCAParser
  def initialize(version=nil)
    super()
    @version = version
    @lengths = []
  end

  def parse(input)
    parse_ctype_uca(input)
    normalize_pages
  end

  def weight_based_characters
    super(1)
  end

  private
  def page_data_pattern
    if @version == "520"
      / uca520_p([\da-fA-F]{3})\[\]=/
    else
      / page([\da-fA-F]{3})data\[\]=/
    end
  end

  def length_pattern
    if @version == "520"
      / uca520_length\[4352\]=/
    else
      / uca_length\[256\]=/
    end
  end

  def parse_ctype_uca(input)
    current_page = nil
    in_length = false
    input.each_line do |line|
      case line
      when page_data_pattern
        current_page = $1.to_i(16)
        @pages[current_page] = []
      when /^\s*(0x(?:[\da-z]+)(?:,\s*0x(?:[\da-z]+))*),?(?: \/\*.+\*\/)?$/i
        weight_values = $1
        next if current_page.nil?
        weights = weight_values.split(/,\s*/).collect do |component|
          Integer(component)
        end
        @pages[current_page].concat(weights)
      when length_pattern
        in_length = true
      when /^\d+(?:,\d+)*,?$/
        next unless in_length
        current_lengths = line.chomp.split(/,/).collect do |length|
          Integer(length)
        end
        @lengths.concat(current_lengths)
      when /^\};$/
        current_page = nil
        in_length = false
      end
    end
  end

  def normalize_pages
    @pages.each do |page, flatten_weights|
      weights_set = flatten_weights.each_slice(@lengths[page])
      @pages[page] = weights_set.with_index.collect do |weights, i|
        weights = weights.collect do |level1_weight|
          [level1_weight]
        end
        while weights.last == [0]
          weights.pop
        end
        code_point = (page << 8) + i
        Character.new(weights, code_point)
      end
    end
  end
end
