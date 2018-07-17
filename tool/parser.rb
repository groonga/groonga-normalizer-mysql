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

  def normalization_table(options={})
    table = {}
    group_characters(options).each do |characters|
      characters.extend(CharacterArray)
      next if characters.size == 1
      representative_character =
        characters.find_representative_character(options)
      representative_code_point = representative_character.code_point
      rest_characters = characters.reject do |character|
        character == representative_character
      end
      rest_characters.each do |character|
        code_point = character.code_point
        page = code_point >> 8
        low_code = code_point & 0xff
        table[page] ||= [nil] * 256
        table[page][low_code] = representative_code_point
      end
    end
    table.sort_by do |page, code_points|
      page
    end
  end

  private
  def weight_based_characters(level)
    sorted_pages = @pages.sort_by do |page, characters|
      page
    end
    weight_based_characters = {}
    sorted_pages.each do |page, characters|
      characters.each do |character|
        weights = character.weights
        target_weights = weights.collect do |weight|
          weight[0, level]
        end
        while target_weights.last and target_weights.last.all?(&:zero?)
          target_weights.pop
        end
        weight_based_characters[target_weights] ||= []
        weight_based_characters[target_weights] << character
      end
    end
    weight_based_characters
  end

  def group_characters(options={})
    grouped_characters = []
    level = options[:level] || 1
    weight_based_characters(level).each do |weight, characters|
      grouped_characters.concat(split_characters(characters, options))
    end
    grouped_characters
  end

  SMALL_KANAS = [
    "ぁ", "ぃ", "ぅ", "ぇ", "ぉ",
    "っ",
    "ゃ", "ゅ", "ょ",
    "ゎ",
    "ァ", "ィ", "ゥ", "ェ", "ォ",
    "ッ",
    "ャ", "ュ", "ョ",
    "ヮ",
    "ｧ", "ｨ", "ｩ", "ｪ", "ｫ",
    "ｯ",
    "ｬ", "ｭ", "ｮ",
  ]
  def small_kana?(character)
    SMALL_KANAS.include?(character[:utf8])
  end

  KANA_WITH_VOICED_SOUND_MARKS = [
    "が", "ぎ", "ぐ", "げ", "ご",
    "ざ", "じ", "ず", "ぜ", "ぞ",
    "だ", "ぢ", "づ", "で", "ど",
    "ば", "び", "ぶ", "べ", "ぼ",
    "ガ", "ギ", "グ", "ゲ", "ゴ",
    "ザ", "ジ", "ズ", "ゼ", "ゾ",
    "ダ", "ヂ", "ヅ", "デ", "ド",
    "バ", "ビ", "ブ", "ベ", "ボ",
  ]
  def kana_with_voiced_sound_mark?(character)
    KANA_WITH_VOICED_SOUND_MARKS.include?(character[:utf8])
  end

  KANA_WITH_SEMI_VOICED_SOUND_MARKS = [
    "ぱ", "ぴ", "ぷ", "ぺ", "ぽ",
    "パ", "ピ", "プ", "ペ", "ポ",
  ]
  def kana_with_semi_voiced_sound_mark?(character)
    KANA_WITH_SEMI_VOICED_SOUND_MARKS.include?(character[:utf8])
  end

  def split_characters(characters, options)
    grouped_characters = characters.group_by do |character|
      if options[:split_small_kana] and small_kana?(character)
        :small_kana
      elsif options[:split_kana_with_voiced_sound_mark] and
          kana_with_voiced_sound_mark?(character)
        :kana_with_voiced_sound_mark
      elsif options[:split_kana_with_semi_voiced_sound_mark] and
          kana_with_semi_voiced_sound_mark?(character)
        :kana_with_semi_voiced_sound_mark
      else
        :other
      end
    end
    grouped_characters.values
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
        code_point = (page << 8) + i
        Character.new(weights, code_point)
      end
    end
  end
end

class UCA900Parser < UCAParser
  def parse(input)
    parse_data(input)
    normalize_pages
  end

  private
  def parse_data(input)
    current_page = nil
    in_n_collation_elements = false
    nth_character = nil
    nth_weight = nil
    nth_collation_element = nil
    input.each_line do |line|
      case line.chomp
      when ""
        in_n_collation_elements = false
        nth_character = nil
        nth_weight = nil
        nth_collation_element = nil
      when / uca900_p([\da-fA-F]{3})\[\]=/
        current_page = Integer($1, 16)
        @pages[current_page] = []
      when /\A  \/\* Primary weight (\d) for each character. \*\//
        nth_character = 0
        nth_weight = 0
        nth_collation_element = Integer($1, 10) - 1
      when /\A  \/\* Secondary weight (\d) for each character. \*\//
        nth_character = 0
        nth_weight = 1
        nth_collation_element = Integer($1, 10) - 1
      when /\A  \/\* Tertiary weight (\d) for each character. \*\//
        nth_character = 0
        nth_weight = 2
        nth_collation_element = Integer($1, 10) - 1
      when /\A  0x([\da-zA-F]+),/
        nth_weight_value = Integer($1, 16)
        next if current_page.nil?
        next if nth_character.nil?
        next if nth_weight.nil?
        next if nth_collation_element.nil?
        if [nth_weight, nth_collation_element] == [0, 0]
          @pages[current_page][nth_character] = []
        end
        weight_sets = @pages[current_page][nth_character]
        weight_sets[nth_collation_element] ||= []
        weight_sets[nth_collation_element][nth_weight] = nth_weight_value
        nth_character += 1
      when /^\};$/
        current_page = nil
      end
    end
  end

  def normalize_pages
    @pages.each do |page, weight_sets|
      @pages[page] = weight_sets.collect.with_index do |weights, i|
        code_point = (page << 8) + i
        Character.new(weights, code_point)
      end
    end
  end
end
