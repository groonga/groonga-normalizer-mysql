# Copyright (C) 2013-2024  Sutou Kouhei <kou@clear-code.com>
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

# Ubuntu 20.04: 3.16
# CentOS 7's EPEL: 3.17
cmake_minimum_required(VERSION 3.16)
set(GROONGA_NORMALIZER_MYSQL_PROJECT_NAME "groonga-normalizer-mysql")
project("${GROONGA_NORMALIZER_MYSQL_PROJECT_NAME}"
  VERSION "1.2.7")

include(GNUInstallDirs)

if(NOT DEFINED GROONGA_NORMALIZER_MYSQL_DOC_DIR)
  set(GROONGA_NORMALIZER_MYSQL_DOC_DIR
    "${CMAKE_INSTALL_DOCDIR}/${GROONGA_NORMALIZER_MYSQL_PROJECT_NAME}")
endif()

if(DEFINED GROONGA_NORMALIZER_MYSQL_EMBED)
  set(GROONGA_NORMALIZER_MYSQL_EMBED_DEFAULT
    ${GROONGA_NORMALIZER_MYSQL_EMBED})
else()
  set(GROONGA_NORMALIZER_MYSQL_EMBED_DEFAULT OFF)
endif()
set(GROONGA_NORMALIZER_MYSQL_EMBED
  ${GROONGA_NORMALIZER_MYSQL_EMBED_DEFAULT}
  CACHE
  BOOL
  "Build as a static library to embed into an application")

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(GROONGA_NORMALIZER_MYSQL_BUNDLED FALSE)
else()
  set(GROONGA_NORMALIZER_MYSQL_BUNDLED TRUE)
endif()

if(GROONGA_NORMALIZER_MYSQL_BUNDLED)
  if(GRN_RELATIVE_PLUGINS_DIR)
    set(GRN_PLUGINS_DIR "${GRN_RELATIVE_PLUGINS_DIR}")
  else()
    set(GRN_PLUGINS_DIR "lib/groonga/plugins")
  endif()
  if(GRN_VERSION VERSION_LESS_EQUAL "13.0.1")
    set(GRN_EMBEDED_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/../groonga")
    if(EXISTS "${GRN_EMBEDED_BINARY_DIR}")
      target_include_directories(libgroonga INTERFACE
	"$<BUILD_INTERFACE:${GRN_EMBEDED_BINARY_DIR}/include>")
    endif()
    set(GRN_EMBEDED_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../groonga")
    if(EXISTS "${GRN_EMBEDED_SOURCE_DIR}")
      target_include_directories(libgroonga INTERFACE
	"$<BUILD_INTERFACE:${GRN_EMBEDED_SOURCE_DIR}/include>")
    endif()
  endif()
  add_library(Groonga::libgroonga ALIAS libgroonga)
else()
  file(READ
    ${CMAKE_CURRENT_SOURCE_DIR}/required_groonga_version
    GROONGA_REQUIRED_VERSION)
  string(STRIP "${GROONGA_REQUIRED_VERSION}" GROONGA_REQUIRED_VERSION)

  find_package(Groonga ${GROONGA_REQUIRED_VERSION})
  if(NOT Groonga_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(pkg_groonga REQUIRED IMPORTED_TARGET
      "groonga >= ${GROONGA_REQUIRED_VERSION}")
    add_library(Groonga::libgroonga ALIAS PkgConfig::pkg_groonga)
    _pkgconfig_invoke(groonga GRN PLUGINS_DIR "" --variable=pluginsdir)
  endif()
endif()

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(normalizers)

if(NOT GROONGA_NORMALIZER_MYSQL_EMBED)
  configure_file(
    groonga-normalizer-mysql.pc.in
    "${CMAKE_CURRENT_BINARY_DIR}/groonga-normalizer-mysql.pc"
    @ONLY)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/groonga-normalizer-mysql.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
endif()

install(FILES
  "README.md"
  "doc/text/lgpl-2.0.txt"
  DESTINATION "${GROONGA_NORMALIZER_MYSQL_DOC_DIR}")
