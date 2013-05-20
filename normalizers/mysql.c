/* -*- c-basic-offset: 2 -*- */
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
*/

#include <groonga/normalizer.h>
#include <groonga/nfkc.h>

#include "mysql_general_ci_table.h"
#include "mysql_unicode_ci_table.h"
#include "mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table.h"

#ifdef __GNUC__
#  define GNUC_UNUSED __attribute__((__unused__))
#else
#  define GNUC_UNUSED
#endif

#ifdef _MSC_VER
#  define inline _inline
#endif

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

static inline void
decompose_character(const char *rest, int character_length,
                    int *page, uint32_t *low_code)
{
  switch (character_length) {
  case 1 :
    *page = 0x00;
    *low_code = rest[0] & 0x7f;
    break;
  case 2 :
    *page = (rest[0] & 0x1c) >> 2;
    *low_code = ((rest[0] & 0x03) << 6) + (rest[1] & 0x3f);
    break;
  case 3 :
    *page = ((rest[0] & 0x0f) << 4) + ((rest[1] & 0x3c) >> 2);
    *low_code = ((rest[1] & 0x03) << 6) + (rest[2] & 0x3f);
    break;
  case 4 :
    *page =
      ((rest[0] & 0x07) << 10) +
      ((rest[1] & 0x3f) << 4) +
      ((rest[2] & 0x3c) >> 2);
    *low_code = ((rest[1] & 0x03) << 6) + (rest[2] & 0x3f);
    break;
  default :
    *page = -1;
    *low_code = 0x00;
    break;
  }
}


static void
normalize(grn_ctx *ctx, grn_obj *string, uint32_t **normalize_table)
{
  const char *original, *rest;
  unsigned int original_length_in_bytes, rest_length;
  char *normalized;
  unsigned int normalized_length_in_bytes = 0;
  unsigned int normalized_n_characters = 0;
  unsigned char *types = NULL;
  unsigned char *current_type = NULL;
  grn_encoding encoding;
  int flags;
  grn_bool remove_blank_p;

  encoding = grn_string_get_encoding(ctx, string);
  flags = grn_string_get_flags(ctx, string);
  remove_blank_p = flags & GRN_STRING_REMOVE_BLANK;
  grn_string_get_original(ctx, string, &original, &original_length_in_bytes);
  {
    unsigned int max_normalized_length_in_bytes = original_length_in_bytes;
    normalized = GRN_PLUGIN_MALLOC(ctx, max_normalized_length_in_bytes);
  }
  if (flags & GRN_STRING_WITH_TYPES) {
    unsigned int max_normalized_n_characters = original_length_in_bytes;
    types = GRN_PLUGIN_MALLOC(ctx, max_normalized_n_characters);
    current_type = types;
  }
  rest = original;
  rest_length = original_length_in_bytes;
  while (rest_length > 0) {
    int character_length;

    character_length = grn_plugin_charlen(ctx, rest, rest_length, encoding);
    if (character_length == 0) {
      break;
    }

    if (remove_blank_p && character_length == 1 && rest[0] == ' ') {
      if (current_type > types) {
        current_type[-1] |= GRN_CHAR_BLANK;
      }
    } else {
      int page;
      uint32_t low_code;
      decompose_character(rest, character_length, &page, &low_code);
      if ((0x00 <= page && page <= 0xff) && normalize_table[page]) {
        uint32_t normalized_code;
        unsigned int n_bytes;
        normalized_code = normalize_table[page][low_code];
        if (normalized_code != 0) {
          n_bytes = unichar_to_utf8(normalized_code,
                                    normalized + normalized_length_in_bytes);
          normalized_length_in_bytes += n_bytes;
        }
      } else {
        int i;
        for (i = 0; i < character_length; i++) {
          normalized[normalized_length_in_bytes + i] = rest[i];
        }
        normalized_length_in_bytes += character_length;
      }
      if (current_type) {
        char *current_normalized;
        current_normalized =
          normalized + normalized_length_in_bytes - character_length;
        current_type[0] =
          grn_nfkc_char_type((unsigned char *)current_normalized);
        current_type++;
      }
      normalized_n_characters++;
    }

    rest += character_length;
    rest_length -= character_length;
  }

  if (rest_length == 0) {
    grn_string_set_normalized(ctx,
                              string,
                              normalized,
                              normalized_length_in_bytes,
                              normalized_n_characters);
    grn_string_set_types(ctx, string, types);
  } else {
    /* TODO: report error */
    GRN_PLUGIN_FREE(ctx, normalized);
  }
}

static grn_obj *
mysql_general_ci_next(GNUC_UNUSED grn_ctx *ctx,
                      GNUC_UNUSED int nargs,
                      grn_obj **args,
                      GNUC_UNUSED grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_encoding encoding;

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][mysql-general-ci] "
                     "UTF-8 encoding is only supported: %s",
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx, string, general_ci_table);
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

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer][mysql-unicode-ci] "
                     "UTF-8 encoding is only supported: %s",
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx, string, unicode_ci_table);
  return NULL;
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

  encoding = grn_string_get_encoding(ctx, string);
  if (encoding != GRN_ENC_UTF8) {
    GRN_PLUGIN_ERROR(ctx,
                     GRN_FUNCTION_NOT_IMPLEMENTED,
                     "[normalizer]"
                     "[mysql-unicode-ci-except-kana-ci-kana-with-voiced-sound-mark] "
                     "UTF-8 encoding is only supported: %s",
                     grn_encoding_to_string(encoding));
    return NULL;
  }
  normalize(ctx, string,
            unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table);
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
  grn_normalizer_register(ctx, "NormalizerMySQLGeneralCI", -1,
                          NULL, mysql_general_ci_next, NULL);
  grn_normalizer_register(ctx, "NormalizerMySQLUnicodeCI", -1,
                          NULL, mysql_unicode_ci_next, NULL);
  grn_normalizer_register(ctx,
                          "NormalizerMySQLUnicodeCI"
                          "Except"
                          "KanaCI"
                          "KanaWithVoicedSoundMark",
                          -1,
                          NULL,
                          mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_next,
                          NULL);
  return GRN_SUCCESS;
}

grn_rc
GRN_PLUGIN_FIN(GNUC_UNUSED grn_ctx *ctx)
{
  return GRN_SUCCESS;
}
