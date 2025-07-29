# Copyright (C) 2013-2025  Sutou Kouhei <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; version 2
# of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301, USA

set(NORMALIZERS_DIR "${GRN_PLUGINS_DIR}/normalizers")
set(MYSQL_SOURCES
  mysql.c
  mysql_general_ci_table.h
  mysql_unicode_ci_table.h
  mysql_unicode_ci_except_kana_ci_kana_with_voiced_sound_mark_table.h
  mysql_unicode_520_ci_table.h
  mysql_unicode_520_ci_except_kana_ci_kana_with_voiced_sound_mark_table.h
  mysql_unicode_900_ai_ci_table.h
  mysql_unicode_900_as_ci_table.h
  mysql_unicode_900_as_cs_table.h
  mysql_unicode_900_ja_as_cs_table.h
  mysql_unicode_900_ja_as_cs_ks_table.h
  mysql_unicode_1400_ai_ci_table.h
  mysql_unicode_1400_ai_cs_table.h
  mysql_unicode_1400_as_ci_table.h
  mysql_unicode_1400_as_cs_table.h)
if(GROONGA_NORMALIZER_MYSQL_EMBED)
  add_library(mysql_normalizer STATIC ${MYSQL_SOURCES})
  set_target_properties(
    mysql_normalizer
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
else()
  add_library(mysql_normalizer MODULE ${MYSQL_SOURCES})
  set_target_properties(mysql_normalizer PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mysql")
  install(TARGETS mysql_normalizer DESTINATION "${NORMALIZERS_DIR}")
  if(MSVC)
    install(FILES $<TARGET_PDB_FILE:mysql_normalizer>
      DESTINATION "${NORMALIZERS_DIR}"
      OPTIONAL)
  endif()
endif()
target_include_directories(mysql_normalizer BEFORE PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/.."
  "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(mysql_normalizer PRIVATE Groonga::libgroonga)
target_compile_definitions(mysql_normalizer PRIVATE HAVE_CONFIG_H)
