# Copyright(C) 2013  Kouhei Sutou <kou@clear-code.com>
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

cmake_minimum_required(VERSION 2.6)
project(groonga-normalizer-mysql)

file(READ "${CMAKE_SOURCE_DIR}/version" VERSION)

include(FindPkgConfig)
include(${CMAKE_SOURCE_DIR}/build/cmake_modules/ReadFileList.cmake)

file(READ ${CAMKE_SOURCE_DIR}/required_groonga_version GROONGA_REQUIRED_VERSION)
string(STRIP "${GROONGA_REQUIRED_VERSION}" GROONGA_REQUIRED_VERSION)

pkg_check_modules(GROONGA REQUIRED "groonga >= ${GROONGA_REQUIRED_VERSION}")
_pkgconfig_invoke(groonga GROONGA PLUGINS_DIR "" --variable=pluginsdir)

add_definitions(
  -DHAVE_CONFIG_H
  )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${GROOGNA_INCLUDE_DIRS}
  )

link_directories(
  ${GROONGA_LIBRARY_DIRS}
  )

add_subdirectory(normalizers)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

configure_file(groonga.pc.in ${CMAKE_CURRENT_BINARY_DIR}/groonga.pc @ONLY)

install(FILES groonga-normalizer-mysql.pc DESTINATION "${LIB_DIR}/pkgconfig/")
