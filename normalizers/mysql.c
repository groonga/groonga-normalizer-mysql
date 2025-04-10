/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013-2025  Sutou Kouhei <kou@clear-code.com>

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
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef GROONGA_NORMALIZER_MYSQL_EMBED
#  define GRN_PLUGIN_FUNCTION_TAG normalizers_mysql
#endif

#include <groonga/nfkc.h>
#include <groonga/normalizer.h>

#include <stdio.h>
#include <string.h>

#include "mysql_general_ci_table.h"
#include "mysql_unicode_1400_ai_ci_table.h"
#include "mysql_unicode_1400_ai_cs_table.h"
#include "mysql_unicode_1400_as_ci_table.h"
#include "mysql_unicode_1400_as_cs_table.h"
#include "mysql_unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_table.h"
#include "mysql_unicode_520_ci_table.h"
#include "mysql_unicode_900_ai_ci_table.h"
#include "mysql_unicode_900_as_ci_table.h"
#include "mysql_unicode_900_as_cs_table.h"
#include "mysql_unicode_900_ja_as_cs_ks_table.h"
#include "mysql_unicode_900_ja_as_cs_table.h"
#include "mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table.h"
#include "mysql_unicode_ci_table.h"

#ifdef __GNUC__
#  define GNUC_UNUSED __attribute__((__unused__))
#else
#  define GNUC_UNUSED
#endif

#ifdef _MSC_VER
#  define inline   _inline
#  define snprintf _snprintf
#endif

#define SNIPPET_BUFFER_SIZE 256

typedef grn_bool (*normalizer_func)(grn_ctx *ctx,
                                    const char *utf8,
                                    int *character_length,
                                    int rest_length,
                                    uint32_t **normalize_table,
                                    char *normalized,
                                    unsigned int *normalized_characer_length,
                                    unsigned int *normalized_length_in_bytes,
                                    unsigned int *normalized_n_characters);

static inline unsigned int
unichar_to_utf8(uint32_t unichar, char *output)
{
  unsigned int n_bytes;

  if (unichar < 0x80) {
    output[0] = unichar;
    n_bytes = 1;
  } else if (unichar < 0x0800) {
    output[0] = ((unichar >> 6) & 0x1f) | 0xc0;
    output[1] = (unichar & 0x3f) | 0x80;
    n_bytes = 2;
  } else if (unichar < 0x10000) {
    output[0] = (unichar >> 12) | 0xe0;
    output[1] = ((unichar >> 6) & 0x3f) | 0x80;
    output[2] = (unichar & 0x3f) | 0x80;
    n_bytes = 3;
  } else if (unichar < 0x200000) {
    output[0] = (unichar >> 18) | 0xf0;
    output[1] = ((unichar >> 12) & 0x3f) | 0x80;
    output[2] = ((unichar >> 6) & 0x3f) | 0x80;
    output[3] = (unichar & 0x3f) | 0x80;
    n_bytes = 4;
  } else if (unichar < 0x4000000) {
    output[0] = (unichar >> 24) | 0xf8;
    output[1] = ((unichar >> 18) & 0x3f) | 0x80;
    output[2] = ((unichar >> 12) & 0x3f) | 0x80;
    output[3] = ((unichar >> 6) & 0x3f) | 0x80;
    output[4] = (unichar & 0x3f) | 0x80;
    n_bytes = 5;
  } else {
    output[0] = (unichar >> 30) | 0xfc;
    output[1] = ((unichar >> 24) & 0x3f) | 0x80;
    output[2] = ((unichar >> 18) & 0x3f) | 0x80;
    output[3] = ((unichar >> 12) & 0x3f) | 0x80;
    output[4] = ((unichar >> 6) & 0x3f) | 0x80;
    output[5] = (unichar & 0x3f) | 0x80;
    n_bytes = 6;
  }

  return n_bytes;
}

static inline uint32_t
utf8_to_unichar(const char *utf8, int byte_size)
{
  uint32_t unichar;
  const unsigned char *bytes = (const unsigned char *)utf8;

  switch (byte_size) {
  case 1:
    unichar = bytes[0] & 0x7f;
    break;
  case 2:
    unichar = ((bytes[0] & 0x1f) << 6) + (bytes[1] & 0x3f);
    break;
  case 3:
    unichar = ((bytes[0] & 0x0f) << 12) + ((bytes[1] & 0x3f) << 6) +
              ((bytes[2] & 0x3f));
    break;
  case 4:
    unichar = ((bytes[0] & 0x07) << 18) + ((bytes[1] & 0x3f) << 12) +
              ((bytes[2] & 0x3f) << 6) + ((bytes[3] & 0x3f));
    break;
  case 5:
    unichar = ((bytes[0] & 0x03) << 24) + ((bytes[1] & 0x3f) << 18) +
              ((bytes[2] & 0x3f) << 12) + ((bytes[3] & 0x3f) << 6) +
              ((bytes[4] & 0x3f));
    break;
  case 6:
    unichar = ((bytes[0] & 0x01) << 30) + ((bytes[1] & 0x3f) << 24) +
              ((bytes[2] & 0x3f) << 18) + ((bytes[3] & 0x3f) << 12) +
              ((bytes[4] & 0x3f) << 6) + ((bytes[5] & 0x3f));
    break;
  default:
    unichar = 0;
    break;
  }

  return unichar;
}

static inline void
decompose_character(const char *rest,
                    int character_length,
                    size_t *page,
                    uint32_t *low_code)
{
  switch (character_length) {
  case 1:
    *page = 0x00;
    *low_code = rest[0] & 0x7f;
    break;
  case 2:
    *page = (rest[0] & 0x1c) >> 2;
    *low_code = ((rest[0] & 0x03) << 6) + (rest[1] & 0x3f);
    break;
  case 3:
    *page = ((rest[0] & 0x0f) << 4) + ((rest[1] & 0x3c) >> 2);
    *low_code = ((rest[1] & 0x03) << 6) + (rest[2] & 0x3f);
    break;
  case 4:
    *page = ((rest[0] & 0x07) << 10) + ((rest[1] & 0x3f) << 4) +
            ((rest[2] & 0x3c) >> 2);
    *low_code = ((rest[2] & 0x03) << 6) + (rest[3] & 0x3f);
    break;
  case 5:
    *page = ((rest[0] & 0x03) << 16) + ((rest[1] & 0x3f) << 10) +
            ((rest[2] & 0x3f) << 4) + ((rest[3] & 0x3c) >> 2);
    *low_code = ((rest[3] & 0x03) << 6) + (rest[4] & 0x3f);
    break;
  case 6:
    *page = ((rest[0] & 0x01) << 22) + ((rest[1] & 0x3f) << 16) +
            ((rest[2] & 0x3f) << 10) + ((rest[3] & 0x3f) << 4) +
            ((rest[4] & 0x3c) >> 2);
    *low_code = ((rest[4] & 0x03) << 6) + (rest[5] & 0x3f);
    break;
  default:
    *page = (size_t)-1;
    *low_code = 0x00;
    break;
  }
}

static inline void
normalize_character(const char *utf8,
                    int character_length,
                    uint32_t **normalize_table,
                    size_t normalize_table_size,
                    char *normalized,
                    unsigned int *normalized_character_length,
                    unsigned int *normalized_length_in_bytes,
                    unsigned int *normalized_n_characters)
{
  size_t page;
  uint32_t low_code;
  decompose_character(utf8, character_length, &page, &low_code);
  if (page < normalize_table_size && normalize_table[page]) {
    uint32_t normalized_code;
    unsigned int n_bytes;
    normalized_code = normalize_table[page][low_code];
    if (normalized_code == 0x00000) {
      *normalized_character_length = 0;
    } else {
      n_bytes = unichar_to_utf8(normalized_code,
                                normalized + *normalized_length_in_bytes);
      *normalized_character_length = n_bytes;
      *normalized_length_in_bytes += n_bytes;
      (*normalized_n_characters)++;
    }
  } else {
    int i;
    for (i = 0; i < character_length; i++) {
      normalized[*normalized_length_in_bytes + i] = utf8[i];
    }
    *normalized_character_length = character_length;
    *normalized_length_in_bytes += character_length;
    (*normalized_n_characters)++;
  }
}

static void
sized_buffer_append(char *buffer,
                    unsigned int *buffer_rest_length,
                    const char *string)
{
  size_t string_length;

  string_length = strlen(string);
  if (string_length >= *buffer_rest_length) {
    return;
  }

  strncat(buffer, string, string_length);
  *buffer_rest_length -= string_length;
}

static void
sized_buffer_dump_string(char *buffer,
                         unsigned int *buffer_rest_length,
                         const char *string,
                         unsigned int string_length)
{
  const unsigned char *bytes;
  unsigned int i;

  bytes = (const unsigned char *)string;
  for (i = 0; i < string_length; i++) {
    unsigned char byte = bytes[i];
#define FORMATTED_BYTE_BUFFER_SIZE 5 /* "0xFF\0" */
    char formatted_byte[FORMATTED_BYTE_BUFFER_SIZE];
    if (i > 0) {
      sized_buffer_append(buffer, buffer_rest_length, " ");
    }
    if (byte == 0) {
      strncpy(formatted_byte, "0x00", FORMATTED_BYTE_BUFFER_SIZE);
    } else {
      snprintf(formatted_byte, FORMATTED_BYTE_BUFFER_SIZE, "%#04x", byte);
    }
    sized_buffer_append(buffer, buffer_rest_length, formatted_byte);
#undef FORMATTED_BYTE_BUFFER_SIZE
  }
}

static const char *
snippet(const char *string,
        unsigned int length,
        unsigned int target_byte,
        char *buffer,
        unsigned int buffer_length)
{
  const char *elision_mark = "...";
  unsigned int max_window_length = 12;
  unsigned int window_length;
  unsigned int buffer_rest_length = buffer_length - 1;

  buffer[0] = '\0';

  if (target_byte > 0) {
    sized_buffer_append(buffer, &buffer_rest_length, elision_mark);
  }

  sized_buffer_append(buffer, &buffer_rest_length, "<");
  if (target_byte + max_window_length > length) {
    window_length = length - target_byte;
  } else {
    window_length = max_window_length;
  }
  sized_buffer_dump_string(buffer,
                           &buffer_rest_length,
                           string + target_byte,
                           window_length);
  sized_buffer_append(buffer, &buffer_rest_length, ">");

  if (target_byte + window_length < length) {
    sized_buffer_append(buffer, &buffer_rest_length, elision_mark);
  }

  return buffer;
}

static void
normalize(grn_ctx *ctx,
          grn_obj *string,
          const char *normalizer_type_label,
          uint32_t **normalize_table,
          size_t normalize_table_size,
          normalizer_func custom_normalizer)
{
  const char *original, *rest;
  unsigned int original_length_in_bytes, rest_length;
  char *normalized;
  unsigned int normalized_length_in_bytes = 0;
  unsigned int normalized_n_characters = 0;
  unsigned char *types = NULL;
  unsigned char *current_type = NULL;
  short *checks = NULL;
  short *current_check = NULL;
  grn_encoding encoding;
  int flags;
  grn_bool remove_blank_p;

  encoding = grn_string_get_encoding(ctx, string);
  flags = grn_string_get_flags(ctx, string);
  remove_blank_p = flags & GRN_STRING_REMOVE_BLANK;
  grn_string_get_original(ctx, string, &original, &original_length_in_bytes);
  {
    unsigned int max_normalized_length_in_bytes = original_length_in_bytes + 1;
    normalized = GRN_PLUGIN_MALLOC(ctx, max_normalized_length_in_bytes);
  }
  if (flags & GRN_STRING_WITH_TYPES) {
    unsigned int max_normalized_n_characters = original_length_in_bytes + 1;
    types = GRN_PLUGIN_MALLOC(ctx, max_normalized_n_characters);
    current_type = types;
  }
  if (flags & GRN_STRING_WITH_CHECKS) {
    unsigned int max_checks_size = sizeof(short) * original_length_in_bytes + 1;
    checks = GRN_PLUGIN_MALLOC(ctx, max_checks_size);
    current_check = checks;
    current_check[0] = 0;
  }
  rest = original;
  rest_length = original_length_in_bytes;
  while (rest_length > 0) {
    int character_length;
    grn_bool custom_normalized = GRN_FALSE;
    unsigned int normalized_character_length;
    unsigned int previous_normalized_length_in_bytes =
      normalized_length_in_bytes;
    unsigned int previous_normalized_n_characters = normalized_n_characters;

    character_length = grn_plugin_charlen(ctx, rest, rest_length, encoding);
    if (character_length == 0) {
      break;
    }

    if (custom_normalizer) {
      custom_normalized = custom_normalizer(ctx,
                                            rest,
                                            &character_length,
                                            rest_length - character_length,
                                            normalize_table,
                                            normalized,
                                            &normalized_character_length,
                                            &normalized_length_in_bytes,
                                            &normalized_n_characters);
    }
    if (!custom_normalized) {
      normalize_character(rest,
                          character_length,
                          normalize_table,
                          normalize_table_size,
                          normalized,
                          &normalized_character_length,
                          &normalized_length_in_bytes,
                          &normalized_n_characters);
    }

    if (remove_blank_p && normalized_character_length == 1 &&
        normalized[previous_normalized_length_in_bytes] == ' ') {
      if (current_type > types) {
        current_type[-1] |= GRN_CHAR_BLANK;
      }
      if (current_check) {
        current_check[0] += character_length;
      }
      normalized_length_in_bytes = previous_normalized_length_in_bytes;
      normalized_n_characters = previous_normalized_n_characters;
    } else {
      if (current_type && normalized_character_length > 0) {
        char *current_normalized;
        current_normalized =
          normalized + normalized_length_in_bytes - normalized_character_length;
        current_type[0] =
          grn_nfkc_char_type((unsigned char *)current_normalized);
        current_type++;
      }
      if (current_check) {
        current_check[0] += character_length;
        if (normalized_character_length > 0) {
          unsigned int i;
          current_check++;
          for (i = 1; i < normalized_character_length; i++) {
            current_check[0] = 0;
            current_check++;
          }
          current_check[0] = 0;
        }
      }
    }

    rest += character_length;
    rest_length -= character_length;
  }
  if (current_type) {
    current_type[0] = GRN_CHAR_NULL;
  }
  normalized[normalized_length_in_bytes] = '\0';

  if (rest_length > 0) {
    char buffer[SNIPPET_BUFFER_SIZE];
    GRN_PLUGIN_LOG(ctx,
                   GRN_LOG_DEBUG,
                   "[normalizer][%s] failed to normalize at %u byte: %s",
                   normalizer_type_label,
                   original_length_in_bytes - rest_length,
                   snippet(original,
                           original_length_in_bytes,
                           original_length_in_bytes - rest_length,
                           buffer,
                           SNIPPET_BUFFER_SIZE));
  }
  grn_string_set_normalized(ctx,
                            string,
                            normalized,
                            normalized_length_in_bytes,
                            normalized_n_characters);
  grn_string_set_types(ctx, string, types);
  grn_string_set_checks(ctx, string, checks);
}

static grn_obj *
mysql_general_ci_next(GNUC_UNUSED grn_ctx *ctx,
                      GNUC_UNUSED int nargs,
                      grn_obj **args,
                      GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;
  const char *normalizer_type_label = "mysql-general-ci";

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx,
            string,
            normalizer_type_label,
            general_ci_table,
            sizeof(general_ci_table) / sizeof(uint32_t *),
            NULL);
  return NULL;
}

static grn_obj *
mysql_unicode_ci_next(GNUC_UNUSED grn_ctx *ctx,
                      GNUC_UNUSED int nargs,
                      grn_obj **args,
                      GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;
  const char *normalizer_type_label = "mysql-unicode-ci";

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx,
            string,
            normalizer_type_label,
            unicode_ci_table,
            sizeof(unicode_ci_table) / sizeof(uint32_t *),
            NULL);
  return NULL;
}

#define HALFWIDTH_KATAKANA_LETTER_KA              0xff76
#define HALFWIDTH_KATAKANA_LETTER_KI              0xff77
#define HALFWIDTH_KATAKANA_LETTER_KU              0xff78
#define HALFWIDTH_KATAKANA_LETTER_KE              0xff79
#define HALFWIDTH_KATAKANA_LETTER_KO              0xff7a

#define HALFWIDTH_KATAKANA_LETTER_SA              0xff7b
#define HALFWIDTH_KATAKANA_LETTER_SI              0xff7c
#define HALFWIDTH_KATAKANA_LETTER_SU              0xff7d
#define HALFWIDTH_KATAKANA_LETTER_SE              0xff7e
#define HALFWIDTH_KATAKANA_LETTER_SO              0xff7f

#define HALFWIDTH_KATAKANA_LETTER_TA              0xff80
#define HALFWIDTH_KATAKANA_LETTER_TI              0xff81
#define HALFWIDTH_KATAKANA_LETTER_TU              0xff82
#define HALFWIDTH_KATAKANA_LETTER_TE              0xff83
#define HALFWIDTH_KATAKANA_LETTER_TO              0xff84

#define HALFWIDTH_KATAKANA_LETTER_HA              0xff8a
#define HALFWIDTH_KATAKANA_LETTER_HI              0xff8b
#define HALFWIDTH_KATAKANA_LETTER_HU              0xff8c
#define HALFWIDTH_KATAKANA_LETTER_HE              0xff8d
#define HALFWIDTH_KATAKANA_LETTER_HO              0xff8e

#define HALFWIDTH_KATAKANA_VOICED_SOUND_MARK      0xff9e
#define HALFWIDTH_KATAKANA_SEMI_VOICED_SOUND_MARK 0xff9f

#define HIRAGANA_LETTER_KA                        0x304b
#define HIRAGANA_VOICED_SOUND_MARK_OFFSET         1
#define HIRAGANA_VOICED_SOUND_MARK_GAP            2

#define HIRAGANA_LETTER_HA                        0x306f
#define HIRAGANA_HA_LINE_BA_OFFSET                1
#define HIRAGANA_HA_LINE_PA_OFFSET                2
#define HIRAGANA_HA_LINE_GAP                      3

static grn_bool
normalize_halfwidth_katakana_with_voiced_sound_mark(
  grn_ctx *ctx,
  const char *utf8,
  int *character_length,
  int rest_length,
  GNUC_UNUSED uint32_t **normalize_table,
  char *normalized,
  unsigned int *normalized_character_length,
  unsigned int *normalized_length_in_bytes,
  unsigned int *normalized_n_characters)
{
  grn_bool custom_normalized = GRN_FALSE;
  grn_bool is_voiced_sound_markable_halfwidth_katakana = GRN_FALSE;
  grn_bool is_semi_voiced_sound_markable_halfwidth_katakana = GRN_FALSE;
  grn_bool is_ha_line = GRN_FALSE;
  uint32_t unichar;

  if (*character_length != 3) {
    return GRN_FALSE;
  }
  if (rest_length < 3) {
    return GRN_FALSE;
  }

  unichar = utf8_to_unichar(utf8, *character_length);
  if (HALFWIDTH_KATAKANA_LETTER_KA <= unichar &&
      unichar <= HALFWIDTH_KATAKANA_LETTER_TO) {
    is_voiced_sound_markable_halfwidth_katakana = GRN_TRUE;
  } else if (HALFWIDTH_KATAKANA_LETTER_HA <= unichar &&
             unichar <= HALFWIDTH_KATAKANA_LETTER_HO) {
    is_voiced_sound_markable_halfwidth_katakana = GRN_TRUE;
    is_semi_voiced_sound_markable_halfwidth_katakana = GRN_TRUE;
    is_ha_line = GRN_TRUE;
  }

  if (!is_voiced_sound_markable_halfwidth_katakana &&
      !is_semi_voiced_sound_markable_halfwidth_katakana) {
    return GRN_FALSE;
  }

  {
    int next_character_length;
    uint32_t next_unichar;
    next_character_length = grn_plugin_charlen(ctx,
                                               utf8 + *character_length,
                                               rest_length,
                                               GRN_ENC_UTF8);
    if (next_character_length != 3) {
      return GRN_FALSE;
    }
    next_unichar =
      utf8_to_unichar(utf8 + *character_length, next_character_length);
    if (next_unichar == HALFWIDTH_KATAKANA_VOICED_SOUND_MARK) {
      if (is_voiced_sound_markable_halfwidth_katakana) {
        unsigned int n_bytes;
        if (is_ha_line) {
          n_bytes = unichar_to_utf8(
            HIRAGANA_LETTER_HA + HIRAGANA_HA_LINE_BA_OFFSET +
              ((unichar - HALFWIDTH_KATAKANA_LETTER_HA) * HIRAGANA_HA_LINE_GAP),
            normalized + *normalized_length_in_bytes);
        } else {
          int small_tu_offset = 0;
          if (HALFWIDTH_KATAKANA_LETTER_TU <= unichar &&
              unichar <= HALFWIDTH_KATAKANA_LETTER_TO) {
            small_tu_offset = 1;
          }
          n_bytes = unichar_to_utf8(
            HIRAGANA_LETTER_KA + HIRAGANA_VOICED_SOUND_MARK_OFFSET +
              small_tu_offset +
              ((unichar - HALFWIDTH_KATAKANA_LETTER_KA) *
               HIRAGANA_VOICED_SOUND_MARK_GAP),
            normalized + *normalized_length_in_bytes);
        }
        *character_length += next_character_length;
        *normalized_character_length = n_bytes;
        *normalized_length_in_bytes += n_bytes;
        (*normalized_n_characters)++;
        custom_normalized = GRN_TRUE;
      }
    } else if (next_unichar == HALFWIDTH_KATAKANA_SEMI_VOICED_SOUND_MARK) {
      if (is_semi_voiced_sound_markable_halfwidth_katakana) {
        unsigned int n_bytes;
        n_bytes = unichar_to_utf8(
          HIRAGANA_LETTER_HA + HIRAGANA_HA_LINE_PA_OFFSET +
            ((unichar - HALFWIDTH_KATAKANA_LETTER_HA) * HIRAGANA_HA_LINE_GAP),
          normalized + *normalized_length_in_bytes);
        *character_length += next_character_length;
        *normalized_character_length = n_bytes;
        *normalized_length_in_bytes += n_bytes;
        (*normalized_n_characters)++;
        custom_normalized = GRN_TRUE;
      }
    }
  }

  return custom_normalized;
}

static grn_obj *
mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_next(
  GNUC_UNUSED grn_ctx *ctx,
  GNUC_UNUSED int nargs,
  grn_obj **args,
  GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;
  const char *normalizer_type_label =
    "mysql-unicode-ci-except-kana-ci-kana-with-voiced-sound-mark";

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(
    ctx,
    string,
    normalizer_type_label,
    unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table,
    sizeof(unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table) /
      sizeof(uint32_t *),
    normalize_halfwidth_katakana_with_voiced_sound_mark);
  return NULL;
}

static grn_obj *
mysql_unicode_520_ci_next(GNUC_UNUSED grn_ctx *ctx,
                          GNUC_UNUSED int nargs,
                          grn_obj **args,
                          GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;
  const char *normalizer_type_label = "mysql-unicode-520-ci";

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx,
            string,
            normalizer_type_label,
            unicode_520_ci_table,
            sizeof(unicode_520_ci_table) / sizeof(uint32_t *),
            NULL);
  return NULL;
}

static grn_obj *
mysql_unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_next(
  GNUC_UNUSED grn_ctx *ctx,
  GNUC_UNUSED int nargs,
  grn_obj **args,
  GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;
  const char *normalizer_type_label =
    "mysql-unicode-520-ci-except-kana-ci-kana-with-voiced-sound-mark";

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(
    ctx,
    string,
    normalizer_type_label,
    unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_table,
    sizeof(unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_table) /
      sizeof(uint32_t *),
    normalize_halfwidth_katakana_with_voiced_sound_mark);
  return NULL;
}

typedef enum {
  MYSQL_UNICODE_900_LOCALE_NONE,
  MYSQL_UNICODE_900_LOCALE_JA,
} mysql_unicode_900_locale;

typedef struct {
  uint8_t weight_level;
  mysql_unicode_900_locale locale;
} mysql_unicode_900_options;

static void
mysql_unicode_900_options_init(mysql_unicode_900_options *options)
{
  options->weight_level = 1;
  options->locale = MYSQL_UNICODE_900_LOCALE_NONE;
}

static void *
mysql_unicode_900_open_options(grn_ctx *ctx,
                               GNUC_UNUSED grn_obj *normalizer,
                               grn_obj *raw_options,
                               GNUC_UNUSED void *user_data)
{
  const char *normalizer_type_label = "mysql-unicode-900";
  mysql_unicode_900_options *options;

  options = GRN_PLUGIN_MALLOC(ctx, sizeof(*options));
  if (!options) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_NO_MEMORY_AVAILABLE,
                     "[normalizer][%s] "
                     "failed to allocate memory for options",
                     normalizer_type_label);
    return NULL;
  }

  mysql_unicode_900_options_init(options);

  GRN_OPTION_VALUES_EACH_BEGIN(ctx, raw_options, i, name, name_length)
  {
    /* TODO: Use grn_raw_string. */
#define STRING_EQUAL(raw_string, raw_string_length, c_string)                  \
  (raw_string_length == strlen(c_string) &&                                    \
   strncmp(raw_string, c_string, raw_string_length) == 0)
    if (STRING_EQUAL(name, name_length, "weight_level")) {
      options->weight_level =
        grn_vector_get_element_uint8(ctx,
                                     raw_options,
                                     i,
                                     options->weight_level);
    } else if (STRING_EQUAL(name, name_length, "locale")) {
      const char *locale;
      unsigned int locale_length;
      grn_id domain;
      locale_length =
        grn_vector_get_element(ctx, raw_options, i, &locale, NULL, &domain);
      if (!grn_type_id_is_text_family(ctx, domain)) {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        {
          grn_obj value;
          grn_obj inspected;
          GRN_OBJ_INIT(&value, GRN_BULK, 0, domain);
          grn_bulk_write(ctx, &value, locale, locale_length);
          GRN_TEXT_INIT(&inspected, 0);
          grn_inspect(ctx, &value, &inspected);
          GRN_OBJ_FIN(ctx, &value);
          GRN_PLUGIN_ERROR(ctx,
                           GRN_INVALID_ARGUMENT,
                           "[normalizer][%s] "
                           "locale must be text: <%.*s>",
                           normalizer_type_label,
                           (int)GRN_TEXT_LEN(&inspected),
                           GRN_TEXT_VALUE(&inspected));
          GRN_OBJ_FIN(ctx, &inspected);
        }
        break;
      }
      if (STRING_EQUAL(locale, locale_length, "none")) {
        options->locale = MYSQL_UNICODE_900_LOCALE_NONE;
      } else if (STRING_EQUAL(locale, locale_length, "ja")) {
        options->locale = MYSQL_UNICODE_900_LOCALE_JA;
      } else {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        GRN_PLUGIN_ERROR(ctx,
                         GRN_INVALID_ARGUMENT,
                         "[normalizer][%s] "
                         "locale must be <none> or <ja>: <%.*s>",
                         normalizer_type_label,
                         (int)locale_length,
                         locale);
        break;
      }
    }
#undef NAME_EQUAL
  }
  GRN_OPTION_VALUES_EACH_END();

  return options;
}

static void
mysql_unicode_900_close_options(grn_ctx *ctx, void *data)
{
  mysql_unicode_900_options *options = data;
  GRN_PLUGIN_FREE(ctx, options);
}

static grn_obj *
mysql_unicode_900_next(grn_ctx *ctx,
                       GNUC_UNUSED int n_args,
                       grn_obj **args,
                       GNUC_UNUSED grn_user_data *user_data)
{
  const char *normalizer_type_label = "mysql-unicode-900";
  grn_obj *string = args[0];
  grn_encoding encoding;
  grn_obj *table;
  mysql_unicode_900_options *options;
  mysql_unicode_900_options options_raw;

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }

  table = grn_string_get_table(ctx, string);
  if (table) {
    options =
      grn_table_cache_normalizer_options(ctx,
                                         table,
                                         string,
                                         mysql_unicode_900_open_options,
                                         mysql_unicode_900_close_options,
                                         NULL);
    if (ctx->rc != GRN_SUCCESS) {
      return NULL;
    }
  } else {
    mysql_unicode_900_options_init(&options_raw);
    options = &options_raw;
  }
  if (options->weight_level == 1) {
    normalize(ctx,
              string,
              normalizer_type_label,
              unicode_900_ai_ci_table,
              sizeof(unicode_900_ai_ci_table) / sizeof(uint32_t *),
              NULL);
  } else if (options->weight_level == 2) {
    normalize(ctx,
              string,
              normalizer_type_label,
              unicode_900_as_ci_table,
              sizeof(unicode_900_as_ci_table) / sizeof(uint32_t *),
              NULL);
  } else if (options->weight_level == 3) {
    switch (options->locale) {
    case MYSQL_UNICODE_900_LOCALE_JA:
      normalize(ctx,
                string,
                normalizer_type_label,
                unicode_900_ja_as_cs_table,
                sizeof(unicode_900_ja_as_cs_table) / sizeof(uint32_t *),
                NULL);
      break;
    default:
      normalize(ctx,
                string,
                normalizer_type_label,
                unicode_900_as_cs_table,
                sizeof(unicode_900_as_cs_table) / sizeof(uint32_t *),
                NULL);
      break;
    }
  } else if (options->weight_level == 4) {
    switch (options->locale) {
    case MYSQL_UNICODE_900_LOCALE_JA:
      normalize(ctx,
                string,
                normalizer_type_label,
                unicode_900_ja_as_cs_ks_table,
                sizeof(unicode_900_ja_as_cs_ks_table) / sizeof(uint32_t *),
                NULL);
      break;
    default:
      GRN_PLUGIN_ERROR(ctx,
                       GRN_FUNCTION_NOT_IMPLEMENTED,
                       "[normalizer][%s] "
                       "locale must be ja for weight level 4",
                       normalizer_type_label);
      break;
    }
  } else {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "weight level must be 1, 2, 3 or 4: %u",
                     normalizer_type_label,
                     options->weight_level);
    return NULL;
  }

  return NULL;
}

typedef enum {
  MYSQL_UNICODE_VERSION_900,
  MYSQL_UNICODE_VERSION_1400,
} mysql_unicode_version;

static const char *
mysql_unicode_version_to_string(mysql_unicode_version version)
{
  switch (version) {
  case MYSQL_UNICODE_VERSION_900:
    return "900";
  case MYSQL_UNICODE_VERSION_1400:
    return "1400";
  }
  return "unknown";
}

typedef enum {
  MYSQL_UNICODE_LOCALE_NONE,
  MYSQL_UNICODE_LOCALE_JA,
} mysql_unicode_locale;

static const char *
mysql_unicode_locale_to_string(mysql_unicode_locale locale)
{
  switch (locale) {
  case MYSQL_UNICODE_LOCALE_NONE:
    return "none";
  case MYSQL_UNICODE_LOCALE_JA:
    return "ja";
  }
  return "unknown";
}

typedef struct {
  mysql_unicode_version version;
  bool accent_sensitive;
  bool case_sensitive;
  bool kana_sensitive;
  mysql_unicode_locale locale;
} mysql_unicode_options;

static void
mysql_unicode_options_init(mysql_unicode_options *options)
{
  options->version = MYSQL_UNICODE_VERSION_1400;
  options->accent_sensitive = false;
  options->case_sensitive = false;
  options->kana_sensitive = false;
  options->locale = MYSQL_UNICODE_LOCALE_NONE;
}

static void *
mysql_unicode_open_options(grn_ctx *ctx,
                           GNUC_UNUSED grn_obj *normalizer,
                           grn_obj *raw_options,
                           GNUC_UNUSED void *user_data)
{
  const char *normalizer_type_label = "mysql-unicode";
  mysql_unicode_options *options;

  options = GRN_PLUGIN_MALLOC(ctx, sizeof(*options));
  if (!options) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_NO_MEMORY_AVAILABLE,
                     "[normalizer][%s] "
                     "failed to allocate memory for options",
                     normalizer_type_label);
    return NULL;
  }

  mysql_unicode_options_init(options);

  GRN_OPTION_VALUES_EACH_BEGIN(ctx, raw_options, i, name, name_length)
  {
    grn_raw_string name_raw;
    name_raw.value = name;
    name_raw.length = name_length;

    if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "version")) {
      grn_raw_string version_raw;
      grn_id domain;
      version_raw.length = grn_vector_get_element(ctx,
                                                  raw_options,
                                                  i,
                                                  &(version_raw.value),
                                                  NULL,
                                                  &domain);
      if (!grn_type_id_is_text_family(ctx, domain)) {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        {
          grn_obj value;
          grn_obj inspected;
          GRN_OBJ_INIT(&value, GRN_BULK, 0, domain);
          grn_bulk_write(ctx, &value, version_raw.value, version_raw.length);
          GRN_TEXT_INIT(&inspected, 0);
          grn_inspect(ctx, &value, &inspected);
          GRN_OBJ_FIN(ctx, &value);
          GRN_PLUGIN_ERROR(ctx,
                           GRN_INVALID_ARGUMENT,
                           "[normalizer][%s] "
                           "version must be text: <%.*s>",
                           normalizer_type_label,
                           (int)GRN_TEXT_LEN(&inspected),
                           GRN_TEXT_VALUE(&inspected));
          GRN_OBJ_FIN(ctx, &inspected);
        }
        break;
      }
      if (GRN_RAW_STRING_EQUAL_CSTRING(version_raw, "9.0.0")) {
        options->version = MYSQL_UNICODE_VERSION_900;
      } else if (GRN_RAW_STRING_EQUAL_CSTRING(version_raw, "14.0.0")) {
        options->version = MYSQL_UNICODE_VERSION_1400;
      } else {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        GRN_PLUGIN_ERROR(ctx,
                         GRN_INVALID_ARGUMENT,
                         "[normalizer][%s] "
                         "version must be <900> or <1400>: <%.*s>",
                         normalizer_type_label,
                         (int)(version_raw.length),
                         version_raw.value);
        break;
      }
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "accent_sensitive")) {
      options->accent_sensitive =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->accent_sensitive);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "case_sensitive")) {
      options->case_sensitive =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->case_sensitive);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "kana_sensitive")) {
      options->kana_sensitive =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->kana_sensitive);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "locale")) {
      grn_raw_string locale_raw;
      grn_id domain;
      locale_raw.length = grn_vector_get_element(ctx,
                                                 raw_options,
                                                 i,
                                                 &(locale_raw.value),
                                                 NULL,
                                                 &domain);
      if (!grn_type_id_is_text_family(ctx, domain)) {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        {
          grn_obj value;
          grn_obj inspected;
          GRN_OBJ_INIT(&value, GRN_BULK, 0, domain);
          grn_bulk_write(ctx, &value, locale_raw.value, locale_raw.length);
          GRN_TEXT_INIT(&inspected, 0);
          grn_inspect(ctx, &value, &inspected);
          GRN_OBJ_FIN(ctx, &value);
          GRN_PLUGIN_ERROR(ctx,
                           GRN_INVALID_ARGUMENT,
                           "[normalizer][%s] "
                           "locale must be text: <%.*s>",
                           normalizer_type_label,
                           (int)GRN_TEXT_LEN(&inspected),
                           GRN_TEXT_VALUE(&inspected));
          GRN_OBJ_FIN(ctx, &inspected);
        }
        break;
      }
      if (GRN_RAW_STRING_EQUAL_CSTRING(locale_raw, "none")) {
        options->locale = MYSQL_UNICODE_LOCALE_NONE;
      } else if (GRN_RAW_STRING_EQUAL_CSTRING(locale_raw, "ja")) {
        options->locale = MYSQL_UNICODE_LOCALE_JA;
      } else {
        GRN_PLUGIN_FREE(ctx, options);
        options = NULL;
        GRN_PLUGIN_ERROR(ctx,
                         GRN_INVALID_ARGUMENT,
                         "[normalizer][%s] "
                         "locale must be <none> or <ja>: <%.*s>",
                         normalizer_type_label,
                         (int)(locale_raw.length),
                         locale_raw.value);
        break;
      }
    }
  }
  GRN_OPTION_VALUES_EACH_END();

  if (!options) {
    return NULL;
  }

  if (options->version == MYSQL_UNICODE_VERSION_900) {
    if (options->locale == MYSQL_UNICODE_LOCALE_JA) {
      if (!options->accent_sensitive) {
        GRN_PLUGIN_ERROR(ctx,
                         GRN_INVALID_ARGUMENT,
                         "[normalizer][%s][%s][ja] "
                         "accent_sensitive must be true",
                         normalizer_type_label,
                         mysql_unicode_version_to_string(options->version));
        GRN_PLUGIN_FREE(ctx, options);
        return NULL;
      }
      if (!options->case_sensitive) {
        GRN_PLUGIN_ERROR(ctx,
                         GRN_INVALID_ARGUMENT,
                         "[normalizer][%s][%s][ja] "
                         "case_sensitive must be true",
                         normalizer_type_label,
                         mysql_unicode_version_to_string(options->version));
        GRN_PLUGIN_FREE(ctx, options);
        return NULL;
      }
    } else {
      if (options->kana_sensitive) {
        GRN_PLUGIN_ERROR(
          ctx,
          GRN_INVALID_ARGUMENT,
          "[normalizer][%s][%s] "
          "kana_sensitive=true is available only with locale=ja: "
          "<%s>",
          normalizer_type_label,
          mysql_unicode_version_to_string(options->version),
          mysql_unicode_locale_to_string(options->locale));
        GRN_PLUGIN_FREE(ctx, options);
        return NULL;
      }
    }
    if (!options->accent_sensitive && options->case_sensitive) {
      GRN_PLUGIN_ERROR(
        ctx,
        GRN_INVALID_ARGUMENT,
        "[normalizer][%s][%s] "
        "can't mix accent_sensitive=false and case_sensitive=true",
        normalizer_type_label,
        mysql_unicode_version_to_string(options->version));
      GRN_PLUGIN_FREE(ctx, options);
      return NULL;
    }
  } else if (options->version == MYSQL_UNICODE_VERSION_1400) {
    if (options->locale != MYSQL_UNICODE_LOCALE_NONE) {
      GRN_PLUGIN_ERROR(ctx,
                       GRN_INVALID_ARGUMENT,
                       "[normalizer][%s][%s] "
                       "locale must be <none>: <%s>",
                       normalizer_type_label,
                       mysql_unicode_version_to_string(options->version),
                       mysql_unicode_locale_to_string(options->locale));
      GRN_PLUGIN_FREE(ctx, options);
      return NULL;
    }
    if (options->kana_sensitive) {
      GRN_PLUGIN_ERROR(ctx,
                       GRN_INVALID_ARGUMENT,
                       "[normalizer][%s][%s] "
                       "can't use kana_sensitive=true",
                       normalizer_type_label,
                       mysql_unicode_version_to_string(options->version));
      GRN_PLUGIN_FREE(ctx, options);
      return NULL;
    }
  }

  return options;
}

static void
mysql_unicode_close_options(grn_ctx *ctx, void *data)
{
  mysql_unicode_options *options = data;
  GRN_PLUGIN_FREE(ctx, options);
}

static grn_obj *
mysql_unicode_next(grn_ctx *ctx,
                   GNUC_UNUSED int n_args,
                   grn_obj **args,
                   GNUC_UNUSED grn_user_data *user_data)
{
  const char *normalizer_type_label = "mysql-unicode";
  grn_obj *string = args[0];
  grn_encoding encoding;
  grn_obj *table;
  mysql_unicode_options *options;
  mysql_unicode_options options_raw;

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][%s] "
                     "UTF-8 encoding is only supported: %s",
                     normalizer_type_label,
                     grn_encoding_to_string(encoding));
    return NULL;
  }

  table = grn_string_get_table(ctx, string);
  if (table) {
    options = grn_table_cache_normalizer_options(ctx,
                                                 table,
                                                 string,
                                                 mysql_unicode_open_options,
                                                 mysql_unicode_close_options,
                                                 NULL);
    if (ctx->rc != GRN_SUCCESS) {
      return NULL;
    }
  } else {
    mysql_unicode_options_init(&options_raw);
    options = &options_raw;
  }
  switch (options->version) {
  case MYSQL_UNICODE_VERSION_900:
    if (options->accent_sensitive) {
      if (options->case_sensitive) {
        if (options->locale == MYSQL_UNICODE_LOCALE_JA) {
          if (options->kana_sensitive) {
            normalize(ctx,
                      string,
                      normalizer_type_label,
                      unicode_900_ja_as_cs_ks_table,
                      sizeof(unicode_900_ja_as_cs_ks_table) /
                        sizeof(uint32_t *),
                      NULL);
          } else {
            normalize(ctx,
                      string,
                      normalizer_type_label,
                      unicode_900_ja_as_cs_table,
                      sizeof(unicode_900_ja_as_cs_table) / sizeof(uint32_t *),
                      NULL);
          }
        } else {
          normalize(ctx,
                    string,
                    normalizer_type_label,
                    unicode_900_as_cs_table,
                    sizeof(unicode_900_as_cs_table) / sizeof(uint32_t *),
                    NULL);
        }
      } else {
        normalize(ctx,
                  string,
                  normalizer_type_label,
                  unicode_900_as_ci_table,
                  sizeof(unicode_900_as_ci_table) / sizeof(uint32_t *),
                  NULL);
      }
    } else {
      normalize(ctx,
                string,
                normalizer_type_label,
                unicode_900_ai_ci_table,
                sizeof(unicode_900_ai_ci_table) / sizeof(uint32_t *),
                NULL);
    }
    break;
  case MYSQL_UNICODE_VERSION_1400:
    if (options->accent_sensitive) {
      if (options->case_sensitive) {
        normalize(ctx,
                  string,
                  normalizer_type_label,
                  unicode_1400_as_cs_table,
                  sizeof(unicode_1400_as_cs_table) / sizeof(uint32_t *),
                  NULL);
      } else {
        normalize(ctx,
                  string,
                  normalizer_type_label,
                  unicode_1400_as_ci_table,
                  sizeof(unicode_1400_as_ci_table) / sizeof(uint32_t *),
                  NULL);
      }
    } else {
      if (options->case_sensitive) {
        normalize(ctx,
                  string,
                  normalizer_type_label,
                  unicode_1400_ai_cs_table,
                  sizeof(unicode_1400_ai_cs_table) / sizeof(uint32_t *),
                  NULL);
      } else {
        normalize(ctx,
                  string,
                  normalizer_type_label,
                  unicode_1400_ai_ci_table,
                  sizeof(unicode_1400_ai_ci_table) / sizeof(uint32_t *),
                  NULL);
      }
    }
    break;
  }

  return NULL;
}

grn_rc
GRN_PLUGIN_INIT(grn_ctx *ctx)
{
  return ctx->rc;
}

grn_rc
GRN_PLUGIN_REGISTER(grn_ctx *ctx)
{
  grn_normalizer_register(ctx,
                          "NormalizerMySQLGeneralCI",
                          -1,
                          NULL,
                          mysql_general_ci_next,
                          NULL);
  grn_normalizer_register(ctx,
                          "NormalizerMySQLUnicodeCI",
                          -1,
                          NULL,
                          mysql_unicode_ci_next,
                          NULL);
  grn_normalizer_register(
    ctx,
    "NormalizerMySQLUnicodeCI"
    "Except"
    "KanaCI"
    "KanaWithVoicedSoundMark",
    -1,
    NULL,
    mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_next,
    NULL);
  grn_normalizer_register(ctx,
                          "NormalizerMySQLUnicode520CI",
                          -1,
                          NULL,
                          mysql_unicode_520_ci_next,
                          NULL);
  grn_normalizer_register(
    ctx,
    "NormalizerMySQLUnicode520CI"
    "Except"
    "KanaCI"
    "KanaWithVoicedSoundMark",
    -1,
    NULL,
    mysql_unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_next,
    NULL);
  grn_normalizer_register(ctx,
                          "NormalizerMySQLUnicode900",
                          -1,
                          NULL,
                          mysql_unicode_900_next,
                          NULL);
  grn_normalizer_register(ctx,
                          "NormalizerMySQLUnicode",
                          -1,
                          NULL,
                          mysql_unicode_next,
                          NULL);
  return GRN_SUCCESS;
}

grn_rc
GRN_PLUGIN_FIN(GNUC_UNUSED grn_ctx *ctx)
{
  return GRN_SUCCESS;
}
